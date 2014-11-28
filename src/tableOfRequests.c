#include "tableOfRequests.h"
#include "flashUpdate.h"
#include "displayData.h"
#include "stm32f10x_conf.h"



modbReq req;
static unsigned short reqAmount = 0;
static unsigned long reqAddr = 0;

modbReq* getRequest(unsigned short num)
{
    unsigned long addr;
    unsigned char reqType = 0;
    if(num<reqAmount)
    {
        addr = reqAddr + (unsigned long)num*6;
        reqType = *(__IO uint8_t*)addr;
        switch(reqType)
        {
            case 1: req.cmdType = CMD_RD_HLD;break;
            case 2: req.cmdType = CMD_RD_INP;break;
            case 3: req.cmdType = CMD_RD_COIL;break;
            case 4: req.cmdType = CMD_RD_DIN;break;
            default: return ((void*)0);
        }
        addr++; req.netAddress = *(__IO uint8_t*)addr;
        addr+=2; req.regCount = *(__IO uint16_t*)addr;
        addr+=2; req.memAddress = *(__IO uint16_t*)addr;
        return (&req);
    }
    return (void*)0;
}

unsigned short getRequestsCount(void) // this function must be called after getReqHead()
{
    if(getVersion()==0x01)
    {
        return reqAmount;
    }
    return 0;
}

void getReqHead(void)
{
    unsigned long addr;
    if(getVersion()==0x01)
    {
        addr = DATA_START_ADDRESS + 16; // address of requests' amount
        reqAmount = *(__IO uint16_t*)addr;
        readSettings();
        reqAddr = getAddressAfterDisplayData();
    }
}
