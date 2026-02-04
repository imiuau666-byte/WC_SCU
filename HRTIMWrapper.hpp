/**
* @file HRTIMWrapper.hpp
 * @author Xian Ziming (zxianaa@connect.ust.hk); Baoqi (zzhongas@connect.ust.hk);
 *
 * @copyright Copyright (c) 2025
 */

#pragma once
#include "Config.hpp"
#include "main.h"

#include "stdint.h"

namespace HRTIM
{
    struct HRTIMStatus
    {
        bool timerEnabled = 0;
        bool outputEnabled = 0;
        uint16_t period = 40000;    // 仅用于 monitor, 不做修改
        float fPhase = 0;           // 仅用于 monitor, 不做修改
    };

    extern HRTIMStatus hrtimStatus;

    /**
        * @brief 设置 HRTIM 的周期
        * @param period 周期值
    */
    void setPeriod(uint16_t period);

    /**
        * @brief 设置 HRTIM 的相位
        * @param fPhase 相位值，范围在 0.0 到 0.5 之间
    */
    void setfPhase(float fPhase);

    /**
        * @brief 开启 HRTimer Counter 计数
    */
    void startTimer();

    /**
        * @brief 关闭 HRTimer Counter 计数
    */
    void stopTimer();

    /**
        * @brief 开启低侧输出, 用于冲 bootstrap 电容
    */
    void enableOutputLowSide();

    /**
        * @brief 开启高侧输出
    */
    void enableOutputHighSide();

    /**
        * @brief 关闭高低测输出
    */
    void disableOutput();


} // namespace HRTIM

