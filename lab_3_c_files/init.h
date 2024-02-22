#ifndef INIT_H
#define INIT_H

#include "FreeRTOS.h"
#include "stm32l476xx.h"
#include "system_stm32l4xx.h"
#include "task.h"
#include "timers.h"
#include "stdint.h"
#include "queue.h"

#define BufferSize 8

static QueueHandle_t but_led_queue;
static QueueHandle_t but_tim_queue;
//static uint8_t buffer[1];

void LED_task(void *pvParameters);
void Button_task(void *pvParameters);

void clock_Config(void);
void gpio_Config(void);

void timer_Config(void);
void DAC_Config(void);
void TIM4_IRQHandler(void);

void UART_config(void);
void USART_read(uint8_t uart_buffer);
void USART2_IRQHandler(void);
void change_note(uint8_t uart_buffer);

#endif