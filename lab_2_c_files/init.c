#include "FreeRTOS.h"
#include "stm32l476xx.h"
#include "system_stm32l4xx.h"
#include "task.h"
#include "timers.h"
#include "stdint.h"

#include "init.h"

void clock_Config(void){
	//Change System Clock from MSI to HSI 
	RCC->CR |= RCC_CR_HSION;         // enable HSI (internal 16 MHz clock)
	while ((RCC->CR & RCC_CR_HSIRDY) == 0);
	RCC->CFGR |= RCC_CFGR_SW_HSI;    // make HSI the system clock
	SystemCoreClockUpdate();
	
	//Turn Clock on for GPIOs
	RCC -> AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	//RCC -> AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
	RCC -> AHB2ENR |= RCC_AHB2ENR_GPIOCEN;
}

void gpio_Config(void){
	//Set PA5 to output mode for LED
	GPIOA->MODER &= ~GPIO_MODER_MODE5_1;
	GPIOA->MODER |= GPIO_MODER_MODE5_0;
	//Turn LED on
	GPIOA->BSRR |= GPIO_BSRR_BS5; 			
	//Set PC13 to input mode for Button 
	GPIOC->MODER &= ~GPIO_MODER_MODE13; //0xf3ffffff
}

void timer_Config(void){
	//Turn on Clock for TIM4
	RCC -> APB1ENR1 |= RCC_APB1ENR1_TIM4EN;
	
	//Enable interrupts for TIM4
	NVIC->ISER[0] |= 1 << 30;
	NVIC_EnableIRQ(TIM4_IRQn);
	
	//Enable DMA/Interrupt Handler 
	/*
	TIM4->DIER |= TIM_DIER_TIE;
	TIM4->DIER |= TIM_DIER_UIE;
	
	//Configure Auto-Reload Register
	TIM4->ARR = 0xFFFF023A;
	*/
	
	TIM4->CR1 &= ~TIM_CR1_CMS;    // Edge-aligned mode
	TIM4->CR1 &= ~TIM_CR1_DIR;    // Up-counting
	
	TIM4->CR2 &= ~TIM_CR2_MMS;    // Select master mode
	TIM4->CR2 |= TIM_CR2_MMS_2;    // 100 = OC1REF as TRGO
	
	TIM4->DIER |= TIM_DIER_TIE;    // Trigger interrupt enable
	TIM4->DIER |= TIM_DIER_UIE;    // Update interrupt enable
	
	TIM4->CCMR1 &= ~TIM_CCMR1_OC1M;
	TIM4->CCMR1 |= (TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2);  // 0110 = PWM mode 1
	
	TIM4->PSC = 0x7;      		// 16 MHz / (7+1) = 2 MHz timer ticks
	TIM4->ARR = 0xFFFF0046; 	// 2 MHz / (70+1) = 28.169 kHz interrupt rate; 64 entry look-up table = 440.14 Hz sine wave
	TIM4->CCR1 = 0x23;       	// 50% duty cycle (35)
	TIM4->CCER |= TIM_CCER_CC1E;
	
	//Enable Control Register 1 for Counting
	TIM4->CR1 |= TIM_CR1_CEN;	
}

void DAC_Config(void){
	//Turn on Clock for DAC1
	RCC -> APB1ENR1 |= RCC_APB1ENR1_DAC1EN;
	
	//Configure DAC1 GPIO in Analog Mode 0x3
	GPIOA->MODER |= GPIO_MODER_MODE4;
	
	//Enable DAC1 Channel 1
	DAC->CR |= DAC_CR_EN1;	
}
