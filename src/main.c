/*
**
**                           Main.c
**
**
**********************************************************************/
/*
   Last committed:     $Revision: 00 $
   Last changed by:    $Author: $
   Last changed date:  $Date:  $
   ID:                 $Id:  $

**********************************************************************/
#include "stm32f10x_conf.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define Main_TASK_PRIORITY                  ( tskIDLE_PRIORITY + 1)
#define Main_DELAY						    ( ( portTickType ) 500 / portTICK_RATE_MS )

#include "lcd.h"
#include "uart.h"
#include "uartManager.h"
#include "lcdManager.h"
#include "keyScan.h"
#include "settings.h"

static void MainTask( void *pvParameters );

int main(void)
{
    xTaskCreate( MainTask, ( signed portCHAR * ) "Main", configMINIMAL_STACK_SIZE, NULL, Main_TASK_PRIORITY, NULL );
    xTaskCreate( LcdTask, (signed portCHAR *) "Lcd", configMINIMAL_STACK_SIZE, NULL, LCD_TASK_PRIORITY, NULL);
    xTaskCreate( UartTask, (signed portCHAR *) "Uart", configMINIMAL_STACK_SIZE, NULL, UART_TASK_PRIORITY, NULL);
    xTaskCreate( UartManagerTask, (signed portCHAR *) "UartManager", configMINIMAL_STACK_SIZE, NULL, UART_MANAGER_TASK_PRIORITY, NULL);
    xTaskCreate( LcdManagerTask, (signed portCHAR *) "LcdManager", configMINIMAL_STACK_SIZE, NULL, LCD_MANAGER_TASK_PRIORITY, NULL);
    xTaskCreate( KeyTask, (signed portCHAR *) "Key", configMINIMAL_STACK_SIZE, NULL, KEY_TASK_PRIORITY, NULL);
    vTaskStartScheduler();
    return 0;
}

void MainTask( void *pvParameters )
{
    portTickType xLastExecutionTime;
    // LED init
    GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE );
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init( GPIOB, &GPIO_InitStructure );

    xLastExecutionTime = xTaskGetTickCount();
    readSettings();
    for( ;; )
	{
        // toggle LED
        //GPIOB->ODR ^= GPIO_Pin_12;
        vTaskDelayUntil( &xLastExecutionTime, Main_DELAY );
    }
}
