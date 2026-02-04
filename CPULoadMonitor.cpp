/**
* @file CPULoadMonitor.hpp
 * @author Rong Yi; Xian Ziming (zxianaa@connect.ust.hk); Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "CPULoadMonitor.hpp"


void CPULoadMonitor::init(TIM_HandleTypeDef* htim_)
{
    this->htim = htim_;
    if((this->htim->Instance->CR1 & TIM_CR1_CEN) == 0)
        HAL_TIM_Base_Start(this->htim);

    this->startCounter = __HAL_TIM_GET_COUNTER(this->htim);
}

void CPULoadMonitor::start()
{
    if(this->htim == nullptr)
        return;
    this->lastStartCounter = this->startCounter;
    this->startCounter = __HAL_TIM_GET_COUNTER(this->htim);
}

void CPULoadMonitor::stop()
{
    if(this->htim == nullptr)
        return;

    this->periodDeltaCounter = 1;    // 两次调用 start() 之间的 delta counter
    this->loadDeltaCounter = 1;      // start() 和 stop() 之间的 delta counter

    uint32_t currentCounter = __HAL_TIM_GET_COUNTER(this->htim);

    if(this->startCounter < this->lastStartCounter)
    {
        this->periodDeltaCounter = __HAL_TIM_GET_AUTORELOAD(this->htim) - (this->lastStartCounter - this->startCounter);
    }
    else
    {
        this->periodDeltaCounter = this->startCounter - this->lastStartCounter;
    }
    if(currentCounter < this->startCounter)
    {
        this->loadDeltaCounter = __HAL_TIM_GET_AUTORELOAD(this->htim) - (this->startCounter - currentCounter);
    }
    else
    {
        this->loadDeltaCounter = currentCounter - this->startCounter;
    }
    this->load =
        (float)this->loadDeltaCounter / this->periodDeltaCounter * this->loadAlpha +
        this->load * (1 - this->loadAlpha);
}