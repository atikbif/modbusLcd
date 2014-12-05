#include "settings.h"
#include "flashUpdate.h"
#include "stm32f10x_conf.h"

// settings are saved in flash
// 0 - version
// 2 - rowCount (4)
// 4 - amount of first strings
// 6 - amount of second strings
// 8 - amount of third strings
// 10 - amount of fourth strings
// one page is reserved (include version and settings)
// 0 (2-nd page) series data of strings (1,2,3,4)

static unsigned long strStartAddr[4]; // start address of first strings, second strings, third strings, fourth strings.
static unsigned short strMaxNum[4]={1,1,1,1};  // amount of strings in row
static unsigned short reqAmount = 0;
static unsigned long reqAddr = 0;
static unsigned short varsCount = 0;

static unsigned long getVarsAddr(void);
static unsigned long getVarsOnLcdAddr(void);


unsigned short getVersion(void)
{
    unsigned short vers = (*(__IO uint16_t*) DATA_START_ADDRESS);
    return vers;
}

void readSettings(void)
{
    unsigned long addr;
    if(getVersion() == 0x01)
    {
        addr = DATA_START_ADDRESS + 4; // address of amount of first strings
        strMaxNum[0] = *(__IO uint16_t*)addr;addr+=2;
        strMaxNum[1] = *(__IO uint16_t*)addr;addr+=2;
        strMaxNum[2] = *(__IO uint16_t*)addr;addr+=2;
        strMaxNum[3] = *(__IO uint16_t*)addr;

        strStartAddr[0] = DATA_START_ADDRESS + PAGE_SIZE;   // address of begin of first strings
        strStartAddr[1] = strStartAddr[0] + strMaxNum[0]*20; // address of begin of second strings
        strStartAddr[2] = strStartAddr[1] + strMaxNum[1]*20; // third
        strStartAddr[3] = strStartAddr[2] + strMaxNum[2]*20; // fourth

        addr = DATA_START_ADDRESS + 12; // address of vars' count
        varsCount = *(__IO uint16_t*)addr;

        addr = DATA_START_ADDRESS + 16; // address of requests' amount
        reqAmount = *(__IO uint16_t*)addr;

        reqAddr = strStartAddr[3] + strMaxNum[3]*20;
    }
}

unsigned short getVarsCount(void)
{
    return varsCount;
}

unsigned short getStrMaxNum(unsigned char sNum)
{
    if(sNum<4) return strMaxNum[sNum];
    return 1;
}

unsigned long getStrStartAddr(unsigned char sNum)
{
    if(sNum<4) return strStartAddr[sNum];
    return strStartAddr[0];
}

unsigned short getRequestsCount(void) // this function must be called after getReqHead()
{
    if(getVersion()==0x01)
    {
        return reqAmount;
    }
    return 0;
}

unsigned long getReqAddr(void)
{
    return reqAddr;
}

unsigned long getVarsAddr(void)
{
    unsigned long addr = getReqAddr() + getRequestsCount()*6;
    return addr;
}

unsigned long getVarsOnLcdAddr(void)
{
    unsigned long addr = getVarsAddr() + getVarsCount() * 8 + 2; // 8 bytes for each variable, 2 bytes offset
    return addr;
}

unsigned long getVarsOnLcdCount(void)
{
    unsigned long addr = getVarsOnLcdAddr() - 2; // 2 bytes before start of data
    return *(__IO uint16_t*)addr;
}

void getVar(varDef* vDef, unsigned short vIndex)
{
    if((vIndex<getVarsCount())&&(vDef!=0))
    {
        unsigned long addr = getVarsAddr() + (unsigned long)vIndex*8;
        if((addr<DATA_START_ADDRESS)||(addr>DATA_MAX_ADDRESS)) return;
        addr+=2; // offset for net address
        vDef->netAddress = *(__IO uint8_t*)addr;
        addr++; // offset for type code
        vDef->typeCode = *(__IO uint8_t*)addr;
        addr++; // offset for memory address
        vDef->memAddress = *(__IO uint16_t*)addr;
        addr+=2; // offset for flags
        vDef->flags = *(__IO uint8_t*)addr;
    }
}

void getVarOnLcd(varLcdDef* vDef, unsigned short vIndex)
{
    if((vIndex<getVarsOnLcdCount())&&(vDef!=0))
    {
        unsigned long addr = getVarsOnLcdAddr() + vIndex * 8; // 8 bytes for each definition
        if((addr<DATA_START_ADDRESS)||(addr>DATA_MAX_ADDRESS)) return;
        vDef->symbBeforeDot = *(__IO uint8_t*)addr;
        addr++;
        vDef->symbAfterDot = *(__IO uint8_t*)addr;
        addr++;
        vDef->rowNum = *(__IO uint8_t*)addr;
        addr++;
        vDef->lineNum = *(__IO uint8_t*)addr;
        addr++;
        vDef->posInLine = *(__IO uint8_t*)addr;
        addr++;
        vDef->idInVarDef = *(__IO uint16_t*)addr;
    }
}
