#ifndef MODBUSMEMORY_H_INCLUDED
#define MODBUSMEMORY_H_INCLUDED

#include "tableOfrequests.h"

// operations with data got from controllers

unsigned short getHoldReg(unsigned short addr);
unsigned char validateHoldReg(unsigned short addr);
unsigned short getInpReg(unsigned short addr);
unsigned char validateInpReg(unsigned short addr);
unsigned char getCoil(unsigned short addr);
unsigned char validateCoil(unsigned short addr);
unsigned char getDiscrInput(unsigned short addr);
unsigned char validateDiscrInput(unsigned short addr);
void updateMemory(modbReq* reqStruct);
void markAsBadInfo(modbReq* reqStruct);
void initModbusMemory(void);

#endif /* MODBUSMEMORY_H_INCLUDED */
