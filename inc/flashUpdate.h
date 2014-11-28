#ifndef FLASHUPDATE_H_INCLUDED
#define FLASHUPDATE_H_INCLUDED

#define DATA_START_ADDRESS          0x08010000
#define DATA_MAX_ADDRESS            0x0801F000
#define PAGE_SIZE                   0x400

// flash operations (read/write)

void initFlashWrite(void);
unsigned char erasePage(unsigned short pageNum);
unsigned char isFlashBusy(void);
void writePageBuf(unsigned short pageNum, unsigned short offset, unsigned short cnt, unsigned char* buf);
unsigned char readPage(unsigned short pageNum, unsigned short offset, unsigned short cnt, unsigned char* buf);

#endif /* FLASHUPDATE_H_INCLUDED */
