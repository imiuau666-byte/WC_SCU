/**
* @file Buzzer.cpp
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#pragma once
#include "Config.hpp"

#include "main.h"
#include "stdint.h"

namespace Buzzer
{
    typedef struct
    {
        float frequency;
        float onDuration;
        float offDuration;
        float intensity;
    } Note;

    typedef struct
    {
        uint16_t preScaler;
        uint16_t compare;

        uint32_t onDuration;
        uint32_t offDuration;
        uint32_t playTime;
    } NoteQueueItem;

    void init();

    void play(Note note);

    void stopAll();

    uint8_t anyNotePlaying();

    void update();
} // namespace Buzzer