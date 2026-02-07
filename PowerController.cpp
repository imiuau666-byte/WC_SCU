/**
 * @file PowerController.cpp
 * @brief 【全功率输出版】
 * - 频率锁定: 149kHz (你的黄金谐振点)
 * - 目标相位: 120度 (之前是 50度 -> 3.29V; 现在 120度 -> 预计 >5V)
 * - 软启动: 0度 -> 120度 缓慢爬升
 */

#include "PowerController.hpp"
#include "AnalogWrapper.hpp"
#include "HRTIMWrapper.hpp"
#include "Communication.hpp"
#include "hrtim.h"
#include <cstdio>
#include <cstring>
#include "usart.h"
namespace PowerController
{
    PowerControllerStatus powerControllerStatus;

    // --- 核心参数调整 ---
    const uint32_t TARGET_FREQ = 149000;

    // 【关键修改】加大目标相位到 120度！
    const float    TARGET_PHASE = 120.0f;

    // 保护阈值 (加大到 7A，防止误触)
    const float    MAX_CURRENT = 7.0f;

    uint32_t GetPeriod(uint32_t freq) {
        return (uint32_t)(5440000000.0f / (float)freq);
    }

    void init()
    {
        Analog::init();
        Communication::init(Analog::adcData.adc2Buffer, ADC2_CHANNEL_NUM * ADC2_BUFFER_SIZE);

        // 1. 串口日志
        HAL_Delay(500);
        if(HAL_UART_GetState(&huart3) == HAL_UART_STATE_READY) {
            const char* msg = "[BOOT] Target Phase: 120. Starting Ramp...\n";
            HAL_UART_Transmit(&huart3, (uint8_t*)msg, strlen(msg), 100);
        }

        // 2. 硬件初始化
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        __HAL_RCC_GPIOA_CLK_ENABLE();
        GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_10 | GPIO_PIN_9 | GPIO_PIN_11;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Alternate = GPIO_AF13_HRTIM1;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        // 3. 初始状态: 149kHz, 0度相位
        uint32_t p = GetPeriod(TARGET_FREQ);
        HRTIM::setPeriod(p);
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_3, p/2);
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_3, p/2);

        float current_phase = 0.0f;
        HRTIM::setfPhase(current_phase);

        // 4. 启动 PWM
        HRTIM::startTimer();
        HAL_HRTIM_WaveformOutputStart(&hhrtim1,
            HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2 |
            HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TB2);

        // 5. 【爬升主循环】
        char buf[64];
        uint32_t last_step = 0;
        uint32_t start_tick = HAL_GetTick();

        while(1)
        {
            Analog::decode();

            // --- A. 保护检测 ---
            // 上电 500ms 后才开启保护
            if(HAL_GetTick() - start_tick > 500)
            {
                if(Analog::adcData.iTX > MAX_CURRENT || Analog::adcData.iTX < -MAX_CURRENT)
                {
                    current_phase = 0.0f; // 归零
                    HRTIM::setfPhase(current_phase);

                    sprintf(buf, "[ALARM] OCP! I=%.2f. Resetting...\n", Analog::adcData.iTX);
                    if(HAL_UART_GetState(&huart3) == HAL_UART_STATE_READY)
                        HAL_UART_Transmit(&huart3, (uint8_t*)buf, strlen(buf), 10);

                    HAL_Delay(2000);
                    continue;
                }
            }

            // --- B. 爬升逻辑 ---
            // 每 50ms 增加 1.0度 (加快一点点速度，因为要爬到120)
            if(HAL_GetTick() - last_step > 50)
            {
                last_step = HAL_GetTick();

                if(current_phase < TARGET_PHASE)
                {
                    current_phase += 1.0f;
                    HRTIM::setfPhase(current_phase);
                }

                // --- C. 数据回传 ---
                if(HAL_UART_GetState(&huart3) == HAL_UART_STATE_READY)
                {
                    sprintf(buf, "%.2f,%.2f,Phase=%.1f\n", Analog::adcData.iTX, Analog::adcData.vInput, current_phase);
                    HAL_UART_Transmit(&huart3, (uint8_t*)buf, strlen(buf), 10);
                }
            }

            HAL_Delay(2);
        }
    }

    void runLowFreq1KHz() {}
    void runHighFreq() {}

} // namespace PowerController