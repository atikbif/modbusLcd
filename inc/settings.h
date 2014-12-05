#ifndef SETTINGS_H_INCLUDED
#define SETTINGS_H_INCLUDED

typedef struct
{
    unsigned char netAddress;
    unsigned char typeCode;
    unsigned short memAddress;
    unsigned char flags;
}varDef;

typedef struct
{
    unsigned char symbBeforeDot;
    unsigned char symbAfterDot;
    unsigned char rowNum;
    unsigned char lineNum;
    unsigned char posInLine;
    unsigned short idInVarDef;
}varLcdDef;


unsigned short getVersion(void);
void readSettings(void);

unsigned long getStrStartAddr(unsigned char sNum);
unsigned short getStrMaxNum(unsigned char sNum);

unsigned short getRequestsCount(void);
unsigned long getReqAddr(void);

unsigned short getVarsCount(void);
void getVar(varDef* vDef, unsigned short vIndex);
unsigned long getVarsOnLcdCount(void);
void getVarOnLcd(varLcdDef* vDef, unsigned short vIndex);



#endif /* SETTINGS_H_INCLUDED */
