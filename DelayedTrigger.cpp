/**
* @file DelayedTrigger.cpp
 * @author Baoqi (zzhongas@connect.ust.hk);
 *
 * @copyright Copyright (c) 2025
 */

#include "DelayedTrigger.hpp"

namespace DelayedTrigger
{
    uint8_t DelayedTrigger::operator()(uint8_t currentStatus)
    {
        if(currentStatus)
        {
            if(triggerCounter < timeout)
            {
                triggerCounter += increasingSpeed;
            }
            else
            {
                triggerStatus = 1; // Triggered
            }
        }
        else
        {
            if(triggerCounter)
            {
                triggerCounter -= decreasingSpeed;
            }
            else
            {
                triggerStatus = 0; // Not triggered
            }
        }
        return triggerStatus;
    }
} // namespace DelayedTrigger