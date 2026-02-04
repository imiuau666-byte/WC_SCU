/**
* @file IncreasementPID.cpp
 * @author Xian Ziming (zxianaa@connect.ust.hk);
 *
 * @copyright Copyright (c) 2025
 */

#pragma once
#include "Config.hpp"
#include "main.h"

#include "stdint.h"

struct IncreasementPID
{
public:
    float kp = 0.0f;
    float ki = 0.0f;
    float kd = 0.0f;

    float error_0 = 0.0f;
    float error_1 = 0.0f;
    float error_2 = 0.0f;

    float target = 0.0f;
    float current = 0.0f;
    float delta_output = 0.0f;

    float clamp_upper = 0.0f;
    float clamp_lower = 0.0f;
    bool clamp_enabled = false;

    IncreasementPID(float _kp, float _ki, float _kd) : kp(_kp), ki(_ki), kd(_kd) {}
    void setParameter(float _kp, float _ki, float _kd);
    void setClamp(float _lower, float _upper);
    void disableClamp();
    float computeDelta(float _target, float _current);
    void resetError();
};
