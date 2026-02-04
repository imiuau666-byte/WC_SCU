/**
* @file Communication.hpp
 * @author Baoqi (zzhongas@connect.ust.hk); Xian Ziming (zxianaa@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#pragma once

#include "Config.hpp"
#include "main.h"
#include "stdint.h"

#include "DelayedTrigger.hpp"


namespace Communication
{
    typedef struct
    {
        uint8_t requiredPowerSelection  = 0;        // 用来切换 10W 和 100W 闭环
        uint8_t rawPowerFeedback        = 0;
        float powerFeedback             = 0.0f;     // 换算后的功率, 单位 W
        float transmitEfficiency        = 0.0f;
    } BackwardCommunicationData_t;

    typedef struct
    {
        uint16_t* communicationBuffer;          // 指向 ADC DMA buffer
        uint8_t communicationBufferLength;      // ADC DMA buffer 长度

        uint8_t currentBitLevel         = 0;

        uint8_t _40BitsBufferPointer = 0;       // 指示下一个 write 的位置, 0 - 39
        uint8_t data20Bits[20];                 // Packet body 的 20 bit
        uint8_t raw10Bit[10];                   // 2 bit 合一解码后的发送端原始数据

        BackwardCommunicationData_t backwardCommunicationData;

        float dynamicMax            = COMMUNICATION_MIDDLE_THRESHOLD;
        float dynamicMin            = COMMUNICATION_MIDDLE_THRESHOLD;
        uint16_t upperThreshold     = 2048 + COMMUNICATION_UPPER_THRESHOLD;    // 上限阈值
        uint16_t lowerThreshold     = 2048 - COMMUNICATION_LOWER_THRESHOLD;    // 下限阈值
        DelayedTrigger::DelayedTrigger upperDelayedTrigger  {COMMUNICATION_TRIGGER_TIMEOUT};
        DelayedTrigger::DelayedTrigger lowerDelayedTrigger  {COMMUNICATION_TRIGGER_TIMEOUT};

        // 解码正确才算 connected
        uint32_t disconnectCounter      = 0;
        uint8_t isConnected             = 0;

        uint8_t isValid                 = 0;
    } CommunicationStatus_t;

    extern CommunicationStatus_t CommunicationStatus;

    void init(uint16_t* communicationBuffer, uint8_t communicationBufferLength);

    void decode();

    uint8_t isConnected();

    uint8_t isValid();
}