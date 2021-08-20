/**
  ******************************************************************************
  * Autores:
  * Ruben Pinto nº1170839
  * Rui   Nunes nº1170375
  ******************************************************************************

Standard includes*/
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <stm32f10x_spi.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "lcd.h"
#include "stm32f10x.h"
#include "stm32f10x_it.h"
#include "queue.h"

/*Drive includes*/
#include "drivejoystick.h"
#include "driveusart.h"
#include "drivespiadxl.h"


//Configure RCC clock at 72 MHz
static void prvSetupRCC( void );

//Configure GPIO.
static void prvSetupGPIO( void );


/***************************************/
//Simple LED toggle task.
static void prvLEDPISCA( void *pvParameters );

static void prvADXL345( void *pvParameters );

static void prvMENU(void *pvParameters);

void obstaculo(void);

/************************************/
/* Task 1 handle variable. */
TaskHandle_t HandleTask1;
TaskHandle_t HandleTask2;
TaskHandle_t HandleTask3;


QueueHandle_t xQueue;

float leitura_X1,leitura_Y1,leitura_Z1;  //Variaveis utilizadas para a conversao dos valores do acelerometro
volatile int jogo=0; //Ativa e desativa o jogo
volatile int col; //Deteta colisao



void init(){
    prvSetupRCC();
    prvSetupGPIO();
    SPI_gpio();
    lcd_init();
    SPI_config();
    ADXL345();
    USART_Setup(115200, 50);
    joystick_setup();
}

