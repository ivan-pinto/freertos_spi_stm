#ifndef DRIVEUSART_H_
#define DRIVEUSART_H_

#include <string.h>
#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "queue.h"



//Fila Tx
QueueHandle_t xQueueUSARTransmit, xQueueUSARTReceive;


void USART_Gpio();
void USART_Setup(uint32_t baudrate, uint16_t buffer_depth_Tx);
void USART_Interrupt(uint8_t channel, uint32_t nvic_group, uint8_t priority, uint8_t subpriority);
void USART_Close();
void USART_Flush(void);
void USART_PutChar(char put_char);
void USART_PutString(char *put_string);


#endif
