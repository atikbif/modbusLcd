#include "tableOfRequests.h"
#include "settings.h"
#include "stm32f10x_conf.h"



modbReq req;


modbReq* getRequest(unsigned short num)
{
    unsigned long addr;
    unsigned char reqType = 0;
    if(num<getRequestsCount())
    {
        addr = getReqAddr() + (unsigned long)num*6;
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
        addr++; req.regCount = *(__IO uint16_t*)addr;
        addr+=2; req.memAddress = *(__IO uint16_t*)addr;
        return (&req);
    }
    return (void*)0;
}


