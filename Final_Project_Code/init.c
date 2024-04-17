#include "FreeRTOS.h"
#include "stm32l476xx.h"
#include "system_stm32l4xx.h"
#include "task.h"
#include "timers.h"
#include "stdint.h"
#include "queue.h"
#include "semphr.h"

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
	
	TIM4->CR1 &= ~TIM_CR1_CMS;    // Edge-aligned mode
	TIM4->CR1 &= ~TIM_CR1_DIR;    // Up-counting
	
	TIM4->CR2 &= ~TIM_CR2_MMS;    // Select master mode
	TIM4->CR2 |= TIM_CR2_MMS_2;    // 100 = OC1REF as TRGO
	
	TIM4->DIER |= TIM_DIER_TIE;    // Trigger interrupt enable
	TIM4->DIER |= TIM_DIER_UIE;    // Update interrupt enable
	
	TIM4->CCMR1 &= ~TIM_CCMR1_OC1M;
	TIM4->CCMR1 |= (TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2);  // 0110 = PWM mode 1
	
	TIM4->PSC = 0x7;      		// 16 MHz / (7+1) = 2 MHz timer ticks
	TIM4->ARR = 0xFFFF008E; 	// 2 MHz / (141+1) = 14.080 kHz interrupt rate; 64 entry look-up table = 220 Hz sine wave
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

void USART2_config(void){
	//Enable PA2 (TX) and PA3 (RX) to alternate function mode
	GPIOA->MODER &= ~(0xF << (2*2));
	GPIOA->MODER |= (0xA << (2*2));
	//Enable alternate function for USART2 for the GPIO pins
	GPIOA->AFR[0] |= 0x77 << (4*2); //set pin 2 and 3 to AF7
	//High Speed mode
	GPIOA->OSPEEDR |= 0xF<<(2*2);
	//Pull up mode for PA3 RX
	GPIOA->PUPDR &= ~(0xF<<(2*2));
	GPIOA->PUPDR |= 0x5<<(2*2); //Select pull-up
	//GPIO Output type: 0 = push-pull
	GPIOA->OTYPER &= ~(0x3<<2);
	
	//Enable clk for USART2
	RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN; 
	//Select system clock for USART2
  RCC->CCIPR &= ~RCC_CCIPR_USART2SEL_0;
  RCC->CCIPR |= RCC_CCIPR_USART2SEL_1;
	
	//Disable USART2
	USART2->CR1 &= ~USART_CR1_UE;
	//set data length to 8 bits
	USART2->CR1 &= ~USART_CR1_M;
	//select 1 stop bit
	USART2->CR2 &= ~USART_CR2_STOP;
	//Set parity control as no parity
	USART2->CR1 &= ~USART_CR1_PCE;
	//Oversampling to 16
	USART2->CR1 &= ~USART_CR1_OVER8;
	//Set up Baud rate for USART to 9600 Baud
	USART2->BRR = 0x683;  		//1D4C
	//USART2 Enable Receiver and transmitter
	USART2->CR1 |= (USART_CR1_TE | USART_CR1_RE);
	
	//Enable interrupt for USART2
	NVIC_EnableIRQ(USART2_IRQn);
	//Enables interrupts for USART RX
	USART2->CR1 |= USART_CR1_RXNEIE;	
	
	//Enable USART2 
	USART2->CR1 |= USART_CR1_UE;
	
	//Verify that USART2 is ready for transmission
	while ((USART2->ISR & USART_ISR_TEACK) == 0);
	//Verify that USART2 is ready for reception
	while ((USART2->ISR & USART_ISR_REACK) == 0);
}

