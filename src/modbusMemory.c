#include "modbusMemory.h"
#include "tableOfRequests.h"
#include "stm32f10x_conf.h"

#define reqBufSize  (6*1024ul)

typedef struct MemReqDef
{
    unsigned char netAddr;
    unsigned char reqType;
    unsigned short memAddr;
    unsigned short dataAmount;
    unsigned short flags;
    unsigned char* bufPtr;
    unsigned short bufSize;
    struct MemReqDef* next;
}MemReq;

MemReq* firstReq = 0;
MemReq* nReq;
modbReq* tmpReq;

static unsigned char membuf[reqBufSize];

static void addMemForReq(unsigned short rNum)
{
    tmpReq = getRequest(rNum);
    if(tmpReq==0) return;
    if(firstReq==0) // first record
    {
        firstReq = (MemReq*)&membuf[0];
        nReq = firstReq;
    }
    else
    {
        // search the last record
        nReq = firstReq;
        while(nReq->next!=0) nReq = nReq->next;
        nReq->next = (MemReq*)((unsigned char*)nReq + nReq->bufSize + sizeof(MemReq));
        nReq = nReq->next;
    }
        // take Part of Memory for the request and fill it
        nReq->netAddr = tmpReq->netAddress;
        nReq->reqType = tmpReq->cmdType;
        nReq->memAddr = tmpReq->memAddress;
        nReq->dataAmount = tmpReq->regCount;
        nReq->flags = 0;
        nReq->bufPtr = (unsigned char*)nReq + sizeof(MemReq);
        switch(nReq->reqType)
        {
            case 1:nReq->bufSize = (tmpReq->regCount)*2;break;
            case 2:nReq->bufSize = (tmpReq->regCount)*2;break;
            case 3:nReq->bufSize = (tmpReq->regCount);break;
            case 4:nReq->bufSize = (tmpReq->regCount);break;
        }
        nReq->next = 0;
}

void initModbusMemory(void)
{
    unsigned short rCount = getRequestsCount();
    unsigned short i;
    for(i=0;i<rCount;i++)
    {
        addMemForReq(i);
    }
}

unsigned short getHoldReg(unsigned char netAddress, unsigned short addr)
{
    MemReq* iReq;
    unsigned char offset;
    if(firstReq==0) return;
    iReq = firstReq;
    while(1)
    {
        if(iReq->netAddr == netAddress)
        {
            if(iReq->reqType == 0x01)
            {
                if((addr>=iReq->memAddr)&&(addr<iReq->memAddr+iReq->dataAmount))
                {
                    offset = (addr - iReq->memAddr)*2;
                    return (((unsigned short)iReq->bufPtr[offset])<<8) | iReq->bufPtr[offset+1];
                }
            }
        }
        if(iReq->next==0) break;
        iReq = iReq->next;
    }
    return 0;
}

unsigned char validateHoldReg(unsigned char netAddress, unsigned short addr)
{
    MemReq* iReq;
    if(firstReq==0) return;
    iReq = firstReq;
    while(1)
    {
        if(iReq->netAddr == netAddress)
        {
            if(iReq->reqType == 0x01)
            {
                if((addr>=iReq->memAddr)&&(addr<iReq->memAddr+iReq->dataAmount))
                {
                    return iReq->flags & 0x0001?1:0;
                }
            }
        }
        if(iReq->next==0) break;
        iReq = iReq->next;
    }
    return 0;
}

unsigned short getInpReg(unsigned char netAddress, unsigned short addr)
{
    MemReq* iReq;
    unsigned char offset;
    if(firstReq==0) return;
    iReq = firstReq;
    while(1)
    {
        if(iReq->netAddr == netAddress)
        {
            if(iReq->reqType == 0x02)
            {
                if((addr>=iReq->memAddr)&&(addr<iReq->memAddr+iReq->dataAmount))
                {
                    offset = (addr - iReq->memAddr)*2;
                    return (((unsigned short)iReq->bufPtr[offset])<<8) | iReq->bufPtr[offset+1];
                }
            }
        }
        if(iReq->next==0) break;
        iReq = iReq->next;
    }
    return 0;
}

unsigned char validateInpReg(unsigned char netAddress, unsigned short addr)
{
    MemReq* iReq;
    if(firstReq==0) return;
    iReq = firstReq;
    while(1)
    {
        if(iReq->netAddr == netAddress)
        {
            if(iReq->reqType == 0x02)
            {
                if((addr>=iReq->memAddr)&&(addr<iReq->memAddr+iReq->dataAmount))
                {
                    return iReq->flags & 0x0001?1:0;
                }
            }
        }
        if(iReq->next==0) break;
        iReq = iReq->next;
    }
    return 0;
}

