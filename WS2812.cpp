/**
 * @file WS2812.cpp
 * @author Guo Zilin; Baoqi (zzhongas@connect.ust.hk); Xian Ziming (zxianaa@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "WS2812.hpp"

#include "string.h"

namespace WS2812
{
typedef struct
{
    RGB rgbs[WS2812_LED_NUM];
    unsigned char updatedFlag = 0;
    unsigned char txFlag = 0;
} RGBStatus;

static volatile RGBStatus rgbStatus;

void PWM_DMA_TransmitFinshed_Callback(TIM_HandleTypeDef *htim)
{
    if (htim == &WS2812_TIM)
    {
        #if (WS2812_TIM_PWMN)
        HAL_TIMEx_PWMN_Stop_DMA(htim, WS2812_TIM_CHANNEL);
        #else
        HAL_TIM_PWM_Stop_DMA(htim, WS2812_TIM_CHANNEL);
        #endif
        rgbStatus.txFlag = 0;
    }
}

static unsigned int txFailCnt = 0;
static long unsigned int CCRDMABuff[WS2812_LED_NUM * sizeof(RGB) * 8 + 1];
void PWM_DMA_TransmitError_Callback(TIM_HandleTypeDef *htim)
{
    if (htim == &WS2812_TIM)
    {
        #if (WS2812_TIM_PWMN)
        HAL_TIMEx_PWMN_Stop_DMA(htim, WS2812_TIM_CHANNEL);
        #else
        HAL_TIM_PWM_Stop_DMA(htim, WS2812_TIM_CHANNEL);
        #endif
        if (++txFailCnt < 10)
        {
            #if (WS2812_TIM_PWMN)
            HAL_TIMEx_PWMN_Start_DMA(&WS2812_TIM, WS2812_TIM_CHANNEL, CCRDMABuff, WS2812_LED_NUM * sizeof(RGB) * 8 + 1);
            #else
            HAL_TIM_PWM_Start_DMA(&WS2812_TIM, WS2812_TIM_CHANNEL, CCRDMABuff, WS2812_LED_NUM * sizeof(RGB) * 8 + 1);
            #endif
        }
        else
        {
            txFailCnt        = 0;
            rgbStatus.txFlag = 0;
        }
    }
}

void init()
{
    HAL_TIM_RegisterCallback(&WS2812_TIM, HAL_TIM_PWM_PULSE_FINISHED_CB_ID, PWM_DMA_TransmitFinshed_Callback);
    HAL_TIM_RegisterCallback(&WS2812_TIM, HAL_TIM_ERROR_CB_ID, PWM_DMA_TransmitError_Callback);

    blankAll();
}

void transmit()
{
    if (!rgbStatus.updatedFlag)
        return;
    if (rgbStatus.txFlag)
        return;
    rgbStatus.txFlag = 1;
    unsigned int data;

    /*Pack the RGB Value*/
    for (unsigned int i = 0; i < WS2812_LED_NUM; i++)
    {
        data = *(unsigned volatile int *)(&rgbStatus.rgbs[i]);
        for (unsigned int j = 0; j < sizeof(RGB) * 8; j++)
            CCRDMABuff[i * sizeof(RGB) * 8 + j] = ((1UL << (23 - j)) & data) ? BIT1_WIDTH : BIT0_WIDTH;
    }
    CCRDMABuff[WS2812_LED_NUM * sizeof(RGB) * 8] = 0;

    /*Transmit DMA*/
    #if (WS2812_TIM_PWMN)
    HAL_TIMEx_PWMN_Start_DMA(&WS2812_TIM, WS2812_TIM_CHANNEL, CCRDMABuff, WS2812_LED_NUM * sizeof(RGB) * 8 + 1);
    #else
    HAL_TIM_PWM_Start_DMA(&WS2812_TIM, WS2812_TIM_CHANNEL, CCRDMABuff, WS2812_LED_NUM * sizeof(RGB) * 8 + 1);
    #endif
    rgbStatus.updatedFlag = 0;
}

void blink(int index, unsigned char r, unsigned char g, unsigned char b)
{
    if (index < 0 || index >= WS2812_LED_NUM)
        return;
    if (rgbStatus.txFlag)  // Could not modify if it is sending message
        return;
    rgbStatus.updatedFlag = 1;
    volatile RGB *pRgb    = &rgbStatus.rgbs[index];
    pRgb->blue            = b;
    pRgb->green           = g;
    pRgb->red             = r;
}

void blank(int index)
{
    blink(index, 0, 0, 0);
}

void blankAll()
{
    for (int i = 0; i < WS2812_LED_NUM; i++)
        blank(i);
}

} // namespace WS2812