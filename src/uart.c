#include "uart.h"
#include "stm32f10x_conf.h"
#include "task.h"

#define UART_BUF_SIZE   256
#define USART1_DR_Base			0x40013804

const char helloString[] = "Modbus Display ver 1.0";

volatile unsigned char tx_buf[UART_BUF_SIZE];
volatile unsigned char rx_buf[UART_BUF_SIZE];
volatile unsigned short rx_cnt=0;
unsigned char rx_pause=3;
volatile unsigned short rx_tmr=0;



static void init_uart(void);
static void write_uart(unsigned short cnt);
static void write_start_message(void);

void USART1_IRQHandler(void);

void init_uart(void)
{
    USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 19200;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(USART1, &USART_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_ITConfig(USART1, USART_IT_RXNE , ENABLE);
	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
	USART_Cmd(USART1, ENABLE);

	GPIO_WriteBit(GPIOB, GPIO_Pin_5, Bit_RESET);
}

void UartTask( void *pvParameters )
{
    portTickType xLastExecutionTime;
    init_uart();
    write_start_message();
    xLastExecutionTime = xTaskGetTickCount();
    for( ;; )
	{
	    taskENTER_CRITICAL();
	    if(rx_cnt)
        {
            if(rx_tmr<rx_pause) rx_tmr++;
        }
        taskEXIT_CRITICAL();
        vTaskDelayUntil( &xLastExecutionTime, UART_DELAY );
    }
}

unsigned short getUartBufSize(void)
{
    return UART_BUF_SIZE;
}

unsigned char* getTxPtr(void)
{
    return (unsigned char*)tx_buf;
}

unsigned char* getRxPtr(void)
{
    return (unsigned char*)rx_buf;
}

char isRxDataReady(void)
{
    char tmp=0;
    taskENTER_CRITICAL();
    if((rx_tmr>=rx_pause)&&(rx_pause)) tmp=1;
    taskEXIT_CRITICAL();
    return tmp;
}

unsigned short getRxCnt(void)
{
    unsigned short tmp;
    taskENTER_CRITICAL();
    tmp = rx_cnt;
    taskEXIT_CRITICAL();
    return tmp;
}

void setRxPause(unsigned char value)
{
    taskENTER_CRITICAL();
    if(value != 0) rx_pause = value;
    taskEXIT_CRITICAL();
}

unsigned char getRxByte(unsigned short i)
{
    if(i<UART_BUF_SIZE) return rx_buf[i];
    else return 0;
}

void setTxByte(unsigned short i, unsigned char value)
{
    if(i<UART_BUF_SIZE) tx_buf[i] = value;
}

unsigned short writeBuf(unsigned short cnt)
{
    if(GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_5)==Bit_RESET)
    {
        if(cnt)
        {
            if(cnt <= UART_BUF_SIZE) {write_uart(cnt);return cnt;}
            else{write_uart(UART_BUF_SIZE); return UART_BUF_SIZE;}
        }
    }
    return 0;
}

void write_start_message(void)
{
    unsigned short tmp;
    tx_buf[0]='\r';tx_buf[1]='\n';
    for(tmp=0;tmp<sizeof(helloString);tmp++) tx_buf[2+tmp]=helloString[tmp];
    tx_buf[2+tmp++]='\r';tx_buf[2+tmp++]='\n';
    write_uart(2+tmp);
}

void clearRxBuf(void)
{
    taskENTER_CRITICAL();
    rx_cnt=0;
    rx_tmr=0;
    taskEXIT_CRITICAL();
}

void write_uart(unsigned short cnt)
{
    clearRxBuf();

	DMA_InitTypeDef DMA_InitStructure;
	if(cnt>UART_BUF_SIZE) return;
	GPIO_WriteBit(GPIOB, GPIO_Pin_5, Bit_SET);
	DMA_DeInit(DMA1_Channel4);
	DMA_InitStructure.DMA_PeripheralBaseAddr = USART1_DR_Base;
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)tx_buf;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize = cnt;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel4, &DMA_InitStructure);
	DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);
	USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
	DMA_Cmd(DMA1_Channel4, ENABLE);
}

void USART1_IRQHandler(void)
{
    if(USART_GetITStatus(USART1, USART_IT_TC) != RESET)
    {
        USART_ClearITPendingBit(USART1, USART_IT_TC);
        if(USART_GetFlagStatus(USART1, USART_FLAG_TXE)!=RESET)
        {
            GPIO_WriteBit(GPIOB, GPIO_Pin_5, Bit_RESET);
            USART_ITConfig(USART1, USART_IT_TC, DISABLE);
            USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
        }
    }
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        rx_buf[rx_cnt]=USART_ReceiveData(USART1);
        rx_cnt++;
        if(rx_cnt>=UART_BUF_SIZE) rx_cnt=0;
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
        rx_tmr = 0;
    }
}

void DMA1_Channel4_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_IT_TC4) != RESET)
	{
		DMA_Cmd(DMA1_Channel4, DISABLE);
		DMA_ClearITPendingBit(DMA1_IT_GL4);
		USART_ITConfig(USART1, USART_IT_TC, ENABLE);
	}
}
