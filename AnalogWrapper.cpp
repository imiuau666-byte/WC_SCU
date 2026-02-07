

#include "AnalogWrapper.hpp"

#include <cstdio>

#include "opamp.h"
#include "adc.h"
#include "stdio.h"

#include "usart.h"  // 确保能找到 huart3
#include <string.h>
#include <stdlib.h>
#include "main.h"
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



    float Vin = ((float)adcData.VinWindowAccumulate / ADC1_BUFFER_SIZE / ADC_WINDOW_SIZE - VIN_ADC_BAIS   ) * VIN_ADC_GAIN   ;

    adcData.iTX     = iTx * ISENSE_FILTER_ALPHA + adcData.iTX    * (1 - ISENSE_FILTER_ALPHA);
    adcData.vInput  = Vin * VIN_FILTER_ALPHA    + adcData.vInput * (1 - VIN_FILTER_ALPHA   );
    adcData.pTX     = adcData.vInput * adcData.iTX;

    // ---------------------------------------------------------
    // 3. 发送数据到 VOFA+ (FireWater协议: "电流,电压\n")
    // ---------------------------------------------------------
    static uint16_t send_div = 0;

    // 降频发送：每 10 次 decode 发送一次 (防止串口堵死)
    // 如果你想看更细节的波形，可以把 10 改成 5 或 2，但要注意不要让单片机卡死
    if (++send_div >= 10)
    {
        send_div = 0;

        char vofa_buf[64];

        // 拼接字符串 (使用整数拼接法避免浮点打印问题)
        int i_main = (int)adcData.iTX;
        int i_sub  = abs((int)((adcData.iTX - i_main) * 1000)); // 3位小数

        int v_main = (int)adcData.vInput;
        int v_sub  = abs((int)((adcData.vInput - v_main) * 100)); // 2位小数

        // 格式: "Current,Voltage\n"
        sprintf(vofa_buf, "%d.%03d,%d.%02d\n", i_main, i_sub, v_main, v_sub);

        // 发送 (确保 huart3 已初始化)
        if(HAL_UART_GetState(&huart3) == HAL_UART_STATE_READY)
        {
            HAL_UART_Transmit(&huart3, (uint8_t*)vofa_buf, strlen(vofa_buf), 5);
        }
    }

 }
}// namespace ADC
