#include "FreeRTOS.h"
#include "stm32l476xx.h"
#include "system_stm32l4xx.h"
#include "task.h"
#include "timers.h"
#include "stdint.h"
#include "queue.h"
#include "stdio.h"

#include "init.h"

int main(void) {
	//Initialize System
	SystemInit(); 
	clock_Config();
	but_led_queue = xQueueCreate(2,sizeof(uint8_t));
	but_tim_queue = xQueueCreate(2,sizeof(uint8_t));
	uart2_to_uart3_queue = xQueueCreate(4,sizeof(uint8_t));
	//uart3_prox_sensor_queue = xQueueCreate(8,sizeof(uint8_t));
	gpio_Config();
	USART2_config();
	USART3_config();
	timer_Config();
	DAC_Config();

		//Set the priority for the interrupts
	NVIC_SetPriority(USART2_IRQn,0x07);
	NVIC_SetPriority(TIM4_IRQn,0x07);
	
	//Task for LED
	if(xTaskCreate(LED_task, "LED", 32, NULL, 2, NULL) != pdPASS){
		while(1);
	}
	//Task for Button
  if(xTaskCreate(Button_task, "Button", 32, NULL, 2, NULL) != pdPASS){
		while(1);
	}
	
	if(xTaskCreate(prox_sensor_task, "Proximity Sensor", 256, NULL, 2, &prox_handle) != pdPASS){
		while(1);
	}
	
	//Start Task Scheduler
	vTaskStartScheduler();
	while(1);
}

//Function to toggle led_state
void LED_task(void *pvParameters){
	static uint8_t buffer[1];
	buffer[0] = 0;
	while(1){
		if(uxQueueMessagesWaiting(but_led_queue) > 0){
			if(xQueueReceive(but_led_queue,buffer,50)== pdTRUE){}
		}
		//If the LED is off turn it on
		if(buffer[0] == 1){
			GPIOA->BSRR |= GPIO_BSRR_BS5;
		} 
		//If the LED is on turn it off
		else if(buffer[0] == 0){
			GPIOA->BSRR |= GPIO_BSRR_BR5;
		}
	}
}

//Function to read in button state and led_state
void Button_task(void *pvParameters){
	static uint8_t buffer[1];
	buffer[0] = 0;
	uint32_t button_in;
	while(1){
		//Read in the value of the button
		button_in = GPIOC->IDR;
		button_in &= GPIO_IDR_ID13_Msk;

		//If the button is pressed toggle the LED
		if(button_in == 0){
			while(button_in == 0){
				button_in = GPIOC->IDR;
				button_in &= GPIO_IDR_ID13_Msk;
			}
			if(buffer[0] == 0){
				buffer[0] = 1;
				//Send led_state to queue for LED Task
				xQueueSendToBack(but_led_queue,buffer,50);
				//Send led_state to queue for TIM4_IRQHandler
				xQueueSendToBack(but_tim_queue,buffer,50);
			}
			else {
				buffer[0] = 0;
				//Send led_state to queue for LED Task
				xQueueSendToBack(but_led_queue,buffer,50);
				//Send led_state to queue for TIM4_IRQHandler
				xQueueSendToBack(but_tim_queue,buffer,50);
			}
		}
	}
}

void TIM4_IRQHandler(void){
	static uint32_t sine_count = 0;
	static uint8_t buffer[1];
	
	const uint16_t sineLookupTable[] = {
	305, 335, 365, 394, 422, 449, 474, 498, 521, 541, 559, 574, 587, 597, 604,
	609, 610, 609, 604, 597, 587, 574, 559, 541, 521, 498, 474, 449, 422, 394,
	365, 335, 305, 275, 245, 216, 188, 161, 136, 112, 89, 69, 51, 36, 23,
	13, 6, 1, 0, 1, 6, 13, 23, 36, 51, 69, 89, 112, 136, 161,
	188, 216, 245, 275};
	
	//if there is a message waiting in the queue from ISR
	if(uxQueueMessagesWaitingFromISR(but_tim_queue) > 0){
		xQueueReceiveFromISR(but_tim_queue,buffer,NULL);
	}
	//if the LED is on
	if (buffer[0] == 1){
		sine_count++; //Increment to the next value in the table
		if (sine_count == 64){
			sine_count = 0;
		}
	}
	//Assign DAC to Sine_Wave Table Current Value
	DAC->DHR12R1 = sineLookupTable[sine_count] + 45;
	TIM4->SR &= ~TIM_SR_UIF; //Clears Interrupt Flag
}

void USART2_IRQHandler(void){
	uint8_t uart_buffer[1];
	uart_buffer[0] = (uint8_t)(USART2->RDR);	//Get serial data
	change_note(uart_buffer[0]);
	//If uart_buffer = t or p send into the queue
	if (uart_buffer[0] == 't' || uart_buffer[0] == 'p'){
		xQueueSendToBackFromISR(uart2_to_uart3_queue,uart_buffer,NULL);
	}
}

