#ifndef LCDMANAGER_H_INCLUDED
#define LCDMANAGER_H_INCLUDED

#include "FreeRTOS.h"

#define LCD_MANAGER_TASK_PRIORITY                  ( tskIDLE_PRIORITY + 1 )
#define LCD_MANAGER_DELAY						   ( ( portTickType ) 10 / portTICK_RATE_MS )

// lcd buf manager

unsigned short getStrNum(unsigned char rowNum);
void setStrNum(unsigned char rowNum, unsigned short value);
void LcdManagerTask( void *pvParameters );

#endif /* LCDMANAGER_H_INCLUDED */
