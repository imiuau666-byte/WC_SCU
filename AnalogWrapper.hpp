

#pragma once

#include "Config.hpp"
#include "main.h"
#include "stdint.h"

namespace Analog
{
    typedef struct
    {
        uint16_t adc1Buffer[ADC1_CHANNEL_NUM * ADC1_BUFFER_SIZE];
        uint16_t adc2Buffer[ADC2_CHANNEL_NUM * ADC2_BUFFER_SIZE];
        uint32_t iTxWindow[ADC_WINDOW_SIZE] = {0};
        uint32_t VinWindow[ADC_WINDOW_SIZE] = {0};
        uint32_t iTxWindowAccumulate = 0;
        uint32_t VinWindowAccumulate = 0;
        uint8_t windowPointer = 0;

        float vInput;
        float iTX;
        float pTX;
    } ADCData;

    extern ADCData adcData;

    /**
        * @brief: 初始化 ADC & Opamp
        * @param: None
        */
    void init();


    /**
        * @brief: 解码ADC数据, 解码后的数据存放在 adcData 中
        * @param: None
        */
    void decode();

}