int main( void )
{
	/*Setup the hardware, RCC, GPIO, etc...*/
    init();



	/* Create the tasks */
 	xTaskCreate( prvLEDPISCA, "LED", configMINIMAL_STACK_SIZE, NULL, mainLEDPISCA_TASK_PRIORITY, &HandleTask1 );
 	xTaskCreate( prvADXL345, "ADXL345", configMINIMAL_STACK_SIZE*2, NULL, mainADXL345_TASK_PRIORITY, &HandleTask2 );
 	xTaskCreate( prvMENU, "MENU", configMINIMAL_STACK_SIZE*2, NULL, mainMENU_TASK_PRIORITY, &HandleTask3 );



	/* Start the scheduler. */
	vTaskStartScheduler();

	/* Will only get here if there was not enough heap space to create the idle task. */
	return 0;
}
/*-----------------------------------------------------------*/
static void prvLEDPISCA( void *pvParameters )
{

	TickType_t xLastExecutionTime;
	xLastExecutionTime = xTaskGetTickCount();
    for( ;; )
	{
		/* Block 1 second. */
    	vTaskDelayUntil( &xLastExecutionTime, ( TickType_t ) mainLED_DELAY );

        /* Toggle the LED */
		GPIO_WriteBit(GPIOB, GPIO_Pin_6, ( 1-GPIO_ReadOutputDataBit( GPIOB, GPIO_Pin_6 ) ) );
	}
}
/*-----------------------------------------------------------*/
static void prvADXL345(void *pvParameters){
	TickType_t xLastExecutionTime;
	xLastExecutionTime = xTaskGetTickCount();
	int16_t leitura_X,leitura_Y,leitura_Z;	//variaveis utilizadas para a conversao dos valores do acelerometro
	char buffer[30];
    for(;;){
    	        //Juntar Parte High a Low
    	        leitura_X = ((SPI_read(0x33) << 8) + SPI_read(0x32));
    		    leitura_Y = ((SPI_read(0x35) << 8) + SPI_read(0x34));
    		    leitura_Z = ((SPI_read(0x37) << 8) + SPI_read(0x36));


    			//±2 g, 10-bit resolution 3.9 mg/LSB
    			leitura_X1=(float) (leitura_X * 0.0039);
    			leitura_Y1=(float) (leitura_Y * 0.0039);
    			leitura_Z1=(float) (leitura_Z * 0.0039);



    			//Conversão dos valores calculados para uma string
    			sprintf(buffer, "X:%.2f Y:%.2f Z:%.2f\r\n", leitura_X1, leitura_Y1, leitura_Z1);

    			USART_PutString(buffer);

    			vTaskDelayUntil( &xLastExecutionTime, ( TickType_t ) mainADXL345_DELAY );
    }
}
/*-----------------------------------------------------------*/
static void prvMENU(void *pvParameters){
	TickType_t xLastExecutionTime;
	xLastExecutionTime = xTaskGetTickCount();

	lcd_draw_fillrect(0, 0, 130, 70, BLACK);
	lcd_draw_rect(0, 0, 120, 63, WHITE);
	lcd_draw_string(5, 5, "MENU", WHITE, 1);
	lcd_draw_string(5, 20, " +  - Menu", WHITE, 1);
	lcd_draw_string(5, 30, "<-- - Print Tasks", WHITE, 1);
	lcd_draw_string(5, 40, "--> - Start Game", WHITE, 1);
	lcd_draw_string(5, 50, " ^  - PrintXYZ|Stck", WHITE, 1);

	volatile int a;
	char buffer_lcd[50];
    for(;;){

		switch(joystick_read()){

			case 'L':
				a=1;
				jogo=0;
				//Print Tasks
				lcd_draw_fillrect(0, 0, 127, 70, BLACK);//Limpar display
				sprintf(buffer_lcd, "Tick Count: %ld",xLastExecutionTime);
				lcd_draw_string(5, 0,buffer_lcd,0x07F0, 1);
				sprintf(buffer_lcd, "Name | Priority");
				lcd_draw_string(5, 20,buffer_lcd,0x07F0, 1);
				sprintf(buffer_lcd, "%s | %ld",pcTaskGetName(HandleTask1),uxTaskPriorityGet(HandleTask1));
				lcd_draw_string(5, 30,buffer_lcd,0x07F0, 1);
				sprintf(buffer_lcd, "%s | %ld",pcTaskGetName(HandleTask2),uxTaskPriorityGet(HandleTask2));
				lcd_draw_string(5, 40,buffer_lcd,0x07F0, 1);
				sprintf(buffer_lcd, "%s | %ld",pcTaskGetName(HandleTask3),uxTaskPriorityGet(HandleTask3));
				lcd_draw_string(5, 50,buffer_lcd,0x07F0, 1);


				break;


			case 'R':

				//Start Game
				a=0;
				jogo=1;
				lcd_draw_fillrect(0, 0, 127, 70, BLACK);//Limpar display
				lcd_draw_line(0, 0, 120, 0, WHITE);
				lcd_draw_line(0,27, 120, 27, WHITE);
				lcd_draw_line(0,55, 120, 55, WHITE);
				display();
				obstaculo();

				break;

			case'U':

				//Print XYZ
				a=2;
				jogo=0;
				UBaseType_t uxHighWaterMark1;
				uxHighWaterMark1 = uxTaskGetStackHighWaterMark(HandleTask1);
				UBaseType_t uxHighWaterMark2;
				uxHighWaterMark2 = uxTaskGetStackHighWaterMark(HandleTask2);
				UBaseType_t uxHighWaterMark3;
				uxHighWaterMark3 = uxTaskGetStackHighWaterMark(HandleTask3);

				lcd_draw_fillrect(0, 0, 130, 70, BLACK);
				sprintf(buffer_lcd, "X:%.1f ",leitura_X1);
				lcd_draw_string(0, 5, buffer_lcd, WHITE, 1);
				sprintf(buffer_lcd, " Y:%.1f",leitura_Y1);
				lcd_draw_string(35, 5, buffer_lcd, WHITE, 1);
				sprintf(buffer_lcd, " Z:%.1f",leitura_Z1);
				lcd_draw_string(65, 5, buffer_lcd, WHITE, 1);

				sprintf(buffer_lcd, "TickCount = %ld", xTaskGetTickCount());
				lcd_draw_string(5, 20, buffer_lcd, WHITE, 1);

				sprintf(buffer_lcd, "%s: %ld", pcTaskGetName(HandleTask1),uxHighWaterMark1);
				lcd_draw_string(5, 30, buffer_lcd, WHITE, 1);
				sprintf(buffer_lcd, "%s: %ld", pcTaskGetName(HandleTask2),uxHighWaterMark2);
				lcd_draw_string(5, 40, buffer_lcd, WHITE, 1);
				sprintf(buffer_lcd, "%s: %ld", pcTaskGetName(HandleTask3),uxHighWaterMark3);
				lcd_draw_string(5, 50, buffer_lcd, WHITE, 1);

				break;

			case'E':

				//Print MENU
				a=0;
				jogo=0;
				lcd_draw_fillrect(0, 0, 130, 70, BLACK);
				lcd_draw_rect(0, 0, 120, 63, WHITE);
				lcd_draw_string(5, 5, "MENU", WHITE, 1);
				lcd_draw_string(5, 20, " +  - Menu", WHITE, 1);
				lcd_draw_string(5, 30, "<-- - Print Tasks", WHITE, 1);
				lcd_draw_string(5, 40, "--> - Start Game", WHITE, 1);
				lcd_draw_string(5, 50, " ^  - PrintXYZ|Stck", WHITE, 1);

				break;

			default:
				if (a==1){
					sprintf(buffer_lcd, "Tick Count: %ld",xLastExecutionTime);
					lcd_draw_string(5, 0,buffer_lcd,0x07F0,1);
				}
				if(a==2){
					sprintf(buffer_lcd, " X:%.1f",leitura_X1);
					lcd_draw_string(0, 5, buffer_lcd, WHITE, 1);
					sprintf(buffer_lcd, " Y:%.1f",leitura_Y1);
					lcd_draw_string(35, 5, buffer_lcd, WHITE, 1);
					sprintf(buffer_lcd, " Z:%.1f",leitura_Z1);
					lcd_draw_string(65, 5, buffer_lcd, WHITE, 1);

					sprintf(buffer_lcd, "TickCount = %ld", xTaskGetTickCount());
					lcd_draw_string(5, 20, buffer_lcd, WHITE, 1);
				}
				break;

			}
		display();

		vTaskDelayUntil( &xLastExecutionTime, ( TickType_t ) mainMENU_DELAY );

	}
}
/*-----------------------------------------------------------*/

