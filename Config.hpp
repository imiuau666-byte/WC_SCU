/**
 * @file Config.hpp
 * @author Baoqi (zzhongas@connect.ust.hk); Xian Ziming (zxianaa@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#pragma once
#include "tim.h"

/* System */
#define LOAD_TIMER                  htim2                   /* 负载检测 timer */

/* HRTIM */
#define HRTIM_PERIOD                37778                   /* 144 KHz */

#define HRTIM_PRESCALER             4

/* ADC */
#define ADC_WINDOW_SIZE             34

#define ADC1_CHANNEL_NUM            2
#define ADC2_CHANNEL_NUM            1
#define ADC1_BUFFER_SIZE            HRTIM_PRESCALER         /* ADC1 反向通信的 buffer 大小 */
#define ADC2_BUFFER_SIZE            HRTIM_PRESCALER         /* ADC2 电流电压平均次数 */


/* 改内部参考电压要重新校准 */
#define VIN_ADC_BAIS                0.0f
#define VIN_ADC_GAIN                0.016095f
// 备用板
// #define ISENSE_ADC_BAIS             2028.3f          // 1 号板
// #define ISENSE_ADC_BAIS             2019.89f        // 2 号板
#define ISENSE_ADC_BAIS             2043.6f        // 3 号板
#define ISENSE_ADC_GAIN             0.008f

#define ISENSE_FILTER_ALPHA         0.1f
#define VIN_FILTER_ALPHA            0.6f

#define UNDER_VOLTAGE_THRESHOLD     5.0f                   /* 低压阈值, 单位 V */
#define OVER_VOLTAGE_THRESHOLD      30.0f                   /* 过压阈值, 单位 V */
#define OVER_CURRENT_THRESHOLD      8.0f                    /* 过流阈值, 单位 A */
#define OVER_PHASE_THRESHOLD        0.7f                    /* 占空比过高保护阈值, 单位 % */

#define UNDER_VOLTAGE_SHUTDOWM_TIME 10                      /* 低压保护延时, 单位 ms */
#define OVER_VOLTAGE_SHUTDOWM_TIME  10                      /* 过压保护延时, 单位 ms */
#define OVER_CURRENT_SHUTDOWM_TIME  1                       /* 过流保护延时, 单位 ms */
#define OVER_PHASE_SHUTDOWM_TIME    1                       /* 占空比过高保护延时, 单位 ms */

/* Power Controller */
#define POWER_CONTROLLER_LOW_FREQ   1000
#define POWER_CONTROLLER_HIGH_FREQ  36000

#define DETECTING_PHASE             0.17f                   /* 静态检测时的励磁电流的 phase */
#define DETECT_THRESHOLD_POWER      3.0f                    /* 检测的功率阈值 */
#define DETECTED_TIME_THRESHOLD     100                     /* 转换到大功率的延时时间, 单位 ms */

#define LOW_EFFICENCY_THRESHOLD     0.2f                    /* 低效率保护阈值, 单位 % */
#define HIGH_EFFICENCY_THRESHOLD    2.0f                    /* 高效率保护阈值, 单位 % */
#define LOW_EFFICENCY_TIMEOUT       1000                    /* 低效率保护延时, 单位 ms */
#define HIGH_EFFICENCY_TIMEOUT      100                     /* 高效率保护延时, 单位 ms */

#define AUTO_RECOVER_TIMEOUT        2000

#define TARGET_RECEIVER_POWER       115.0f


/* Communication */
#define COMMUNICATION_TIMER             htim8
#define COMMUNICATION_UPPER_THRESHOLD   300                 /* 反向通信上限阈值, 以 middle threshold 为基准, 单位 ADC LSB */
#define COMMUNICATION_LOWER_THRESHOLD   300                 /* 反向通信下限阈值, 以 middle threshold 为基准, , 单位 ADC LSB */
#define COMMUNICATION_MIDDLE_THRESHOLD  2015
#define COMMUNICATION_TRIGGER_TIMEOUT   2                   /* 连续触发时间阈值, 单位高频 tick */

#define COMMUNICATION_DISCONNECT_TIMEOUT 20                 /* 反向通信合法包丢失 timeout, 单位 ms */


/* WS2812 */
#define WS2812_TIM                  htim4
#define WS2812_TIM_CHANNEL          TIM_CHANNEL_4
#define WS2812_LED_NUM              2
#define WS2812_TIM_PWMN             0

/* White LED */
#define WHITE_LED_TIM              htim5
#define WHITE_LED_TIM_CHANNEL      TIM_CHANNEL_2

/* Buzzer */
#define NOTE_QUEUE_LENGTH           10
#define BUZZER_HANDLER_FREQ         1000

#define BUZZER_TIM                  htim3
#define BUZZER_TIM_CHANNEL          TIM_CHANNEL_4
#define BUZZER_TIM_CLOCK_FREQ       170000000



/* 常数和基本函数 */
#define PI                          3.1415926f
#define TWO_PI                      6.2831853f
#define ONE_OVER_SQRT3              0.5773503f
#define TWO_OVER_SQRT3              1.1547005f
#define SQRT3                       1.7320508f
#define SQRT3_OVER_2                0.8660254f


#define CLAMP(x, min, max) ((x) > (max) ? (max) : ((x) < (min) ? (min) : (x)))

#define FABS(x)     ((x) > 0 ? (x) : -(x))
#define MIN(a,b)    ((a)<(b)?(a):(b))
#define MAX(a,b)    ((a)>(b)?(a):(b))
#define FMOD(x, y)  ((x) - (int)((x) / (y)) * (y))
