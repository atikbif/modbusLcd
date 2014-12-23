#ifndef UARTMANAGER_H_INCLUDED
#define UARTMANAGER_H_INCLUDED

#include "FreeRTOS.h"

#define UART_MANAGER_TASK_PRIORITY                  ( tskIDLE_PRIORITY + 1 )
#define UART_MANAGER_DELAY						   ( ( portTickType ) 10 / portTICK_RATE_MS )

#define UART_WORK_MODE      0
#define UART_PROG_MODE      1
#define UART_UNDEF_MODE          2

// main uart task (manage write and read)

void UartManagerTask( void *pvParameters );
void setMode(unsigned char modeValue);
unsigned char getUartMode(void);
void setWaitAnswerPauseMs(unsigned char value);
void setAttemptOfRequestCount(unsigned char value);

#endif /* UARTMANAGER_H_INCLUDED */
