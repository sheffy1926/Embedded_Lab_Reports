#ifndef INIT_H
#define INIT_H

#include "FreeRTOS.h"
#include "stm32l476xx.h"
#include "system_stm32l4xx.h"
#include "task.h"
#include "timers.h"
#include "stdint.h"

void LED_task(void *pvParameters);
void Button_task(void *pvParameters);

void clock_Config(void);
void gpio_Config(void);
void timer_Config(void);
void DAC_Config(void);

void TIM4_IRQHandler(void);

#endif