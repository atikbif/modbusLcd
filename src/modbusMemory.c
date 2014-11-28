#include "modbusMemory.h"
#include "tableOfRequests.h"

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
        nReq->next = (MemReq*)(nReq->bufPtr + nReq->bufSize);
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

unsigned short getHoldReg(unsigned short addr)
{
    return addr;
}

unsigned char validateHoldReg(unsigned short addr)
{
    return addr?1:0;
}

unsigned short getInpReg(unsigned short addr)
{
    return addr;
}

unsigned char validateInpReg(unsigned short addr)
{
    return addr?1:0;
}

void updateMemory(modbReq* reqStruct)
{
    unsigned char equalReq = 0;
    unsigned short tmp;
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
                    // set a flag which marks correct data
                    nReq->flags |= 0x0001;
                }
            }
        }
        if(nReq->next == 0) break;
        nReq = nReq->next;
    }
}

void markAsBadInfo(modbReq* reqStruct)
{
    unsigned char equalReq = 0;
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

unsigned char getCoil(unsigned short addr)
{
    return addr&0x01;
}

unsigned char validateCoil(unsigned short addr)
{
    return addr?1:0;
}

unsigned char getDiscrInput(unsigned short addr)
{
    return addr&0x01;
}

unsigned char validateDiscrInput(unsigned short addr)
{
    return addr?1:0;
}