void change_note(uint8_t uart_buffer){
	if(uart_buffer == 'a'){
		TIM4->ARR = 0xFFFF008E; //2 MHz/(142) = 14.080 kHz interrupt rate; 220 Hz sine wave
	}
	else if (uart_buffer == 'b'){
		TIM4->ARR = 0xFFFF007E; //126; 246.94 Hz
	}
	else if (uart_buffer == 'c'){
		TIM4->ARR = 0xFFFF0077; //119; 261.63 Hz
	}
	else if (uart_buffer == 'd'){
		TIM4->ARR = 0xFFFF006A; //106; 293.66 Hz
	}
	else if (uart_buffer == 'e'){
		TIM4->ARR = 0xFFFF005E;	//94; 329.63 Hz
	}
	else if (uart_buffer == 'f'){
		TIM4->ARR = 0xFFFF0059; //89; 349.23 Hz
	}
	else if (uart_buffer == 'g'){
		TIM4->ARR = 0xFFFF004F; //79; 392.00 Hz
	}
	else if (uart_buffer == 'h'){
		TIM4->ARR = 0xFFFF0046; //71; 440 Hz (High A)
	}	
}

void prox_sensor_task(void *pvParameters){
	uint8_t uart_buffer[1];
	uint16_t measurement;
	
	while(1){
		if(uxQueueMessagesWaiting(uart2_to_uart3_queue) > 0){
			if(xQueueReceive(uart2_to_uart3_queue,uart_buffer,50)== pdTRUE){
				vTaskPrioritySet(prox_handle,3);
				if (uart_buffer[0] == 't'){
					USART3_write(0x50);
					while (!(USART3->ISR & USART_ISR_RXNE)); //Wait until hardware sets RXNE
					measurement = USART3->RDR;
					USART2_write(measurement,uart_buffer);
				}
				else if (uart_buffer[0] == 'p'){
					USART3_write(0x55);
					while (!(USART3->ISR & USART_ISR_RXNE)); //Wait until hardware sets RXNE
					measurement = USART3->RDR;
					measurement *= 256;
					while (!(USART3->ISR & USART_ISR_RXNE)); //Wait until hardware sets RXNE
					measurement += USART3->RDR;
					USART2_write(measurement,uart_buffer);
				}
				vTaskPrioritySet(prox_handle,2);
			}
		}
	}
}

void USART3_write(uint8_t measure_type){
	//Send command to measure the temperature or proximity distance
	while(!(USART3->ISR & USART_ISR_TXE)); //Wait until hardware sets TXE
	USART3->TDR = measure_type & 0xFF; //Writing to TDR clears TXE Flag

	//Wait until TC bit is set. TC is set by hardware and cleared by software
	while (!(USART3->ISR & USART_ISR_TC)); //TC: Transmission complete flag

	//Writing 1 to the TCCF bit in ICR clears the TC bit in ICR
	USART3->ICR |= USART_ICR_TCCF; //TCCF: Transmission complete clear flag 
}

/*void USART3_read(void){
	//Read in the temperature or proximity distance measurements
	int nBytes = 2;
	uint8_t measurement[2] = {0};
	for(int i=0; i < nBytes; i++){
		
		measurement[i] = (uint8_t)(USART3->RDR);
	}
	//Send measurement reading to prox_sensor_task
	xQueueSendToBack(uart3_prox_sensor_queue,measurement,50);
}*/

void USART2_write(uint16_t measurement,uint8_t uart_buffer[1]){
	int nBytes = 18;
	char serial_message[18] = {0};
	float inches = 0;

	//Produce appropriate string for temp or proximity measurement received
	if (uart_buffer[0] == 't'){
		sprintf(serial_message,"%i deg F\n\r",measurement);
	}
	else if (uart_buffer[0] == 'p'){
		inches = (float)(measurement / 25.4);
		sprintf(serial_message,"%.2f inches\n\r",(double)inches);
	}
	//Send Serial message to USART2 to send to serial terminal
	for (int i=0; i < nBytes; i++){
		while(!(USART2->ISR & USART_ISR_TXE)); //Wait until hardware sets TXE
		USART2->TDR = serial_message[i] & 0xFF; //Writing to TDR clears TXE Flag
	}
	//Wait until TC bit is set. TC is set by hardware and cleared by software
	while (!(USART2->ISR & USART_ISR_TC)); //TC: Transmission complete flag

	//Writing 1 to the TCCF bit in ICR clears the TC bit in ICR
	USART2->ICR |= USART_ICR_TCCF; //TCCF: Transmission complete clear flag 
}
