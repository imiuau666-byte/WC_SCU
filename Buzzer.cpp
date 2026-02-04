/**
 * @file Buzzer.cpp
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "Buzzer.hpp"
#include "tim.h"

namespace Buzzer
{
NoteQueueItem noteQueue[NOTE_QUEUE_LENGTH];
uint8_t noteQueueWriteIndex = 0;
uint8_t noteQueueReadIndex  = 0;

static inline uint16_t hz2psc(float freq) { return (uint16_t)((float)BUZZER_TIM_CLOCK_FREQ / (BUZZER_TIM.Init.Period + 1) / freq); }
static inline uint16_t intensityFloatToUInt(float intensity) { return (BUZZER_TIM.Init.Period + 1) * intensity; }


void init()
{
    __HAL_TIM_SetCompare(&BUZZER_TIM, BUZZER_TIM_CHANNEL, 0);
    HAL_TIM_PWM_Start(&BUZZER_TIM, BUZZER_TIM_CHANNEL);
}

void play(Note note)
{
    if ((noteQueueWriteIndex + 1) % NOTE_QUEUE_LENGTH == noteQueueReadIndex)
        return;

    noteQueue[noteQueueWriteIndex].preScaler = hz2psc(note.frequency);
    noteQueue[noteQueueWriteIndex].compare = intensityFloatToUInt(note.intensity);

    noteQueue[noteQueueWriteIndex].onDuration = (uint32_t)(note.onDuration / 1000.0f * BUZZER_HANDLER_FREQ);
    noteQueue[noteQueueWriteIndex].offDuration = (uint32_t)(note.offDuration / 1000.0f * BUZZER_HANDLER_FREQ);
    noteQueue[noteQueueWriteIndex].playTime = 0;

    noteQueueWriteIndex ++;
    if (noteQueueWriteIndex >= NOTE_QUEUE_LENGTH)
        noteQueueWriteIndex = 0;
}

void stopAll()
{
    noteQueueWriteIndex = 0;
    noteQueueReadIndex = 0;
}

uint8_t anyNotePlaying()
{
    return noteQueueReadIndex != noteQueueWriteIndex;
}

void update()
{
    if(noteQueueReadIndex == noteQueueWriteIndex)
        return;


    if(noteQueue[noteQueueReadIndex].playTime <= noteQueue[noteQueueReadIndex].onDuration)
    {
        __HAL_TIM_SetCompare(&BUZZER_TIM, BUZZER_TIM_CHANNEL, noteQueue[noteQueueReadIndex].compare);
        __HAL_TIM_SET_PRESCALER(&BUZZER_TIM, noteQueue[noteQueueReadIndex].preScaler);
    }
    else
    {
        __HAL_TIM_SetCompare(&BUZZER_TIM, BUZZER_TIM_CHANNEL, 0);
    }

    noteQueue[noteQueueReadIndex].playTime ++;

    if(noteQueue[noteQueueReadIndex].playTime >= noteQueue[noteQueueReadIndex].onDuration + noteQueue[noteQueueReadIndex].offDuration)
    {
        noteQueueReadIndex = (noteQueueReadIndex + 1) % NOTE_QUEUE_LENGTH;
    }
}
} // namespace Buzzer