void USART3_config(void){
	//Enable PC4 (TX) and PC5 (RX) to alternate function mode
	GPIOC->MODER &= ~GPIO_MODER_MODE4_0;
	GPIOC->MODER |= GPIO_MODER_MODE4_1;
	GPIOC->MODER &= ~GPIO_MODER_MODE5_0;
	GPIOC->MODER |= GPIO_MODER_MODE5_1;
	//Enable alternate function for USART3 for the GPIO pins
	GPIOC->AFR[0] |= GPIO_AFRL_AFSEL4; 		//set pin 4 to AF7
	GPIOC->AFR[0] &= ~GPIO_AFRL_AFSEL4_3; 
	GPIOC->AFR[0] |= GPIO_AFRL_AFSEL5; 		//set pin 5 to AF7
	GPIOC->AFR[0] &= ~GPIO_AFRL_AFSEL5_3;
	//High Speed mode
	GPIOC->OSPEEDR |= 0xF << (2*4);
	//Pull up mode for PC5 RX
	GPIOC->PUPDR &= ~(0xF<<(2*4));
	GPIOC->PUPDR |= 0x5<<(2*4); //Select pull-up
	
	//Enable clk for USART3
	RCC->APB1ENR1 |= RCC_APB1ENR1_USART3EN; 
	//Select system clock for USART3
  RCC->CCIPR &= ~RCC_CCIPR_USART3SEL_0;
  RCC->CCIPR |= RCC_CCIPR_USART3SEL_1;
	
	//Disable USART3
	USART3->CR1 &= ~USART_CR1_UE;
	//set data length to 8 bits
	USART3->CR1 &= ~USART_CR1_M;
	//select 1 stop bit
	USART3->CR2 &= ~USART_CR2_STOP;
	//Set parity control as no parity
	USART3->CR1 &= ~USART_CR1_PCE;
	//Oversampling to 16
	USART3->CR1 &= ~USART_CR1_OVER8;
	//Set up Baud rate for USART3 to 9600 Baud
	USART3->BRR = 0x683;  		
	//USART3 Enable Receiver and transmitter
	USART3->CR1 |= (USART_CR1_TE | USART_CR1_RE);
	
	//Enable USART3 
	USART3->CR1 |= USART_CR1_UE;
	
	//Verify that USART3 is ready for transmission
	while ((USART3->ISR & USART_ISR_TEACK) == 0);
	//Verify that USART3 is ready for reception
	while ((USART3->ISR & USART_ISR_REACK) == 0);
}

void UART4_config(void){
	//Enable PA0 (TX) and PA1 (RX) to alternate function mode
	GPIOA->MODER &= ~GPIO_MODER_MODE0_0;
	GPIOA->MODER |= GPIO_MODER_MODE0_1;
	GPIOA->MODER &= ~GPIO_MODER_MODE1_0;
	GPIOA->MODER |= GPIO_MODER_MODE1_1;
	//Enable alternate function for UART4 for the GPIO pins
	GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL0; 		//set pin 0 to AF8
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL0_3; 
	GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL1; 		//set pin 1 to AF8
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL1_3;
	//High Speed mode
	GPIOA->OSPEEDR |= 0xF;//<<(2*0); 
	//Pull up mode for PA1 RX
	GPIOA->PUPDR   &= ~(0xF);//<<(2*0));	
	GPIOA->PUPDR   |= 0x5;//<<(2*0); //Select pull-up
	
	//Enable clk for UART4
	RCC->APB1ENR1 |= RCC_APB1ENR1_UART4EN; 
	//Select system clock for UART4
  RCC->CCIPR &= ~RCC_CCIPR_UART4SEL_0;
  RCC->CCIPR |= RCC_CCIPR_UART4SEL_1;
	
	//Disable UART4 ---------------------------------------
	UART4->CR1 &= ~USART_CR1_UE;
	//set data length to 8 bits
	UART4->CR1 &= ~USART_CR1_M;
	//select 1 stop bit
	UART4->CR2 &= ~USART_CR2_STOP;
	//Set parity control as no parity
	UART4->CR1 &= ~USART_CR1_PCE;
	//Oversampling to 16
	UART4->CR1 &= ~USART_CR1_OVER8;
	//Set up Baud rate for UART4 to 9600 Baud
	UART4->BRR = 0x683;  		
	//USART3 Enable Receiver and transmitter
	UART4->CR1 |= (USART_CR1_TE | USART_CR1_RE);
	
	//Enable USART4
	UART4->CR1 |= USART_CR1_UE;
	
	//Verify that UART4 is ready for transmission
	while ((UART4->ISR & USART_ISR_TEACK) == 0);
	//Verify that UART4 is ready for reception
	while ((UART4->ISR & USART_ISR_REACK) == 0);
}