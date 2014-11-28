#ifndef UART_H_INCLUDED
#define UART_H_INCLUDED

#include "FreeRTOS.h"

#define UART_TASK_PRIORITY                  ( tskIDLE_PRIORITY + 1 )
#define UART_DELAY						   ( ( portTickType ) 1 / portTICK_RATE_MS )

// serial channel write read operations (RS485)

void UartTask( void *pvParameters );
char isRxDataReady(void);
unsigned short getRxCnt(void);
void setRxPause(unsigned char value);
unsigned char getRxByte(unsigned short i);
void setTxByte(unsigned short i, unsigned char value);
unsigned short writeBuf(unsigned short cnt);
unsigned char* getTxPtr(void);
unsigned char* getRxPtr(void);
unsigned short getUartBufSize(void);
void clearRxBuf(void);

#endif /* UART_H_INCLUDED */
