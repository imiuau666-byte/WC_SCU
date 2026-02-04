/**
 * @file PowerController.cpp
 * @author Baoqi (zzhongas@connect.ust.hk); Xian Ziming (zxianaa@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#pragma once

#include "Config.hpp"
#include "main.h"
#include "stdint.h"

#include "DelayedTrigger.hpp"

namespace PowerController
{
typedef enum
{
    Stop = 0,
    ChargingBootCap,
    preDetecting,
    Detecting,
    pre10WWaitingForASK,
    _10WWaitingForASK,
    pre100WRunning,
    _100WRunning,
} PowerControllerState;

typedef struct
{
    uint8_t overVoltage     = 0;
    uint8_t underVoltage    = 0;
    uint8_t lowEfficency    = 0;
    uint8_t highEfficency   = 0;    // 效率大于 95%

    uint8_t overCurrent     = 0;
    uint8_t overPhase       = 0;    // 占空比过高, 仅用于保护

    uint8_t _10WWaitingForASKTimeout = 0;
    uint8_t _100WRunningASKDisconnect = 0;
} ErrorStatus;

typedef struct
{
    PowerControllerState state = PowerControllerState::Stop;

    ErrorStatus lastErrorStatus;        // 自动恢复之后会把上次的错误存起来, For debug purpose
    ErrorStatus errorStatus;            // 在自动恢复前累计的错误, 自动恢复之后会清除, For debug purpose
    ErrorStatus currentErrorStatus;     // 当前瞬间是否仍处于错误状态
    uint16_t noErrorCounter = 0;
    float powerPhaseRatio = 0;

    DelayedTrigger::DelayedTrigger overVoltageDelayedTrigger    {OVER_VOLTAGE_SHUTDOWM_TIME};
    DelayedTrigger::DelayedTrigger underVoltageDelayedTrigger   {UNDER_VOLTAGE_SHUTDOWM_TIME};
    DelayedTrigger::DelayedTrigger lowEfficencyDelayedTrigger   {LOW_EFFICENCY_TIMEOUT};    // 低效率保护延时
    DelayedTrigger::DelayedTrigger highEfficencyDelayedTrigger  {HIGH_EFFICENCY_TIMEOUT};   // 高效率保护延时

    DelayedTrigger::DelayedTrigger overCurrentDelayedTrigger    {OVER_CURRENT_SHUTDOWM_TIME * POWER_CONTROLLER_HIGH_FREQ / POWER_CONTROLLER_LOW_FREQ};
    DelayedTrigger::DelayedTrigger overPhaseDelayedTrigger      {OVER_PHASE_SHUTDOWM_TIME * POWER_CONTROLLER_HIGH_FREQ / POWER_CONTROLLER_LOW_FREQ};

    DelayedTrigger::DelayedTrigger detectDelayedTrigger         {3};

    uint8_t autoRecover                 = 1;

    uint32_t waitingBeforeStartingTime  = 0;
    uint32_t waitingForASKStartTime     = 0;

    float setPhase                      = 0;
    float targetTxPower                 = 5.0f;
} PowerControllerStatus;

extern PowerControllerStatus powerControllerStatus;

/**
    * @brief 初始化控制器
*/
void init();

/**
    * @brief 控制器低速运行, 包括状态机(错误 recovery)
*/
void runLowFreq1KHz();

/**
    * @brief 控制器高速运行, 包括解码 ADC 数据, 电流 PID, 电流保护逻辑
*/
void runHighFreq();
} // namespace PowerController
