#include "displayData.h"
#include "stm32f10x_conf.h"
#include "settings.h"

// return symbol code in concrete position

unsigned char getSymbol(unsigned char rowNum, unsigned short strNum, unsigned char pos)
{
    unsigned short rdValue;
    unsigned long addr;
    if((rowNum>3)||(pos>19)) return 0x20;
    if(getVersion()==0x01)
    {
        addr = getStrStartAddr(rowNum) + strNum*20 + pos;
        /*if(addr & 0x01)
        {
            addr--;
            rdValue = (*(__IO uint16_t*) addr);
            rdValue = rdValue >> 8;
        }else
        {
            rdValue = (*(__IO uint16_t*) addr);
            rdValue = rdValue & 0xFF;
        }*/
        rdValue = (*(__IO uint8_t*) addr);
        return rdValue;
    }else return '*';
}


