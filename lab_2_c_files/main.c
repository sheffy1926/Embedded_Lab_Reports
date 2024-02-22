#include "FreeRTOS.h"
#include "stm32l476xx.h"
#include "system_stm32l4xx.h"
#include "task.h"
#include "timers.h"
#include "stdint.h"

#include "init.h"

static uint32_t led_state;

int main(void) {
	//Initialize System
	SystemInit(); 

	clock_Config();
	gpio_Config();
	timer_Config();
	DAC_Config();
	
	led_state = 0;
	
	xTaskCreate( //Task for LED
		LED_task, "LED", 16, NULL, 1, NULL);
	
  xTaskCreate(	//Task for Button
		Button_task, "Button", 16, NULL, 1, NULL);
			
	//Start Task Scheduler
	vTaskStartScheduler();
	while(1);
}

//Function to toggle led_state
void LED_task(void *pvParameters){
	while(1){
		//If the LED is on turn it off
		if(led_state == 1){
			GPIOA->BSRR |= GPIO_BSRR_BS5;
		} 
		//If the LED is off turn it on
		else {
			GPIOA->BSRR |= GPIO_BSRR_BR5;
		}
	}
}

//Function to read in button state and led_state
void Button_task(void *pvParameters){
	while(1){
		uint32_t button_in;
		//Read in the value of the button
		button_in = GPIOC->IDR;
		button_in &= GPIO_IDR_ID13_Msk;

		//If the button is pressed toggle the LED
		if(button_in == 0){
			while(button_in == 0){
				button_in = GPIOC->IDR;
				button_in &= GPIO_IDR_ID13_Msk;
			}
			if(led_state == 0){
				led_state = 1;
			}
			else {
				led_state = 0;
			}
		}
	}
}

void TIM4_IRQHandler(void){
	static uint32_t sine_count = 0;
	
	const uint16_t sineLookupTable[] = {
	305, 335, 365, 394, 422, 449, 474, 498, 521, 541, 559, 574, 587, 597, 604,
	609, 610, 609, 604, 597, 587, 574, 559, 541, 521, 498, 474, 449, 422, 394,
	365, 335, 305, 275, 245, 216, 188, 161, 136, 112, 89, 69, 51, 36, 23,
	13, 6, 1, 0, 1, 6, 13, 23, 36, 51, 69, 89, 112, 136, 161,
	188, 216, 245, 275};
	
	//If the LED is on
	if (led_state == 1){
		sine_count++; //Increment to the next value in the table
		if (sine_count == 64){
			sine_count = 0;
		}
	}
	//Assign DAC to Sine_Wave Table Current Value
	DAC->DHR12R1 = sineLookupTable[sine_count] + 45;
	
	TIM4->SR &= ~TIM_SR_UIF; //Clears Interrupt Flag
}
