#include "driveusart.h"




void USART_Gpio(){

	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable GPIOA clock */
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE );

	/* USART Periph clock enable */
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART2 , ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);




}



void USART_Setup(uint32_t baudrate, uint16_t buffer_Tx){

	USART_Gpio();

	USART_Interrupt(USART2_IRQn, NVIC_PriorityGroup_1, 0, 2);

	xQueueUSARTransmit = xQueueCreate((uint16_t) buffer_Tx,sizeof(char)); //Cria fila de mensagens


	USART_InitTypeDef USART_InitStructure;


	USART_InitStructure.USART_BaudRate = (uint32_t) baudrate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;


    /* Configure the USART2 */
    USART_Init(USART2, &USART_InitStructure);
    /* Enable the USART2 */
    USART_Cmd(USART2, ENABLE);

	/*Ativação da interrupção da receção de dados*/
	USART_ITConfig(USART2, USART_IT_TXE, ENABLE);

}



void USART_Interrupt(uint8_t channel, uint32_t nvic_group, uint8_t priority, uint8_t subpriority){

	NVIC_InitTypeDef NVIC_InitStructure;

	/* Configura o Priority Group com 1 bit */
	NVIC_PriorityGroupConfig((uint32_t) nvic_group);


	/* Interrupção global do TIM3 com prioridade 0 sub-prioridade 2 */
	NVIC_InitStructure.NVIC_IRQChannel = (uint8_t) channel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = (uint8_t) priority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = (uint8_t) subpriority;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);



}



void USART_Close(){

	USART_Cmd(USART2, DISABLE);
	vQueueDelete(xQueueUSARTransmit);

}




void USART_Flush(void){

	xQueueReset(xQueueUSARTransmit);

}




void USART_PutChar(char put_char){

	if( xQueueSendToBack( xQueueUSARTransmit, ( void * ) &put_char, ( TickType_t ) 10 ) != pdPASS );

	USART_ITConfig( USART2, USART_IT_TXE, ENABLE);


}



void USART_PutString(char *put_string){

	uint8_t string_length;

	string_length = strlen(put_string);

	uint8_t i;

	for(i=0; i<string_length; i++){

		xQueueSendToBack( xQueueUSARTransmit, ( void * ) &put_string[i], ( TickType_t ) portMAX_DELAY );

	}

	USART_ITConfig( USART2, USART_IT_TXE, ENABLE);

}
