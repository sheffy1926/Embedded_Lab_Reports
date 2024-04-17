#include "FreeRTOS.h"
#include "stm32l476xx.h"
#include "system_stm32l4xx.h"
#include "task.h"
#include "timers.h"
#include "stdint.h"
#include "queue.h"
#include "stdio.h"
#include "semphr.h"

#include "init.h"

int main(void) {
	//Initialize System
	SystemInit(); 
	clock_Config();
	but_led_queue = xQueueCreate(2,sizeof(uint8_t));
	but_tim_queue = xQueueCreate(2,sizeof(uint8_t));
	uart2_to_uart3_queue = xQueueCreate(4,sizeof(uint8_t));
	vol_queue = xQueueCreate(2,sizeof(float));
	
	gpio_Config();
	USART2_config();
	USART3_config();
	UART4_config();
	timer_Config();
	DAC_Config();

	//Set the priority for the interrupts
	//NVIC_SetPriority(USART2_IRQn,0x07);
	NVIC_SetPriority(TIM4_IRQn,0x07);
	
	//Mutex for writing to serial terminal
	uart_mutex = xSemaphoreCreateMutex();
	
	//Task for LED
	if(xTaskCreate(LED_task, "LED", 32, NULL, 2, NULL) != pdPASS){
		while(1);
	}
	//Task for Button
  if(xTaskCreate(Button_task, "Button", 32, NULL, 2, NULL) != pdPASS){
		while(1);
	}
	
	if(xTaskCreate(note_task, "note_task", 256, NULL, 2, &note_handle) != pdPASS){
		while(1);
	}
	
	if(xTaskCreate(volume_task, "volume_task", 256, NULL, 2, &vol_handle) != pdPASS){
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
	//Valid Volumes: 0 0.1428 0.2856 0.4284 0.5712 0.714 0.8568 1
	static float volume[1] = {1.0};
	
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
	//if there is a message waiting in the queue from ISR
	if(uxQueueMessagesWaitingFromISR(vol_queue) > 0){
		xQueueReceiveFromISR(vol_queue,volume,NULL);
	}
	
	//Assign DAC to Sine_Wave Table Current Value
	DAC->DHR12R1 = (uint16_t)(sineLookupTable[sine_count]* volume[0]) + 45;
	TIM4->SR &= ~TIM_SR_UIF; //Clears Interrupt Flag
}

/*void USART2_IRQHandler(void){
	uint8_t uart_buffer[1];
	uart_buffer[0] = (uint8_t)(USART2->RDR);	//Get serial data
	change_note(uart_buffer[0]);
	//If uart_buffer = t or p send into the queue
	if (uart_buffer[0] == 't' || uart_buffer[0] == 'p'){
		xQueueSendToBackFromISR(uart2_to_uart3_queue,uart_buffer,NULL);
	}
}*/

void change_note(uint8_t note){
	if (note == 'a'){
		TIM4->ARR = 0xFFFF00EF; //239; 130.813 Hz C3
	}
	else if (note == 'b'){
		TIM4->ARR = 0xFFFF00E1; //225; 138.591 Hz C#3
	}
	else if (note == 'c'){
		TIM4->ARR = 0xFFFF00D5; //213; 146.832 Hz D3
	}
	else if (note == 'd'){
		TIM4->ARR = 0xFFFF00C9; //201; 155.563 Hz D#3
	}
	else if (note == 'e'){
		TIM4->ARR = 0xFFFF00BE;	//190; 164.814 Hz E3
	}
	else if (note == 'f'){
		TIM4->ARR = 0xFFFF00B3; //179; 174.614 Hz F3
	}
	else if (note == 'g'){
		TIM4->ARR = 0xFFFF00A9; //169; 184.997 Hz F#3
	}
	else if (note == 'h'){
		TIM4->ARR = 0xFFFF009F; //159; 195.998 Hz G3
	}	
	else if (note == 'i'){
		TIM4->ARR = 0xFFFF0096; //150; 207.652 Hz G#3
	}
	else if (note == 'j'){
		TIM4->ARR = 0xFFFF008E; //2 MHz/(142) = 14.080 kHz interrupt rate; 220 Hz sine wave A3
	}
	else if (note == 'k'){
		TIM4->ARR = 0xFFFF0086; //134; 233.082 Hz sine wave A#3
	}
	else if (note == 'l'){
		TIM4->ARR = 0xFFFF007E; //126; 246.94 Hz B3
	}
	else if (note == 'm'){
		TIM4->ARR = 0xFFFF0077; //119; 261.626 Hz C4
	}
	else if (note == 'n'){
		TIM4->ARR = 0xFFFF0071; //113; 277.183 Hz C#4
	}
	else if (note == 'o'){
		TIM4->ARR = 0xFFFF006A; //106; 293.66 Hz D4
	}
	else if (note == 'p'){
		TIM4->ARR = 0xFFFF0064; //100; 311.127 Hz D#4
	}
	else if (note == 'q'){
		TIM4->ARR = 0xFFFF005E;	//94; 329.63 Hz E4
	}
	else if (note == 'r'){
		TIM4->ARR = 0xFFFF0059; //89; 349.23 Hz F4
	}
	else if (note == 's'){
		TIM4->ARR = 0xFFFF0054; //84; 369.994 Hz F#4
	}
	else if (note == 't'){
		TIM4->ARR = 0xFFFF004F; //80; 392.00 Hz G4
	}
	else if (note == 'u'){
		TIM4->ARR = 0xFFFF004B; //75; 415.305 Hz G#4
	}
	else if (note == 'v'){
		TIM4->ARR = 0xFFFF0046; //71; 440 Hz A4
	}	
	else if (note == 'w'){
		TIM4->ARR = 0xFFFF0043; //67; 466.164 Hz A#4
	}	
	else if (note == 'x'){
		TIM4->ARR = 0xFFFF003F; //63; 493.883 Hz B4
	}	
	else if (note == 'y'){
		TIM4->ARR = 0xFFFF003C; //60; 523.251 Hz C5
	}	
}

void note_task(void *pvParameters){
	uint16_t measurement;
	float inches = 0;
	uint32_t change = 0;
	
	//Track what is the previous and current note being played
	uint8_t note = 'a';
	uint8_t prev_note = 'a';
	float volume[1] = {1};
	//float prev_volume[1] = {1};
	
	while(1){
		vTaskPrioritySet(note_handle,3);
		USART3_write(0x55);
		while (!(USART3->ISR & USART_ISR_RXNE)); //Wait until hardware sets RXNE
		measurement = USART3->RDR;
		measurement *= 256;
		while (!(USART3->ISR & USART_ISR_RXNE)); //Wait until hardware sets RXNE
		measurement += USART3->RDR;
		inches = (float)(measurement / 25.4);

		//Based on the distance from the sensor change the note being played
		if (inches <= 2){										 //C3
			note = 'a';
		}
		else if (inches > 2 && inches <= 3){ //C#3
			note = 'b';
		}
		else if (inches > 3 && inches <= 4){ //D3
			note = 'c';
		}
		else if (inches > 4 && inches <= 5){ //D#3
			note = 'd';
		}
		else if (inches > 5 && inches <= 6){ //E3
			note = 'e';
		}
		else if (inches > 6 && inches <= 7){ //F3
			note = 'f';
		}
		else if (inches > 7 && inches <= 8){ //F#3
			note = 'g';
		}
		else if (inches > 8 && inches <= 9){ //G3
			note = 'h';
		}
		else if (inches > 9 && inches <= 10){ //G#3
			note = 'i';
		}
		else if (inches > 10 && inches <= 11){ //A3
			note = 'j';
		}
		else if (inches > 11 && inches <= 12){ //A#3
			note = 'k';
		}
		else if (inches > 12 && inches <= 13){ //B3
			note = 'l';
		}
		else if (inches > 13 && inches <= 16){ //C4 --Mary 13 - 16
			note = 'm';
		}
		else if (inches > 16 && inches <= 17){ //C#4
			note = 'n';
		}
		else if (inches > 17 && inches <= 20){ //D4 --Mary 17 - 20 
			note = 'o';
		}
		else if (inches > 20 && inches <= 21){ //D#4
			note = 'p';
		}
		else if (inches > 21 && inches <= 24){ //E4 --Mary 21 - 24
			note = 'q';
		}
		else if (inches > 24 && inches <= 25){ //F4
			note = 'r';
		}
		else if (inches > 25 && inches <= 26){ //F#4
			note = 's';
		}
		else if (inches > 26 && inches <= 29){ //G4 --Mary 26 - 29
			note = 't';
		}
		else if (inches > 29 && inches <= 30){ //G#4
			note = 'u';
		}
		else if (inches > 30 && inches <= 31){ //A4
			note = 'v';
		}
		else if (inches > 31 && inches <= 32){ //A#4
			note = 'w';
		}
		else if (inches > 32 && inches <= 33){ //B4
			note = 'x';
		}
		else if (inches > 34){								 //C5
			note = 'y';
		}
		
		//Change the note frequency based on distance from sensor
		change_note(note);
		//Only update output on serial terminal if the note changed
		if (note != prev_note ){
			change = 0;
			xSemaphoreTake(uart_mutex,(TickType_t)50);
			USART2_write(note,volume,change);
			xSemaphoreGive(uart_mutex);
		}
		prev_note = note; 
		vTaskPrioritySet(note_handle,2);
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}

void volume_task(void *pvParameters){
	uint16_t measurement;
	float inches = 0;
	uint32_t change = 0;

	uint8_t note = 'a';
	//uint8_t prev_note = 'a';
	float volume[1] = {1};
	float prev_volume[1] = {1};
	
	while(1){
		vTaskPrioritySet(vol_handle,3);
		//Pull distance value from 2nd proximity sensor for volume
		UART4_write(0x55);
		while (!(UART4->ISR & USART_ISR_RXNE)); //Wait until hardware sets RXNE
		measurement = UART4->RDR;
		measurement *= 256;
		while (!(UART4->ISR & USART_ISR_RXNE)); //Wait until hardware sets RXNE
		measurement += UART4->RDR;
		inches = (float)(measurement / 25.4);
		
		//Determine what the volume scaler should be based on 2nd proximity sensor
		if (inches <= 6){	//volume off
			volume[0] = 0;
		}
		else if (inches > 6 && inches <= 7){
			volume[0] = 0.1428;
		}
		else if (inches > 7 && inches <= 8){
			volume[0] = 0.2856;
		}
		else if (inches > 8 && inches <= 9){
			volume[0] = 0.4284;
		}
		else if (inches > 9 && inches <= 10){
			volume[0] = 0.5712;
		}
		else if (inches > 10 && inches <= 11){
			volume[0] = 0.714;
		}
		else if (inches > 11 && inches <= 12){
			volume[0] = 0.8568;
		}
		else if (inches > 12){ //Volume on fully
			volume[0] = 1;
		}
		
		//Send volume multiplier to Timer Interrupt handler
		xQueueSendToBack(vol_queue,volume,50);
		
		//Only update output on serial terminal if the volume changed
		if (volume[0] != prev_volume[0]){
			change = 1;
			xSemaphoreTake(uart_mutex,(TickType_t)50);
			USART2_write(note,volume,change);
			xSemaphoreGive(uart_mutex);
		}
		prev_volume[0] = volume[0];
		vTaskPrioritySet(vol_handle,2);
		vTaskDelay(100 / portTICK_PERIOD_MS);
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

void UART4_write(uint8_t measure_type){
	//Send command to measure the temperature or proximity distance
	while(!(UART4->ISR & USART_ISR_TXE)); //Wait until hardware sets TXE
	UART4->TDR = measure_type & 0xFF; //Writing to TDR clears TXE Flag

	//Wait until TC bit is set. TC is set by hardware and cleared by software
	while (!(UART4->ISR & USART_ISR_TC)); //TC: Transmission complete flag

	//Writing 1 to the TCCF bit in ICR clears the TC bit in ICR
	UART4->ICR |= USART_ICR_TCCF; //TCCF: Transmission complete clear flag 
}

void USART2_write(uint8_t note,float volume[1],uint32_t change){
	int nBytes = 30;
	char serial_message[30] = {0};
	
	//Send update to serial terminal if the note changed 
	if (change == 0){
		switch(note){
			case 'a':
				sprintf(serial_message, "C3\n\r");
				break;
			case 'b':
				sprintf(serial_message, "C#3\n\r");
			break;
			case 'c':
				sprintf(serial_message, "D3\n\r");
				break;
			case 'd':
				sprintf(serial_message, "D#3\n\r");
			break;
			case 'e':
				sprintf(serial_message, "E3\n\r");
				break;
			case 'f':
				sprintf(serial_message, "F3\n\r");
			break;
			case 'g':
				sprintf(serial_message, "F#3\n\r");
				break;
			case 'h':
				sprintf(serial_message, "G3\n\r");
			break;
			case 'i':
				sprintf(serial_message, "G#3\n\r");
				break;
			case 'j':
				sprintf(serial_message, "A3\n\r");
			break;
			case 'k':
				sprintf(serial_message, "A#3\n\r");
				break;
			case 'l':
				sprintf(serial_message, "B3\n\r");
			break;
			case 'm':
				sprintf(serial_message, "C4\n\r");
				break;
			case 'n':
				sprintf(serial_message, "C#4\n\r");
			break;
			case 'o':
				sprintf(serial_message, "D4\n\r");
				break;
			case 'p':
				sprintf(serial_message, "D#4\n\r");
			break;
			case 'q':
				sprintf(serial_message, "E4\n\r");
				break;
			case 'r':
				sprintf(serial_message, "F4\n\r");
			break;
			case 's':
				sprintf(serial_message, "F#4\n\r");
				break;
			case 't':
				sprintf(serial_message, "G4\n\r");
			break;
			case 'u':
				sprintf(serial_message, "G#4\n\r");
				break;
			case 'v':
				sprintf(serial_message, "A4\n\r");
			break;
			case 'w':
				sprintf(serial_message, "A#4\n\r");
				break;
			case 'x':
				sprintf(serial_message, "B4\n\r");
			break;
			case 'y':
				sprintf(serial_message, "C5\n\r");
				break;
		}
	}
	//Send update to serial terminal if the volume changed 
	else if (change == 1){
		//Print out current volume 
		if((double)volume[0] < 0.1){
				sprintf(serial_message, "Vol 0\n\r");
		}
		else if((double)volume[0] > 0.1 && (double)volume[0] < .25){
				sprintf(serial_message, "Vol 1\n\r");
		}
		else if((double)volume[0] > 0.25 && (double)volume[0] < .4){
				sprintf(serial_message, "Vol 2\n\r");
		}
		else if((double)volume[0] > 0.4 && (double)volume[0] < .55){
				sprintf(serial_message, "Vol 3\n\r");
		}
		else if((double)volume[0] > 0.55 && (double)volume[0] < .7){
				sprintf(serial_message, "Vol 4\n\r");
		}
		else if((double)volume[0] > 0.7 && (double)volume[0] < .85){
				sprintf(serial_message, "Vol 5\n\r");
		}
		else if((double)volume[0] > 0.85 && (double)volume[0] < 1.0){
				sprintf(serial_message, "Vol 6\n\r");
		}
		else if((double)volume[0] >= 1.0){
				sprintf(serial_message, "Vol 7\n\r");
		}
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
