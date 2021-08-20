#ifndef DRIVESPIADXL_H_
#define DRIVESPIADXL_H_

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "queue.h"


void SPI_gpio(void);
void SPI_config(void);
void ADXL345(void);
void SPI_write(uint8_t address, uint8_t data);
uint8_t SPI_read(uint8_t address);



#endif
