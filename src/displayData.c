#include "displayData.h"
#include "flashUpdate.h"
#include "stm32f10x_conf.h"


static unsigned long strStartAddr[4]; // start address of first strings, second strings, third strings, fourth strings.
static unsigned short strMaxNum[4]={1,1,1,1};  // amount of strings in row

// settings are saved in flash
// 0 - version
// 2 - rowCount (4)
// 4 - amount of first strings
// 6 - amount of second strings
// 8 - amount of third strings
// 10 - amount of fourth strings
// one page is reserved (include version and settings)
// 0 (2-nd page) series data of strings (1,2,3,4)

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
    }
}

// return symbol code in concrete position

unsigned char getSymbol(unsigned char rowNum, unsigned short strNum, unsigned char pos)
{
    unsigned short rdValue;
    unsigned long addr;
    if((rowNum>3)||(pos>19)) return 0x20;
    if(getVersion()==0x01)
    {
        addr = strStartAddr[rowNum] + strNum*20 + pos;
        if(addr & 0x01)
        {
            addr--;
            rdValue = (*(__IO uint16_t*) addr);
            rdValue = rdValue >> 8;
        }else
        {
            rdValue = (*(__IO uint16_t*) addr);
            rdValue = rdValue & 0xFF;
        }
        return rdValue;
    }else return '*';
}

unsigned short getStrMaxNum(unsigned char sNum)
{
    if(sNum<4) return strMaxNum[sNum];
    return 1;
}

unsigned long getAddressAfterDisplayData(void)
{
    return (strStartAddr[3] + strMaxNum[3]*20);
}
