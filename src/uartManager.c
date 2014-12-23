#include "stm32f10x_conf.h"

#include "uartManager.h"
#include "tableOfRequests.h"
#include "modbusMemory.h"
#include "flashUpdate.h"
#include "crc.h"
#include "uart.h"
#include "task.h"



#define START_PAGE          100
#define END_PAGE            150

static unsigned char uartMode = UART_WORK_MODE;//UART_PROG_MODE;//UART_WORK_MODE;
static unsigned char waitAnswerTime = 50;
static unsigned char attemptMax = 3;
unsigned short req_i = 0;
unsigned char attemptNum = 0;
modbReq* request;

void UartManagerTask( void *pvParameters )
{
    unsigned short tmp,crc_value;
    unsigned short req_addr,req_cnt;
    portTickType xLastExecutionTime;
    xLastExecutionTime = xTaskGetTickCount();
    initFlashWrite();
    vTaskDelayUntil( &xLastExecutionTime, ( ( portTickType ) 100 / portTICK_RATE_MS ) );
    initModbusMemory();
    for( ;; )
	{
	    if(uartMode == UART_WORK_MODE)
	    {
	        if(req_i<getRequestsCount())
            {
                request = getRequest(req_i);
                req_i++;
                // form request buffer
                tmp = get_cmd_length(request);
                if((tmp)&&(tmp<getUartBufSize()))
                {
                    attemptNum++;
                    request->tx_ptr = (unsigned char*)getTxPtr();
                    fill_cmd_buf(request);
                    // send request
                    writeBuf(tmp);

                    // wait answer
                    vTaskDelayUntil( &xLastExecutionTime, ( ( portTickType ) tmp / portTICK_RATE_MS ) );                // transmit data
                    vTaskDelayUntil( &xLastExecutionTime, ( ( portTickType ) waitAnswerTime / portTICK_RATE_MS ) );     // wait answer
                    // check response
                    tmp = getRxCnt();
                    if(tmp)
                    {

                        request->rx_ptr = (unsigned char*)getRxPtr();
                        if(check_answer(request,tmp))
                        {
                            attemptNum = 0;
                            // copy read data
                            updateMemory(request);
                        }
                    }
                    if(attemptNum)
                    {
                        if(attemptNum<attemptMax) req_i--;else{attemptNum = 0;markAsBadInfo(request);}
                    }

                }
                // send request from write queue
                // send current key status
            }else req_i=0;
	    }
	    else
        {
            if(isRxDataReady())
            {
                //GPIOB->ODR ^= GPIO_Pin_12;
                tmp = getRxCnt();
                if(GetCRC16((unsigned char*)getRxPtr(),tmp)==0)
                {
                    if(getRxByte(0)==0x01)
                    {
                        switch(getRxByte(1))
                        {
                            case 0xA0:  // getVersion
                                setTxByte(0,0x01);
                                setTxByte(1,0xA0);
                                setTxByte(2,'m');setTxByte(3,'p');setTxByte(4,'u');setTxByte(5,'0');setTxByte(6,'1');
                                crc_value = GetCRC16((unsigned char*)getTxPtr(),7);
                                setTxByte(7,crc_value>>8);
                                setTxByte(8,crc_value&0xFF);
                                writeBuf(9);
                                break;
                            case 0xA1:  // erasePage
                                tmp = getRxByte(2);
                                tmp = (tmp>>8) | getRxByte(3);
                                if(tmp<END_PAGE)
                                {
                                    setTxByte(0,0x01);
                                    setTxByte(1,0xA1);
                                    crc_value = GetCRC16((unsigned char*)getTxPtr(),2);
                                    setTxByte(2,crc_value>>8);
                                    setTxByte(3,crc_value&0xFF);
                                    writeBuf(4);
                                    taskENTER_CRITICAL();
                                    erasePage(tmp);
                                    taskEXIT_CRITICAL();
                                }
                                break;
                            case 0xA2:  // isReadyForNextOperation
                                setTxByte(0,0x01);
                                setTxByte(1,0xA2);
                                setTxByte(2,1);//isFlashBusy()?0:1);
                                crc_value = GetCRC16((unsigned char*)getTxPtr(),3);
                                setTxByte(3,crc_value>>8);
                                setTxByte(4,crc_value&0xFF);
                                writeBuf(5);
                                break;
                            case 0xA3:  // writePartOfBufferForPage
                                // page num
                                req_addr = getRxByte(2);
                                req_addr = (req_addr << 8) | getRxByte(3);
                                // offset
                                tmp = getRxByte(4);
                                tmp = (tmp << 8) | getRxByte(5);
                                // count of bytes
                                req_cnt = getRxByte(6);
                                req_cnt = (req_cnt << 8) | getRxByte(7);
                                taskENTER_CRITICAL();
                                writePageBuf(req_addr,tmp,req_cnt,((unsigned char*)getRxPtr())+8);
                                taskEXIT_CRITICAL();
                                setTxByte(0,0x01);
                                setTxByte(1,0xA3);
                                crc_value = GetCRC16((unsigned char*)getTxPtr(),2);
                                setTxByte(2,crc_value>>8);
                                setTxByte(3,crc_value&0xFF);
                                writeBuf(4);
                                break;
                            case 0xA4:  // readPage
                                // page address
                                req_addr = getRxByte(2);
                                req_addr = (req_addr << 8) | getRxByte(3);
                                // offset inside page
                                tmp = getRxByte(4);
                                tmp = (tmp << 8) | getRxByte(5);
                                // count of bytes
                                req_cnt = getRxByte(6);
                                req_cnt = (req_cnt << 8) | getRxByte(7);
                                if(readPage(req_addr,tmp,req_cnt,(unsigned char*)getTxPtr()+2))
                                {
                                    setTxByte(0,0x01);
                                    setTxByte(1,0xA5);
                                    crc_value = GetCRC16((unsigned char*)getTxPtr(),2+req_cnt);
                                    setTxByte(2+req_cnt,crc_value>>8);
                                    setTxByte(3+req_cnt,crc_value&0xFF);
                                    writeBuf(4+req_cnt);
                                }
                                else    // incorrect input parameters
                                {
                                    setTxByte(0,0x01);
                                    setTxByte(1,0xFF);
                                    setTxByte(2,0xA5);
                                    crc_value = GetCRC16((unsigned char*)getTxPtr(),3);
                                    setTxByte(3,crc_value>>8);
                                    setTxByte(4,crc_value&0xFF);
                                    writeBuf(5);
                                }

                                break;
                        }
                    }
                }
                clearRxBuf();
            }
        }
        vTaskDelayUntil( &xLastExecutionTime, UART_MANAGER_DELAY );
    }
}

void setMode(unsigned char modeValue)
{
    if(modeValue>=UART_UNDEF_MODE) uartMode = UART_WORK_MODE;
    else uartMode = modeValue;
}

void setWaitAnswerPauseMs(unsigned char value)
{
    if(value) waitAnswerTime = value;
}

void setAttemptOfRequestCount(unsigned char value)
{
    if((value>0)&&(value<=10)) attemptMax = value;
}

unsigned char getUartMode(void)
{
    return uartMode;
}
