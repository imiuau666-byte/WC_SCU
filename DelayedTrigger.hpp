/**
* @file DelayedTrigger.hpp
 * @author Baoqi (zzhongas@connect.ust.hk);
 *
 * @copyright Copyright (c) 2025
 */

#pragma once
#include "Config.hpp"

#include "main.h"
#include "stdint.h"

namespace DelayedTrigger
{
    class DelayedTrigger
    {
    public:
        DelayedTrigger(uint32_t timeout_, uint32_t increasingSpeed_, uint32_t decreasingSpeed_) :
            timeout(timeout_),
            increasingSpeed(increasingSpeed_),
            decreasingSpeed(decreasingSpeed_) {}

        DelayedTrigger(uint32_t timeout_ = 10) :
            timeout(timeout_) {}

        uint8_t operator()(uint8_t currentStatus);

        void reset()
        {
            triggerStatus = 0;
            triggerCounter = 0;
        }

    private:
        uint32_t timeout;          // Delay in milliseconds
        uint8_t triggerStatus       = 0;
        uint32_t triggerCounter     = 0;
        uint32_t increasingSpeed    = 1;
        uint32_t decreasingSpeed    = 1;
    };


}  // namespace DelayedTrigger