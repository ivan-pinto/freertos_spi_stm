#ifndef DRIVEJOYSTICK_H_
#define DRIVEJOYSTICK_H_

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "queue.h"

/*
 * Fila de mensagens joystick
 */

QueueHandle_t xQueueEXT;

void joystick_setup(void);
char joystick_read();
void joystick_flush(void);



#endif
