/**
* @file LED.cpp
 * @author Baoqi (zzhongas@connect.ust.hk); Xian Ziming (zxianaa@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#pragma once
#include "Config.hpp"

#include "main.h"
#include "tim.h"
#include "stdint.h"


namespace LED
{
    typedef struct
    {
        uint8_t brightness = 0;  // 0-1000, 等于 timer 的 period
        float blinkPeriod = 0.0f;  // 单位 s
        float t;
        uint32_t onStartTime = 0;  // 开始闪烁的时间
        uint32_t onTime = 0;  // 闪烁的时间,
    } LEDStatus_t;


    /**
     * @brief 设置 LED 的亮度
     * @param brightness 亮度值, 0-1000
     */
    void setBrightness(uint8_t brightness);


    /**
     * @brief 打开 LED, 且常亮
     * @note 亮度由 setBrightness() 设置
     */
    void on();


    /**
     * @brief 关闭 LED, 且常灭 不 blink
     * @note 亮度由 setBrightness() 设置
     */
    void off();

    /**
     * @brief 设置 LED 闪烁周期
     * @param period 闪烁周期, 单位 s, 最小 10ms
     */
    void blink(float period);

    /**
     * @brief 初始化 LED
     */
    void init();

    /**
     * @brief 更新 LED 状态, 需要 1KHz 调用一次
     */
    void update();
}  // namespace LED