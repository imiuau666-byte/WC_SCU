

#include "AnalogWrapper.hpp"

#include <cstdio>

#include "opamp.h"
#include "adc.h"
#include "stdio.h"

#include "usart.h" // 确保包含了串口头文件
#include <string.h>
#include <stdlib.h>

namespace Analog
{
ADCData adcData;

// High Freq 36k 调用`
void init()
{
    for(uint8_t i = 0; i < ADC_WINDOW_SIZE; i++)
    {
        adcData.iTxWindow[i] = 0;
        adcData.VinWindow[i] = 0;
    }

    HAL_Delay(10);
    HAL_OPAMP_SelfCalibrate(&hopamp1);
    HAL_OPAMP_SelfCalibrate(&hopamp2);
    HAL_OPAMP_SelfCalibrate(&hopamp3);

    HAL_OPAMP_Start(&hopamp1);  // 母线电压采样
    HAL_OPAMP_Start(&hopamp2);  // 全桥电流采样放大器
    HAL_OPAMP_Start(&hopamp3);  // VREF跟随

    // 校准ADC1 ADC2
    HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
    HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);
    HAL_Delay(1);

    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adcData.adc1Buffer, ADC1_CHANNEL_NUM * ADC1_BUFFER_SIZE);
    HAL_ADC_Start_DMA(&hadc2, (uint32_t *)adcData.adc2Buffer, ADC2_CHANNEL_NUM * ADC2_BUFFER_SIZE);
    // 使能中断

    // __HAL_ADC_ENABLE_IT(&hadc1, ADC_IT_EOS);
}

void decode()
{
    // ADC1_CH1: Current of FullBridge
    // ADC1_CH2: Voltage of Vin
    // ADC2_CH1: Backward Communication

    uint32_t iTxAccumulate = 0;
    uint32_t VinAccumulate = 0;
    uint16_t* pBuffer = adcData.adc1Buffer;
    for (uint8_t i = 0; i < ADC1_BUFFER_SIZE; i++)
    {
        iTxAccumulate += pBuffer[0];
        VinAccumulate += pBuffer[1];
        pBuffer += ADC1_CHANNEL_NUM;
    }

    adcData.iTxWindowAccumulate += iTxAccumulate - adcData.iTxWindow[adcData.windowPointer];
    adcData.VinWindowAccumulate += VinAccumulate - adcData.VinWindow[adcData.windowPointer];
    adcData.iTxWindow[adcData.windowPointer] = iTxAccumulate;
    adcData.VinWindow[adcData.windowPointer] = VinAccumulate;
    adcData.windowPointer++;
    if (adcData.windowPointer >= ADC_WINDOW_SIZE)
    {
        adcData.windowPointer = 0;
    }


    float iTx = ((float)adcData.iTxWindowAccumulate / ADC1_BUFFER_SIZE / ADC_WINDOW_SIZE - ISENSE_ADC_BAIS) * ISENSE_ADC_GAIN;

    if (iTx < 0.12f && iTx > -0.12f) {
        iTx = 0.0f;
    }
    float Vin = ((float)adcData.VinWindowAccumulate / ADC1_BUFFER_SIZE / ADC_WINDOW_SIZE - VIN_ADC_BAIS   ) * VIN_ADC_GAIN   ;

    adcData.iTX     = iTx * ISENSE_FILTER_ALPHA + adcData.iTX    * (1 - ISENSE_FILTER_ALPHA);
    adcData.vInput  = Vin * VIN_FILTER_ALPHA    + adcData.vInput * (1 - VIN_FILTER_ALPHA   );
    adcData.pTX     = adcData.vInput * adcData.iTX;

/*打印电压电流到串口
    // 低频打印逻辑：每 2500 次触发一次 (假设 TIM6 是 50kHz，这里就是 20Hz)
    static uint16_t print_cnt = 0;
    if (++print_cnt >= 2500)
    {
        print_cnt = 0;

        char t_buf[12]; // 临时缓冲区

        // --- 打印电压 ---
        int v_i = (int)adcData.vInput;
        int v_f = (int)((adcData.vInput - v_i) * 100);

        itoa(v_i, t_buf, 10);
        HAL_UART_Transmit(&huart3, (uint8_t*)t_buf, strlen(t_buf), 2);
        HAL_UART_Transmit(&huart3, (uint8_t*)".", 1, 2);
        if(abs(v_f) < 10) HAL_UART_Transmit(&huart3, (uint8_t*)"0", 1, 2);
        itoa(abs(v_f), t_buf, 10);
        HAL_UART_Transmit(&huart3, (uint8_t*)t_buf, strlen(t_buf), 2);

        // 分隔符
        HAL_UART_Transmit(&huart3, (uint8_t*)",", 1, 2);

        // --- 打印电流 ---
        int i_i = (int)adcData.iTX;
        int i_f = (int)((adcData.iTX - i_i) * 100);

        itoa(i_i, t_buf, 10);
        HAL_UART_Transmit(&huart3, (uint8_t*)t_buf, strlen(t_buf), 2);
        HAL_UART_Transmit(&huart3, (uint8_t*)".", 1, 2);
        if(abs(i_f) < 10) HAL_UART_Transmit(&huart3, (uint8_t*)"0", 1, 2);
        itoa(abs(i_f), t_buf, 10);
        HAL_UART_Transmit(&huart3, (uint8_t*)t_buf, strlen(t_buf), 2);

        // 结尾换行
        HAL_UART_Transmit(&huart3, (uint8_t*)"\n", 1, 2);
    }
*/

 }
}// namespace ADC
