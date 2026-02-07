/**
* @file UserTask.hpp
 * @author Xian Ziming (zxianaa@connect.ust.hk); Rong Yi; Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "main.h"
#include "hrtim.h"
#include "stdint.h"
#include "tim.h"
#include "adc.h"

#include "PowerController.hpp"
#include "Buzzer.hpp"
#include "LED.hpp"
#include "WS2812.hpp"

#include "CPULoadMonitor.hpp"


CPULoadMonitor _1KHzLoad;
CPULoadMonitor _36KHzLoad;

extern "C"
{
    void systemStart()
    {
        _1KHzLoad.init(&htim16);
        _36KHzLoad.init(&htim16);

        Buzzer::init();
        WS2812::init();
        LED::init();

        PowerController::init();

        while (true)
        {
            HAL_Delay(1);
        }
    }


    void TIM6_DAC_IRQHandler(void)
    {
        __HAL_TIM_CLEAR_IT(&htim6, TIM_IT_UPDATE);

        _1KHzLoad.start();

        Buzzer::update();
        LED::update();
        WS2812::transmit();

        PowerController::runLowFreq1KHz();

        _1KHzLoad.stop();
    }



    // 36KHz
    void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
    {
        if(hadc->Instance == ADC1)
        {
            _36KHzLoad.start();

            PowerController::runHighFreq();

            _36KHzLoad.stop();
        }
    }
}