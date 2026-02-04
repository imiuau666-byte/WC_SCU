/**
* @file WS2812.cpp
 * @author Guo Zilin; Baoqi (zzhongas@connect.ust.hk); Xian Ziming (zxianaa@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#pragma once
#include "Config.hpp"

#include "main.h"
#include "tim.h"

namespace WS2812
{
#define BIT1_WIDTH 106
#define BIT0_WIDTH 52

    /**
     * @brief The structure that contains the rgb value of a single ws2812 unit
     */
    typedef struct
    {
        unsigned char blue;
        unsigned char red;
        unsigned char green;
    } RGB;

    /**
     * @brief Initialize the ws2812 drivers
     */
    void init();

    /**
     * @brief Blink an LED at the given index by the rgb color
     * @param index The index of the LED
     * @param r     The red value of LED, ranging from 0 to 255
     * @param g     The green value of LED, ranging from 0 to 255
     * @param b     The blue value of LED, ranging from 0 to 255
     * @note  The index should range from 0 to (LED_NUM - 1)
     * @note  The LED can only update once in 1ms. Hence, you might not want to "blink" the LED too frequently.
     */
    void blink(int index, unsigned char r, unsigned char g, unsigned char b);

    void transmit();

    /**
     * @brief Blank the LED at the given index
     * @param index The index of the LED
     * @note  The index should range from 0 to (LED_NUM - 1)
     */
    void blank(int index);

    /**
     * @brief blank all LED
     *
     */
    void blankAll();

} // namespace WS2812