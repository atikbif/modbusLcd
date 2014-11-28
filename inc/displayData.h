#ifndef DISPLAYDATA_H_INCLUDED
#define DISPLAYDATA_H_INCLUDED

// operations with lcd data saved in flash memory

unsigned char getSymbol(unsigned char rowNum, unsigned short strNum, unsigned char pos);
void readSettings(void);
unsigned short getStrMaxNum(unsigned char sNum);
unsigned short getVersion(void);
unsigned long getAddressAfterDisplayData(void);


#endif /* DISPLAYDATA_H_INCLUDED */
