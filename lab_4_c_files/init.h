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
static TaskHandle_t prox_handle;
//static QueueHandle_t uart3_prox_sensor_queue;

void LED_task(void *pvParameters);
void Button_task(void *pvParameters);
void prox_sensor_task(void *pvParameters);

void clock_Config(void);
void gpio_Config(void);

void timer_Config(void);
void DAC_Config(void);
void TIM4_IRQHandler(void);

void USART2_config(void);
void USART2_IRQHandler(void);
void USART2_write(uint16_t measurement,uint8_t uart_buffer[1]);
void change_note(uint8_t uart_buffer);

void USART3_config(void);
void USART3_write(uint8_t measure_type);
//void USART3_read(void);

#endif