/*---------------------GAME----------------------------------*/
void peao(void){

		if (leitura_Y1<=0){
						lcd_draw_fillrect(0, 35, 15, 15, BLACK);
						lcd_draw_fillrect(0, 8, 15, 15, WHITE);
						display();
						col=1;
					}else{
						lcd_draw_fillrect(0, 8, 15, 15, BLACK);
						lcd_draw_fillrect(0, 35, 15, 15, WHITE);
						display();
						col=2;
					}
}
/*-----------------------------------------------------------*/
void colisao(){
	jogo=0;
	lcd_draw_fillrect(0, 0, 130, 70, BLACK);
	lcd_draw_string(5, 30, "GAME OVER", WHITE, 1);
	lcd_draw_string(5, 40, "( + - Back Menu)", WHITE, 1);
	display();

}
/*-----------------------------------------------------------*/
void obstaculo(void){
	volatile int r1,r2,r3;
	while(jogo==1){
	r1 = rand() % 55;//Gera numero random para posiconar fila do obstaculo
					if(r1 >= 27){
						for(r3=0; r3 < 13; r3++){
							peao();
							r2 = r2-10;
							lcd_draw_circle(r2, 15, 5, WHITE);
							display();
							lcd_draw_fillrect(r2, 10, 25, 15, BLACK);
							display();
							if(r2 == 10 && col==1){
								colisao();
							}
						}
					r2=130;
					}else{
						for(r3=0; r3 < 13; r3++){
							peao();
							r2 = r2-10;
							lcd_draw_circle(r2, 38, 5, WHITE);
							display();
							lcd_draw_fillrect(r2, 33, 20, 15, BLACK);
							display();
							if(r2 == 10 && col==2){//Testa Ccolisãoad
								colisao();
							}
					}
					r2=130;
					}
	}
}
/*-------------------END GAME--------------------------------*/



/*-----------------------------------------------------------*/

static void prvSetupRCC( void )
{
    /* RCC configuration - 72 MHz */
    ErrorStatus HSEStartUpStatus;

    RCC_DeInit();
    /*Enable the HSE*/
    RCC_HSEConfig(RCC_HSE_ON);
    /* Wait untill HSE is ready or time out */
    HSEStartUpStatus = RCC_WaitForHSEStartUp();
    if(HSEStartUpStatus == SUCCESS)
    {
        /* Enable The Prefetch Buffer */
        FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
        /* 72 MHZ - 2 wait states */
        FLASH_SetLatency(FLASH_Latency_2);

        /* No division HCLK = SYSCLK = 72 MHz*/
        RCC_HCLKConfig(RCC_SYSCLK_Div1);
        /* PCLK1 = HCLK/2 (36MHz) */
        RCC_PCLK1Config(RCC_HCLK_Div2);
        /* PCLK2 = HCLK (72MHz)*/
        RCC_PCLK2Config(RCC_HCLK_Div1);

        /* Use PLL with HSE = 12 MHz (12 MHz * 6 = 72 MHz) */
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_6);
        /* Enable the PLL */
        RCC_PLLCmd(ENABLE);
        /* Wait for PLL ready */
        while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET );

        /* Select the PLL as system clock source */
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
        /* Wait until PLL is used as system clock */
        while( RCC_GetSYSCLKSource() != 0x08 );
    }
    else
    {
    	/* HSE error? No further action */
        while(1);
    }
}
/*-----------------------------------------------------------*/

static void prvSetupGPIO( void )
{
	//Enable CLock
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		GPIO_InitTypeDef GPIO_InitStructure;

	//_______________LED_________________________________________________

		//GPIOB6 - LED STATE
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure);


}



