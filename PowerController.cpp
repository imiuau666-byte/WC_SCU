/**
 * @file PowerController.cpp
 * @author Baoqi (zzhongas@connect.ust.hk); Xian Ziming (zxianaa@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "PowerController.hpp"
#include "AnalogWrapper.hpp"
#include "HRTIMWrapper.hpp"
#include "IncreasementPID.hpp"
#include "Communication.hpp"
#include "Buzzer.hpp"
#include "LED.hpp"
#include "WS2812.hpp"

namespace PowerController
{
PowerControllerStatus powerControllerStatus;

IncreasementPID powerPID(0.0f, 0.3f, 0.0f);
IncreasementPID receivePowerPID(0.0f, 20.0f, 0.0f);


void init()
{
    Analog::init();
    Communication::init(Analog::adcData.adc2Buffer, ADC2_CHANNEL_NUM * ADC2_BUFFER_SIZE);
    Buzzer::play({1000.0f, 150.0f, 80.0f, 0.5f});

    // 1kHz tick 状态机
    HAL_TIM_Base_Start_IT(&htim6);

    HRTIM::setPeriod(HRTIM_PERIOD);
    HRTIM::setfPhase(0.0f);
    HRTIM::startTimer();
    powerPID.setClamp(-10.0f, 10.0f);

    powerControllerStatus.state = PowerControllerState::ChargingBootCap;
}

void runLowFreq1KHz()
{
    if(powerControllerStatus.state == PowerControllerState::Stop)
    {
        HRTIM::disableOutput();
    }

    else if(powerControllerStatus.state == PowerControllerState::ChargingBootCap)
    {
        // 充 bootstrap 电容
        HRTIM::setfPhase(0.0f);
        HRTIM::enableOutputLowSide();

        // 等待电压正常
        if(Analog::adcData.vInput > UNDER_VOLTAGE_THRESHOLD && Analog::adcData.vInput < OVER_VOLTAGE_THRESHOLD)
        {
            powerControllerStatus.state = PowerControllerState::preDetecting;
        }
    }

    else if(powerControllerStatus.state == PowerControllerState::preDetecting)
    {
        HRTIM::enableOutputHighSide();
        powerControllerStatus.setPhase = 0.0f;
        HRTIM::setfPhase(powerControllerStatus.setPhase);
        powerControllerStatus.targetTxPower = DETECT_THRESHOLD_POWER * 1.5f;
        powerPID.resetError();

        powerControllerStatus.detectDelayedTrigger.reset();
        powerControllerStatus.lowEfficencyDelayedTrigger.reset();
        powerControllerStatus.highEfficencyDelayedTrigger.reset();

        Buzzer::play({1500.0f, 150.0f, 80.0f, 0.5f});
        powerControllerStatus.state = PowerControllerState::Detecting;
    }

    else if(powerControllerStatus.state == PowerControllerState::Detecting)
    {
        LED::blink(1.0f);
        WS2812::blink(0, 0, 20, 0);
        WS2812::blink(1, 0, 0, 0);

        // 如果发射功率够大, 切换到 10W 等待 ASK
        if(powerControllerStatus.detectDelayedTrigger(Analog::adcData.pTX > DETECT_THRESHOLD_POWER))
        {
            powerControllerStatus.state = PowerControllerState::pre10WWaitingForASK;
        }

        // 如果直接收到了合法的 ASK, 直接切换到 100W
        if(
            Communication::isConnected() && Communication::isValid()
        )
        {
            powerControllerStatus.state = PowerControllerState::pre100WRunning;
        }
    }

    else if(powerControllerStatus.state == PowerControllerState::pre10WWaitingForASK)
    {
        powerControllerStatus.targetTxPower = 10.0f;
        powerControllerStatus.waitingForASKStartTime = HAL_GetTick();
        powerControllerStatus.state = PowerControllerState::_10WWaitingForASK;
    }

    else if(powerControllerStatus.state == PowerControllerState::_10WWaitingForASK)
    {
        // 超时退出运行
        if(
            HAL_GetTick() - powerControllerStatus.waitingForASKStartTime > 50
        )
        {
            HRTIM::disableOutput();
            powerControllerStatus.errorStatus._10WWaitingForASKTimeout          = 1;
            powerControllerStatus.currentErrorStatus._10WWaitingForASKTimeout   = 0;    // 这个断联是瞬间触发的, 不需要一个 current 的逻辑
            powerControllerStatus.noErrorCounter = 0;
            powerControllerStatus.state = PowerControllerState::Stop;                   // 超时用 Auto recovery 处理
            LED::off();
            WS2812::blink(0, 20, 0, 0);
            WS2812::blink(1, 20, 0, 0);
            return;
        }
        WS2812::blink(0, 0, 20, 0);
        WS2812::blink(1, 0, 0, 20);

        // 等待反向通信, 等待合法的 ASK 包才切换
        if(
            Communication::isConnected() && Communication::isValid()
        )
        {
            powerControllerStatus.state = PowerControllerState::pre100WRunning;
        }
    }

    else if(powerControllerStatus.state == PowerControllerState::pre100WRunning)
    {
        // 此时不应该 reset phase, 应该让它从 10W 慢慢闭环到 150
        powerControllerStatus.targetTxPower = 10.0f;
        Buzzer::play({2000.0f, 150.0f, 100.0f, 0.8f});
        powerControllerStatus.state = PowerControllerState::_100WRunning;
    }

    else if(powerControllerStatus.state == PowerControllerState::_100WRunning)
    {
        // 只要 connected, 就算不 Valid 也不退出. 如果只是 connected 但是不 Valid, 则在后面不会更新功率 pid.
        if(Communication::isConnected() == 0)
        {
            HRTIM::disableOutput();
            powerControllerStatus.errorStatus._100WRunningASKDisconnect         = 1;
            powerControllerStatus.currentErrorStatus._100WRunningASKDisconnect  = 0;    // 这个断联是瞬间触发的, 不需要一个 current 的逻辑
            powerControllerStatus.noErrorCounter = 0;
            powerControllerStatus.state = PowerControllerState::Stop;                   // 超时用 Auto recovery 处理
            Buzzer::play({200.0f, 250.0f, 100.0f, 0.6f});
            LED::off();
            WS2812::blink(0, 20, 0, 0);
            WS2812::blink(1, 20, 0, 0);
            return;
        }
    }


    // 过压欠压保护
    powerControllerStatus.currentErrorStatus.overVoltage    = powerControllerStatus.overVoltageDelayedTrigger(Analog::adcData.vInput > OVER_VOLTAGE_THRESHOLD);
    powerControllerStatus.currentErrorStatus.underVoltage   = powerControllerStatus.underVoltageDelayedTrigger(Analog::adcData.vInput < UNDER_VOLTAGE_THRESHOLD);
    // 效率范围保护
    if(Communication::isConnected() && Communication::isValid())
    {
        powerControllerStatus.currentErrorStatus.lowEfficency   = powerControllerStatus.lowEfficencyDelayedTrigger(Communication::CommunicationStatus.backwardCommunicationData.transmitEfficiency < LOW_EFFICENCY_THRESHOLD);
        powerControllerStatus.currentErrorStatus.highEfficency  = powerControllerStatus.highEfficencyDelayedTrigger(Communication::CommunicationStatus.backwardCommunicationData.transmitEfficiency > HIGH_EFFICENCY_THRESHOLD);
    }
    else
    {
        powerControllerStatus.currentErrorStatus.lowEfficency   = 0;
        powerControllerStatus.currentErrorStatus.highEfficency  = 0;
    }

    if(
        powerControllerStatus.currentErrorStatus.overVoltage    ||
        powerControllerStatus.currentErrorStatus.underVoltage
        // 计算 efficiency 但是不保护
        // powerControllerStatus.currentErrorStatus.lowEfficency   ||
        // powerControllerStatus.currentErrorStatus.highEfficency
    )
    {
        powerControllerStatus.errorStatus.overVoltage   |= powerControllerStatus.currentErrorStatus.overVoltage;
        powerControllerStatus.errorStatus.underVoltage  |= powerControllerStatus.currentErrorStatus.underVoltage;
        powerControllerStatus.errorStatus.lowEfficency  |= powerControllerStatus.currentErrorStatus.lowEfficency;
        powerControllerStatus.errorStatus.highEfficency |= powerControllerStatus.currentErrorStatus.highEfficency;

        if(powerControllerStatus.state != PowerControllerState::Stop)
        {
            HRTIM::disableOutput();
            powerControllerStatus.noErrorCounter = 0;
            powerControllerStatus.state = PowerControllerState::Stop;
            Buzzer::play({200.0f, 250.0f, 100.0f, 0.6f});
            LED::off();
            WS2812::blink(0, 20, 0, 0);
            WS2812::blink(1, 20, 0, 0);
        }
    }

    // Auto Recovery from Error
    if(
        powerControllerStatus.state == PowerControllerState::Stop &&
        powerControllerStatus.autoRecover &&
        (
            powerControllerStatus.errorStatus.overVoltage   ||
            powerControllerStatus.errorStatus.underVoltage  ||
            powerControllerStatus.errorStatus.lowEfficency  ||
            powerControllerStatus.errorStatus.highEfficency ||
            powerControllerStatus.errorStatus.overCurrent   ||
            powerControllerStatus.errorStatus.overPhase     ||
            powerControllerStatus.errorStatus._10WWaitingForASKTimeout ||
            powerControllerStatus.errorStatus._100WRunningASKDisconnect
        ) &&
        powerControllerStatus.currentErrorStatus.overVoltage == 0   &&
        powerControllerStatus.currentErrorStatus.underVoltage == 0  &&
        powerControllerStatus.currentErrorStatus.lowEfficency == 0  &&
        powerControllerStatus.currentErrorStatus.highEfficency == 0 &&
        powerControllerStatus.currentErrorStatus.overCurrent == 0   &&
        powerControllerStatus.currentErrorStatus.overPhase == 0
        // ASK 相关的没有人来 set current status, 所以不判断
    )
    {
        powerControllerStatus.noErrorCounter ++;
        if(powerControllerStatus.noErrorCounter > AUTO_RECOVER_TIMEOUT)
        {
            powerControllerStatus.lastErrorStatus = powerControllerStatus.errorStatus; // 保存上次错误状态

            powerControllerStatus.errorStatus.overVoltage    = 0;
            powerControllerStatus.errorStatus.underVoltage   = 0;
            powerControllerStatus.errorStatus.lowEfficency   = 0;
            powerControllerStatus.errorStatus.highEfficency  = 0;
            powerControllerStatus.errorStatus.overCurrent    = 0;
            powerControllerStatus.errorStatus.overPhase      = 0;
            powerControllerStatus.errorStatus._10WWaitingForASKTimeout = 0;
            powerControllerStatus.errorStatus._100WRunningASKDisconnect = 0;

            powerControllerStatus.state = PowerControllerState::ChargingBootCap;
        }
    }


    // 接收功率 pid
    if
    (
        powerControllerStatus.state == PowerControllerState::_100WRunning &&
        Communication::isConnected() &&
        Communication::isValid()
    )
    {
        // 有合法包, 接收端 150W 或者 8W 开环
        if(Communication::CommunicationStatus.backwardCommunicationData.requiredPowerSelection == 1)
        {
            powerControllerStatus.targetTxPower += receivePowerPID.computeDelta(
                TARGET_RECEIVER_POWER,
                Communication::CommunicationStatus.backwardCommunicationData.powerFeedback
            ) / POWER_CONTROLLER_LOW_FREQ;
        }
        else
        {
            powerControllerStatus.targetTxPower = 8.0f;
        }

        powerControllerStatus.targetTxPower = CLAMP(powerControllerStatus.targetTxPower, 0.0f, 145.0f);
        LED::blink(0.15f);
        WS2812::blink(0, 0, 20, 0);
        WS2812::blink(1, 0, 20, 0);
    }
}



void runHighFreq()
{
    Analog::decode();
    Communication::decode();

    // 过流保护
    powerControllerStatus.currentErrorStatus.overCurrent = powerControllerStatus.overCurrentDelayedTrigger(Analog::adcData.iTX > OVER_CURRENT_THRESHOLD);
    powerControllerStatus.currentErrorStatus.overPhase = powerControllerStatus.overPhaseDelayedTrigger(HRTIM::hrtimStatus.fPhase > OVER_PHASE_THRESHOLD);

    if
    (
        powerControllerStatus.currentErrorStatus.overCurrent ||
        powerControllerStatus.currentErrorStatus.overPhase
    )
    {
        HRTIM::setfPhase(0.0f);
        if(powerControllerStatus.currentErrorStatus.overPhase)
        {
            powerControllerStatus.overPhaseDelayedTrigger.reset();
        }

        HRTIM::disableOutput();
        powerControllerStatus.errorStatus.overCurrent   |= powerControllerStatus.currentErrorStatus.overCurrent;
        powerControllerStatus.errorStatus.overPhase     |= powerControllerStatus.currentErrorStatus.overPhase;
        powerControllerStatus.noErrorCounter = 0;
        Buzzer::play({200.0f, 250.0f, 100.0f, 0.6f});
        LED::off();
        WS2812::blink(0, 20, 0, 0);
        WS2812::blink(1, 20, 0, 0);
        powerControllerStatus.state = PowerControllerState::Stop;
    }

    if(
        powerControllerStatus.state == PowerControllerState::Detecting ||
        powerControllerStatus.state == PowerControllerState::_100WRunning ||
        powerControllerStatus.state == PowerControllerState::_10WWaitingForASK
    )
    {
        powerControllerStatus.setPhase += powerPID.computeDelta(powerControllerStatus.targetTxPower, Analog::adcData.pTX) / POWER_CONTROLLER_HIGH_FREQ;
        if(powerControllerStatus.state == PowerControllerState::Detecting)
        {
            powerControllerStatus.setPhase = CLAMP(powerControllerStatus.setPhase, 0.0f, DETECTING_PHASE);
        }
        HRTIM::setfPhase(powerControllerStatus.setPhase);
    }

}
} // namespace PowerController