void updateMemory(modbReq* reqStruct)
{
    unsigned char equalReq = 0;
    unsigned short tmp,i;
    unsigned char byteCount;
    if(firstReq==0) return;

    nReq = firstReq;
    while(1)
    {


        if(nReq->netAddr == reqStruct->netAddress)
        {
            if(nReq->reqType == reqStruct->cmdType)
            {
                if(nReq->memAddr == reqStruct->memAddress)
                {

                    // equal requests
                    // update data
                    if((nReq->reqType == 0x01)||(nReq->reqType == 0x02))
                    {

                        for(tmp=0;tmp<nReq->bufSize;tmp++) (nReq->bufPtr)[tmp] = (reqStruct->rx_ptr)[tmp+3];
                    }
                    else if((nReq->reqType == 0x03)||(nReq->reqType == 0x04))
                    {

                        byteCount = nReq->bufSize / 8;
                        if(nReq->bufSize % 8) byteCount++;
                        i = 0;
                        for(tmp=0;tmp<byteCount;tmp++)
                        {
                            // scan bits
                            for(i=0;i<8;i++)
                            {
                                if((tmp*8 + i) < nReq->bufSize)
                                {
                                    nReq->bufPtr[tmp*8 + i] = reqStruct->rx_ptr[3+tmp]&(1<<i)?1:0;

                                }
                                else break;
                            }
                        }
                        if(nReq->bufPtr[0]) GPIOB->ODR ^= GPIO_Pin_12;
                    }
                    // set a flag which marks correct data
                    nReq->flags |= 0x0001;
                }
            }
        }
        if(nReq->next == 0) break;
        //GPIOB->ODR ^= GPIO_Pin_12;
        nReq = nReq->next;
    }
}

void markAsBadInfo(modbReq* reqStruct)
{
    if(firstReq==0) return;
    nReq = firstReq;
    while(1)
    {
        if(nReq->netAddr == reqStruct->netAddress)
        {
            if(nReq->reqType == reqStruct->cmdType)
            {
                if(nReq->memAddr == reqStruct->memAddress)
                {
                    // equal requests
                    // set a flag which marks incorrect data
                    nReq->flags &= ~0x0001;
                }
            }
        }
        if(nReq->next == 0) break;
        nReq = nReq->next;
    }
}

unsigned char getCoil(unsigned char netAddress, unsigned short addr)
{
    MemReq* iReq;
    unsigned char offset;
    if(firstReq==0) return;
    iReq = firstReq;
    while(1)
    {
        if(iReq->netAddr == netAddress)
        {
            if(iReq->reqType == 0x03)
            {
                if((addr>=iReq->memAddr)&&(addr<iReq->memAddr+iReq->dataAmount))
                {
                    offset = addr - iReq->memAddr;
                    return iReq->bufPtr[offset];
                }
            }
        }
        if(iReq->next==0) break;
        iReq = iReq->next;
    }
    return 0;
}

unsigned char validateCoil(unsigned char netAddress, unsigned short addr)
{
    MemReq* iReq;
    if(firstReq==0) return;
    iReq = firstReq;
    while(1)
    {
        if(iReq->netAddr == netAddress)
        {
            if(iReq->reqType == 0x03)
            {
                if((addr>=iReq->memAddr)&&(addr<iReq->memAddr+iReq->dataAmount))
                {
                    return iReq->flags&0x00001?1:0;
                }
            }
        }
        if(iReq->next==0) break;
        iReq = iReq->next;
    }
    return 0;
}

unsigned char getDiscrInput(unsigned char netAddress, unsigned short addr)
{
    MemReq* iReq;
    unsigned char offset;
    if(firstReq==0) return;
    iReq = firstReq;
    while(1)
    {
        if(iReq->netAddr == netAddress)
        {
            if(iReq->reqType == 0x04)
            {
                if((addr>=iReq->memAddr)&&(addr<iReq->memAddr+iReq->dataAmount))
                {
                    offset = addr - iReq->memAddr;
                    return iReq->bufPtr[offset];
                }
            }
        }
        if(iReq->next==0) break;
        iReq = iReq->next;
    }
    return 0;
}

unsigned char validateDiscrInput(unsigned char netAddress, unsigned short addr)
{
    MemReq* iReq;
    if(firstReq==0) return;
    iReq = firstReq;
    while(1)
    {
        if(iReq->netAddr == netAddress)
        {
            if(iReq->reqType == 0x04)
            {
                if((addr>=iReq->memAddr)&&(addr<iReq->memAddr+iReq->dataAmount))
                {
                    return iReq->flags&0x00001?1:0;
                }
            }
        }
        if(iReq->next==0) break;
        iReq = iReq->next;
    }
    return 0;
}
