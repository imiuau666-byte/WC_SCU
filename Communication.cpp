/**
 * @file Communication.cpp
 * @author Baoqi (zzhongas@connect.ust.hk); Xian Ziming (zxianaa@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "Communication.hpp"
#include "AnalogWrapper.hpp"

namespace Communication
{
CommunicationStatus_t CommunicationStatus;

const uint8_t stratSequense[] = {0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1};
const uint8_t stratSequenseLength = sizeof(stratSequense) / sizeof(stratSequense[0]);

void decode20BitsBuffer()
{
    CommunicationStatus.disconnectCounter   = 0;
    CommunicationStatus.isConnected         = 1;

    // 两个原始 bit 对应 4bit, 这四个 bit 的 bit[1] bit[2] 必然不同, 否则就是寄了
    for(uint8_t i = 0; i < 9; i++)
    {
        if(CommunicationStatus.data20Bits[i * 2 + 1] == CommunicationStatus.data20Bits[i * 2 + 2])
        {
            // bit 间无跳变
            CommunicationStatus.isValid = 0;
            return;
        }
    }

    // 两个 bit 合二为一
    for(uint8_t i = 0; i < 10; i++)
    {
        if(CommunicationStatus.data20Bits[i * 2] == CommunicationStatus.data20Bits[i * 2 + 1])
        {
            CommunicationStatus.raw10Bit[i] = 0;
        }
        else
        {
            CommunicationStatus.raw10Bit[i] = 1;
        }
    }

    uint8_t requiredPowerSelection  = CommunicationStatus.raw10Bit[0];
    uint8_t rawPowerFeedback        = 0;
    float powerFeedback             = 0.0f;
    float efficiency                = 0.0f;

    for(uint8_t i = 0; i < 8; i++)
    {
        rawPowerFeedback |= (CommunicationStatus.raw10Bit[i + 1] << i);
    }

    uint16_t parity = requiredPowerSelection | (rawPowerFeedback << 1);
    parity ^= (parity >> 8);
    parity ^= (parity >> 4);
    parity ^= (parity >> 2);
    parity ^= (parity >> 1);
    if((parity & 0x01) != CommunicationStatus.raw10Bit[9])
    {
        // 奇偶校验错误
        CommunicationStatus.isValid = 0;
        return;
    }

    powerFeedback = (float)rawPowerFeedback / 255.0f * 150.0f;
    efficiency = powerFeedback / Analog::adcData.pTX;
    if(efficiency < 0.2f || efficiency > 0.9f)
    {
        // 效率异常
        CommunicationStatus.isValid = 0;
        return;
    }


    CommunicationStatus.backwardCommunicationData.requiredPowerSelection    = requiredPowerSelection;
    CommunicationStatus.backwardCommunicationData.rawPowerFeedback          = rawPowerFeedback;
    CommunicationStatus.backwardCommunicationData.powerFeedback             = powerFeedback;
    CommunicationStatus.backwardCommunicationData.transmitEfficiency =
        efficiency * 0.5f +
        CommunicationStatus.backwardCommunicationData.transmitEfficiency * 0.5f;

    CommunicationStatus.isValid  = 1;
}

void newBitCome(uint8_t lastLevel)
{
    if(CommunicationStatus._40BitsBufferPointer < stratSequenseLength)
    {
        // 如果当前 bit 是 header, 则需要匹配 header.
        if(stratSequense[CommunicationStatus._40BitsBufferPointer] == lastLevel)
        {
            CommunicationStatus._40BitsBufferPointer += 1;
        }
        else
        {
            CommunicationStatus._40BitsBufferPointer = 0;
        }
    }
    else
    {
        // 不是 header, 直接记录
        CommunicationStatus.data20Bits[CommunicationStatus._40BitsBufferPointer - stratSequenseLength] = lastLevel;
        CommunicationStatus._40BitsBufferPointer += 1;
    }
    if(CommunicationStatus._40BitsBufferPointer == 40)
    {
        CommunicationStatus._40BitsBufferPointer = 0;
        decode20BitsBuffer();
    }
}

void commuResultFlipCallback(uint8_t lastLevel)
{
    uint32_t dt = __HAL_TIM_GetCounter(&COMMUNICATION_TIMER);   // 获取计数器值
    __HAL_TIM_SET_COUNTER(&COMMUNICATION_TIMER, 0);             // 无论后续解码如何都重置计数器

    if(dt < 125 || dt > 625)
    {
        // 采样到错误的时间间隔
        CommunicationStatus._40BitsBufferPointer = 0;
        return;
    }

    // 250us 为 1 的翻转半周期
    // 500us 为 0 的翻转半周期
    if(dt < 375)
    {
        // 250us 小周期翻转, 不管是前半部分还是后半部分都记录
        newBitCome(lastLevel);
    }
    else
    {
        newBitCome(lastLevel);
        newBitCome(lastLevel);
    }
}

// 调用频率: High Freq
void decode()
{
    for(uint8_t i = 0; i < CommunicationStatus.communicationBufferLength; i++)
    {
        // 新 ADC 值突破当前 Dynamic Min/Max 就带滤波地更新, 否则衰减
        if(CommunicationStatus.communicationBuffer[i] > CommunicationStatus.dynamicMax)
        {
            CommunicationStatus.dynamicMax = CommunicationStatus.dynamicMax * 0.2f + CommunicationStatus.communicationBuffer[i] * 0.8f;
        }
        else
        {
            CommunicationStatus.dynamicMax = (COMMUNICATION_MIDDLE_THRESHOLD + 200) * 0.003f + CommunicationStatus.dynamicMax * 0.997f;
        }

        if(CommunicationStatus.communicationBuffer[i] < CommunicationStatus.dynamicMin)
        {
            CommunicationStatus.dynamicMin = CommunicationStatus.dynamicMin * 0.2f + CommunicationStatus.communicationBuffer[i] * 0.8f;
        }
        else
        {
            CommunicationStatus.dynamicMin = (COMMUNICATION_MIDDLE_THRESHOLD - 200) * 0.005f + CommunicationStatus.dynamicMin * 0.995f;
        }

        // 把 threshold 设置为 dynamicMin/Max 的 70%
        CommunicationStatus.upperThreshold = COMMUNICATION_MIDDLE_THRESHOLD * 0.3f + CommunicationStatus.dynamicMax * 0.7f;
        CommunicationStatus.lowerThreshold = COMMUNICATION_MIDDLE_THRESHOLD * 0.3f + CommunicationStatus.dynamicMin * 0.7f;

        if(CommunicationStatus.upperDelayedTrigger(CommunicationStatus.communicationBuffer[i] > CommunicationStatus.upperThreshold))
        {
            CommunicationStatus.lowerDelayedTrigger.reset();
            if(CommunicationStatus.currentBitLevel == 0)
            {
                CommunicationStatus.currentBitLevel = 1;
                commuResultFlipCallback(1);     // 实测发送和接收电平是反向的
            }
        }
        if(CommunicationStatus.lowerDelayedTrigger(CommunicationStatus.communicationBuffer[i] < CommunicationStatus.lowerThreshold))
        {
            CommunicationStatus.upperDelayedTrigger.reset();
            if(CommunicationStatus.currentBitLevel == 1)
            {
                CommunicationStatus.currentBitLevel = 0;
                commuResultFlipCallback(0);     // 实测发送和接收电平是反向的
            }
        }
    }

    if(CommunicationStatus.disconnectCounter < COMMUNICATION_DISCONNECT_TIMEOUT * POWER_CONTROLLER_HIGH_FREQ / POWER_CONTROLLER_LOW_FREQ)
    {
        CommunicationStatus.disconnectCounter ++;
        if(CommunicationStatus.disconnectCounter == COMMUNICATION_DISCONNECT_TIMEOUT * POWER_CONTROLLER_HIGH_FREQ / POWER_CONTROLLER_LOW_FREQ)
        {
            // 反向通信丢失, 断开连接
            CommunicationStatus.isConnected = 0;
            CommunicationStatus.isValid = 0;
        }
    }
}

void init(uint16_t* communicationBuffer, uint8_t communicationBufferLength)
{
    CommunicationStatus.communicationBuffer = communicationBuffer;
    CommunicationStatus.communicationBufferLength = communicationBufferLength;

    HAL_TIM_Base_Start(&COMMUNICATION_TIMER);
}

// 收到 Header 就算是 Connected
uint8_t isConnected()
{
    return CommunicationStatus.isConnected;
}

// Valid 是在 connected 之上的加强
uint8_t isValid()
{
    return CommunicationStatus.isValid;
}
}