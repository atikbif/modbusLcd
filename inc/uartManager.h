#ifndef UARTMANAGER_H_INCLUDED
#define UARTMANAGER_H_INCLUDED

#include "FreeRTOS.h"

#define UART_MANAGER_TASK_PRIORITY                  ( tskIDLE_PRIORITY + 1 )
#define UART_MANAGER_DELAY						   ( ( portTickType ) 10 / portTICK_RATE_MS )

// main uart task (manage write and read)

void UartManagerTask( void *pvParameters );
void setMode(unsigned char modeValue);
void setWaitAnswerPauseMs(unsigned char value);
void setAttemptOfRequestCount(unsigned char value);

#endif /* UARTMANAGER_H_INCLUDED */
