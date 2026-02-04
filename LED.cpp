/**
* @file LED.cpp
 * @author Baoqi (zzhongas@connect.ust.hk); Xian Ziming (zxianaa@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */


#include "LED.hpp"

namespace LED
{
    LEDStatus_t LEDStatus;

    void setBrightness(uint8_t brightness)
    {
        LEDStatus.brightness = CLAMP(brightness, 0, 1000);
    }

    void on()
    {
        LEDStatus.blinkPeriod = 0.0f; // 不闪烁
        __HAL_TIM_SetCompare(&WHITE_LED_TIM, WHITE_LED_TIM_CHANNEL, LEDStatus.brightness);
    }

    void off()
    {
        LEDStatus.blinkPeriod = 0.0f; // 不闪烁
        __HAL_TIM_SetCompare(&WHITE_LED_TIM, WHITE_LED_TIM_CHANNEL, 0);
    }

    void blink(float period)
    {
        LEDStatus.blinkPeriod = CLAMP(period, 0.01f, 10.0f); // 最小周期为 10ms
    }

    void init()
    {
        LEDStatus.brightness = 50;
        LEDStatus.blinkPeriod = 0.0f; // 不闪烁
        LEDStatus.t = 0.0f;
        LEDStatus.onStartTime = 0;
        LEDStatus.onTime = 50;

        __HAL_TIM_SetCompare(&WHITE_LED_TIM, WHITE_LED_TIM_CHANNEL, 0);
        HAL_TIM_PWM_Start(&WHITE_LED_TIM, WHITE_LED_TIM_CHANNEL);
    }

    // 1KHz
    void update()
    {
        if(LEDStatus.blinkPeriod >= 0.01f)
        {
            LEDStatus.t += 0.001f;
            if(LEDStatus.t >= LEDStatus.blinkPeriod)
            {
                LEDStatus.t = FMOD(LEDStatus.t, LEDStatus.blinkPeriod);
                LEDStatus.onStartTime = HAL_GetTick(); // 记录当前时间
                __HAL_TIM_SetCompare(&WHITE_LED_TIM, WHITE_LED_TIM_CHANNEL, LEDStatus.brightness);

            }

            if(HAL_GetTick() - LEDStatus.onStartTime > LEDStatus.onTime)
            {
                __HAL_TIM_SetCompare(&WHITE_LED_TIM, WHITE_LED_TIM_CHANNEL, 0);
            }
        }
        else
        {
            // 如果 period 是 0, 说明常亮或者常灭.
        }
    }
} // namespace LED