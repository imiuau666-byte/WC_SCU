/**
 * @file HRTIMWrapper.cpp
 * @author Xian Ziming (zxianaa@connect.ust.hk); Baoqi (zzhongas@connect.ust.hk);
 *
 * @copyright Copyright (c) 2025
 */

#include "HRTIMWrapper.hpp"
#include "hrtim.h"

namespace HRTIM
{
HRTIMStatus hrtimStatus;

void startTimer()
{
    HAL_HRTIM_WaveformCountStart(&hhrtim1, HRTIM_TIMERID_MASTER);
    HAL_HRTIM_WaveformCountStart(&hhrtim1, HRTIM_TIMERID_TIMER_A);
    HAL_HRTIM_WaveformCountStart(&hhrtim1, HRTIM_TIMERID_TIMER_B);

    __HAL_HRTIM_MASTER_ENABLE_IT(&hhrtim1, HRTIM_MASTER_IT_MREP);
    hrtimStatus.timerEnabled = 1;
}

void stopTimer()
{
    disableOutput();
    HAL_HRTIM_WaveformCounterStop(&hhrtim1, HRTIM_TIMERID_MASTER);
    HAL_HRTIM_WaveformCounterStop(&hhrtim1, HRTIM_TIMERID_TIMER_A);
    HAL_HRTIM_WaveformCounterStop(&hhrtim1, HRTIM_TIMERID_TIMER_B);

    __HAL_HRTIM_MASTER_DISABLE_IT(&hhrtim1, HRTIM_MASTER_IT_MREP);
    hrtimStatus.timerEnabled = 0;
}

void enableOutputLowSide()
{
    HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TA2 | HRTIM_OUTPUT_TB2);
}

void enableOutputHighSide()
{
    HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TB1);
    hrtimStatus.outputEnabled = 1;
}

void disableOutput()
{
    HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2);
    HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TB2);
    hrtimStatus.outputEnabled = 0;
}


void setPeriod(uint16_t period)
{
    hrtimStatus.period = period;

    // 设置 Master Timer, Timer A, Timer B 的周期
    hhrtim1.Instance->sMasterRegs.MPER = period;
    hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].PERxR = period;
    hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_B].PERxR = period;

    // 设置 Timer A, B 占空比为 50%
    hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CMP1xR = 0;
    hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CMP3xR = period / 2 + 1;
    hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_B].CMP1xR = 0;
    hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_B].CMP3xR = period / 2 + 1;

    // 设置 ADC 触发比较值
    hhrtim1.Instance->sMasterRegs.MCMP2R = period / 2;
}

void setfPhase(float fPhase)
{
    hrtimStatus.fPhase = CLAMP(fPhase, 0.0f, 0.9f);

    uint32_t phase = hrtimStatus.fPhase * HRTIM_PERIOD;

    // // Set period of MASTER, Timer A and Timer B. 以 Master Timer 为判断依据
    // if(hhrtim1.Instance->sMasterRegs.MPER != HRTIM_PERIOD)
    //     setPeriod(period);

    // Set phase of Timer A and Timer B
    // Timer A reset is triggered by Master Timer Compare Unit 1
    hhrtim1.Instance->sMasterRegs.MCMP1R = (HRTIM_PERIOD >> 2) - (phase >> 1);
    // Timer B reset is triggered by Master Timer Compare Unit 3
    hhrtim1.Instance->sMasterRegs.MCMP3R = (HRTIM_PERIOD >> 2) + (phase >> 1);
}

} // namespace HRTIM
