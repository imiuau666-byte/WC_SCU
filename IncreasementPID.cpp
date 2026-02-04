/**
* @file IncreasementPID.cpp
 * @author Xian Ziming (zxianaa@connect.ust.hk);
 *
 * @copyright Copyright (c) 2025
 */

#include "IncreasementPID.hpp"

void IncreasementPID::setParameter(float _kp, float _ki, float _kd)
{
    kp = _kp;
    ki = _ki;
    kd = _kd;
}

void IncreasementPID::setClamp(float _lower, float _upper)
{
    clamp_lower = _lower;
    clamp_upper = _upper;
    clamp_enabled = true;
}

void IncreasementPID::disableClamp()
{
    clamp_enabled = false;
}

float IncreasementPID::computeDelta(float _target, float _current)
{
    target = _target;
    error_2 = error_1;
    error_1 = error_0;
    error_0 = _target - _current;
    delta_output = kp * (error_0 - error_1) + ki * error_0 + kd * (error_0 - 2 * error_1 + error_2);
    if (clamp_enabled)
    {
        if (delta_output > clamp_upper)
            delta_output = clamp_upper;
        else if (delta_output < clamp_lower)
            delta_output = clamp_lower;
    }
    return delta_output;
}

void IncreasementPID::resetError()
{
    error_0 = 0.0f;
    error_1 = 0.0f;
    error_2 = 0.0f;
}

