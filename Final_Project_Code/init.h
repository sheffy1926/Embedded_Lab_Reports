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
static QueueHandle_t uart2_to_uart3_queue;
static QueueHandle_t vol_queue; 
static TaskHandle_t note_handle;
static TaskHandle_t vol_handle;
static SemaphoreHandle_t uart_mutex;

void LED_task(void *pvParameters);
void Button_task(void *pvParameters);
void note_task(void *pvParameters);
void volume_task(void *pvParameters);

void clock_Config(void);
void gpio_Config(void);

void timer_Config(void);
void DAC_Config(void);
void TIM4_IRQHandler(void);

void USART2_config(void);
//void USART2_IRQHandler(void);
void USART2_write(uint8_t note,float volume[1],uint32_t change);
void change_note(uint8_t uart_buffer);

void USART3_config(void);
void USART3_write(uint8_t measure_type);

void UART4_config(void);
void UART4_write(uint8_t measure_type);

#endif