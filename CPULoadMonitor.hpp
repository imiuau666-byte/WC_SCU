/**
* @file CPULoadMonitor.hpp
 * @author Rong Yi; Xian Ziming (zxianaa@connect.ust.hk); Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#pragma once

#include "Config.hpp"
#include "main.h"
#include "stdint.h"

#include "tim.h"

class CPULoadMonitor
{
public:
    void init(TIM_HandleTypeDef* htim_);
    void start();
    void stop();

private:
    TIM_HandleTypeDef* htim;
    float load                  = 0.0f;
    float runTime               = 0.0f;     // 单次 CPU 占用的时间, 单位 us
    uint32_t lastStartCounter   = 0;
    uint32_t startCounter       = 0;
    uint32_t periodDeltaCounter = 1;        // 两次调用 start() 之间的 delta counter
    uint32_t loadDeltaCounter   = 1;        // start() 和 stop() 之间的 delta counter

    float loadAlpha             = 0.1f;
};