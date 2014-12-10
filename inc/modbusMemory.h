#ifndef MODBUSMEMORY_H_INCLUDED
#define MODBUSMEMORY_H_INCLUDED

#include "tableOfrequests.h"

// operations with data got from controllers

unsigned short getHoldReg(unsigned char netAddress, unsigned short addr);
unsigned char validateHoldReg(unsigned char netAddress, unsigned short addr);
unsigned short getInpReg(unsigned char netAddress, unsigned short addr);
unsigned char validateInpReg(unsigned char netAddress, unsigned short addr);
unsigned char getCoil(unsigned char netAddress, unsigned short addr);
unsigned char validateCoil(unsigned char netAddress, unsigned short addr);
unsigned char getDiscrInput(unsigned char netAddress, unsigned short addr);
unsigned char validateDiscrInput(unsigned char netAddress, unsigned short addr);
void updateMemory(modbReq* reqStruct);
void markAsBadInfo(modbReq* reqStruct);
void initModbusMemory(void);

#endif /* MODBUSMEMORY_H_INCLUDED */
