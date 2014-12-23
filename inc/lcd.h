#ifndef LCD_H_INCLUDED
#define LCD_H_INCLUDED

#include "FreeRTOS.h"

#define LCD_TASK_PRIORITY                  ( tskIDLE_PRIORITY + 1 )
#define LCD_DELAY						   ( ( portTickType ) 10 / portTICK_RATE_MS )

// image lcd buf

void LcdTask( void *pvParameters );
void setSymbol(unsigned char num, unsigned char value);
unsigned char* getLcdBuf(void);
#endif /* LCD_H_INCLUDED */
