#ifndef KEYSCAN_H_INCLUDED
#define KEYSCAN_H_INCLUDED

#include "FreeRTOS.h"

#define Key_F1          0
#define Key_Left        1
#define Key_Up          2
#define Key_Right       3
#define Key_F2          4
#define Key_Shift       5
#define Key_Down        6
#define Key_Enter       7


#define KEY_TASK_PRIORITY                  ( tskIDLE_PRIORITY + 1 )
#define KEY_DELAY						   ( ( portTickType ) 10 / portTICK_RATE_MS )

void KeyTask( void *pvParameters );

unsigned short getFilteredKeys(void);

#endif /* KEYSCAN_H_INCLUDED */
