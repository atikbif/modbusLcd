#include "flashUpdate.h"
#include "stm32f10x_conf.h"

// read write operations with flash

void initFlashWrite(void)
{
    FLASH_Unlock();
}

unsigned char erasePage(unsigned short pageNum)
{
    FLASH_Status flSt = FLASH_COMPLETE;
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    flSt = FLASH_ErasePage(DATA_START_ADDRESS + pageNum * PAGE_SIZE);
    if(flSt = FLASH_COMPLETE) return 1;
    return 0;
}

unsigned char isFlashBusy(void)
{
    if(FLASH_GetStatus()==FLASH_FLAG_BSY) return 1;
    else return 0;
}

void writePageBuf(unsigned short pageNum, unsigned short offset, unsigned short cnt, unsigned char* buf)
{
    unsigned long addr = DATA_START_ADDRESS + pageNum * PAGE_SIZE + offset;
    unsigned short i;
    unsigned short rdFlash;
    if(addr>DATA_MAX_ADDRESS) return;
    if((addr & 0x01) || (cnt & 0x01)) return;
    for(i=0;i<cnt/2;i++)
    {
        FLASH_ProgramHalfWord(addr+i*2,buf[i*2] | (unsigned short)buf[i*2+1]<<8);
    }
}

unsigned char readPage(unsigned short pageNum, unsigned short offset, unsigned short cnt, unsigned char* buf)
{
    return 1;
}
