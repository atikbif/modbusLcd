#include "lcdManager.h"
#include "displayData.h"
#include "lcd.h"
#include "keyScan.h"
#include "settings.h"
#include "stm32f10x_conf.h"
#include "uartManager.h"

const char noEditVars1[] = "There are no ";
const char noEditVars2[] = "editable variables";
const char noEditVars3[] = "here";
const char noEditVars4[] = "";
static unsigned short strNum[4]={0,0,0,0};
const unsigned short keyTimeLimit = 10;
const unsigned char markerOnTime = 30;
const unsigned char markerOffTime = 60;
static unsigned char butState = 0;
static unsigned char varDigits[10];
static unsigned long vValue;
static varLcdDef vLcdDef;
static varDef vDef;
static unsigned char lcdBufOffset = 0; // start position for string in 80 bytes lcd data buffer
static unsigned char lcdEdit[80];
static unsigned short editMask[80];
static unsigned char markerPos = 0;
static unsigned char markerTmr = 0;
//static unsigned short tmp = 0;

static void printVars(void);
static void goToNextStrings(void);
static void goToPrevStrings(void);
static void getStrNumFromVariables(void);
static unsigned char getLcdBufOffset(unsigned char lineNum);
static unsigned long getAbsValueForSignVar(unsigned long varValue,unsigned char bytesCount, unsigned char* isNegative);
static unsigned char isVarSign(unsigned short flagsValue);
static unsigned short formDigits(unsigned long value);
static void buttonManagerForEditMode(void);
static void buttonManagerForWorkMode(void);
static void buttonManagerForServiceMode(void);
static unsigned char lcdMode = LCD_WORK;
static unsigned short buttons = 0;
static unsigned short keyTmr = 0;
static void updateLcdForWorkMode(void);
static void updateLcdForEditMode(void);
static void updateLcdForConfMode(void);
static void createEditBuf(void);
static void searchEditVars(void);
static void edit_right(void);
static void edit_left(void);
static void edit_up(void);
static void edit_down(void);
static void addWriteRequest(void);

unsigned short getStrNum(unsigned char rowNum)
{
    if(rowNum>3) return 0;
    else return strNum[rowNum];
}

void setStrNum(unsigned char rowNum, unsigned short value)
{
    if(rowNum>3) return;
    else strNum[rowNum] = value;
}

void LcdManagerTask( void *pvParameters )
{
    portTickType xLastExecutionTime;
    xLastExecutionTime = xTaskGetTickCount();
    vTaskDelayUntil( &xLastExecutionTime, ( ( portTickType ) 300 / portTICK_RATE_MS ) );
    for( ;; )
	{
	    switch(lcdMode)
	    {
            case LCD_WORK:
                buttonManagerForWorkMode();
                updateLcdForWorkMode();
                break;
            case LCD_EDIT:
                buttonManagerForEditMode();
                updateLcdForEditMode();
                break;
            case LCD_CONF:
                buttonManagerForServiceMode();
                updateLcdForConfMode();
                break;
	    }
	    vTaskDelayUntil( &xLastExecutionTime, LCD_MANAGER_DELAY );
	}
}

void printVars(void)
{
    unsigned short vNum;

    unsigned char i,j; // template variable - counter
    char isVarHere = 0;
    char isValidete = 0;
    char isSign = 0;
    unsigned short vCount = getVarsOnLcdCount();
    for(vNum=0;vNum<vCount;vNum++)
    {
        isVarHere = 0;vLcdDef.rowNum = 5;
        vLcdDef.symbBeforeDot = 0;vLcdDef.symbAfterDot = 0;
        getVarOnLcd(&vLcdDef,vNum);
        if(vLcdDef.symbBeforeDot>10) vLcdDef.symbBeforeDot = 0;
        if(vLcdDef.symbAfterDot>10) vLcdDef.symbAfterDot = 0;
        for(i=0;i<4;i++) if((vLcdDef.rowNum == i)&&(vLcdDef.lineNum == strNum[i])) isVarHere = 1;
        if(isVarHere)
        {
            isSign = 0;
            getVar(&vDef,vLcdDef.idInVarDef);
            lcdBufOffset = getLcdBufOffset(vLcdDef.rowNum);
            isValidete = 0;vValue = 0;

            switch(vDef.typeCode)
            {
                case 0:// coil
                    isValidete = validateCoil(vDef.netAddress,vDef.memAddress);
                    if(isValidete) vValue = getCoil(vDef.netAddress,vDef.memAddress);
                break;
                case 1:// discrete input
                    isValidete = validateDiscrInput(vDef.netAddress,vDef.memAddress);
                    if(isValidete) vValue = getDiscrInput(vDef.netAddress,vDef.memAddress);
                break;
                case 2: // holding registers

                    isValidete = validateHoldReg(vDef.netAddress,vDef.memAddress);
                    if(isValidete)
                    {
                        vValue = getHoldReg(vDef.netAddress,vDef.memAddress);
                        if(isVarSign(vDef.flags)) {vValue = getAbsValueForSignVar(vValue,2,&isSign);}
                    }
                break;
                case 3: // input registers
                    isValidete = validateInpReg(vDef.netAddress,vDef.memAddress);
                    if(isValidete)
                    {
                        vValue = getInpReg(vDef.netAddress,vDef.memAddress);
                        if(isVarSign(vDef.flags)) {vValue = getAbsValueForSignVar(vValue,2,&isSign);}
                    }
                break;
                case 4:// double holding registers
                    isValidete = validateHoldReg(vDef.netAddress,vDef.memAddress);
                    if(isValidete)
                    {
                        isValidete = validateHoldReg(vDef.netAddress,vDef.memAddress+1);
                        if(isValidete)
                        {
                            vValue = getHoldReg(vDef.netAddress,vDef.memAddress+1);
                            vValue = vValue << 16;
                            vValue |= getHoldReg(vDef.netAddress,vDef.memAddress);
                            if(isVarSign(vDef.flags)) {vValue = getAbsValueForSignVar(vValue,4,&isSign);}
                        }
                    }
                break;
                case 5:// double input registers
                    isValidete = validateInpReg(vDef.netAddress,vDef.memAddress);
                    if(isValidete)
                    {
                        isValidete = validateInpReg(vDef.netAddress,vDef.memAddress+1);
                        if(isValidete)
                        {
                            vValue = getInpReg(vDef.netAddress,vDef.memAddress+1);
                            vValue = vValue << 16;
                            vValue |= getInpReg(vDef.netAddress,vDef.memAddress);
                            if(isVarSign(vDef.flags)) {vValue = getAbsValueForSignVar(vValue,2,&isSign);}
                        }
                    }
                break;
            }
            if(isValidete) {formDigits(vValue);}

            // print symbols after dot
            j=0;
            if(vLcdDef.symbAfterDot)
            {
                setSymbol(lcdBufOffset + vLcdDef.posInLine + vLcdDef.symbBeforeDot,'.');
                for(i=0;i<vLcdDef.symbAfterDot;i++)
                {
                    if(isValidete)
                    {
                        if(j<sizeof(varDigits))
                        setSymbol(lcdBufOffset + vLcdDef.posInLine + vLcdDef.symbBeforeDot + vLcdDef.symbAfterDot - i,varDigits[j]+0x30);
                        else setSymbol(lcdBufOffset + vLcdDef.posInLine + vLcdDef.symbBeforeDot + vLcdDef.symbAfterDot - i, 0x30);
                    }
                    else
                    {
                        setSymbol(lcdBufOffset + vLcdDef.posInLine + vLcdDef.symbBeforeDot + vLcdDef.symbAfterDot - i,'?');
                    }
                    j++;
                }
            }
            // print symbols before dot
            for(i=0;i<vLcdDef.symbBeforeDot;i++)
            {
                if(isValidete)
                {
                    if(j<sizeof(varDigits))
                    setSymbol(lcdBufOffset + vLcdDef.posInLine + vLcdDef.symbBeforeDot - 1 - i,varDigits[j]+0x30);
                    else setSymbol(lcdBufOffset + vLcdDef.posInLine + vLcdDef.symbBeforeDot - 1 - i, 0x30);
                    if((isSign)&&(vLcdDef.posInLine)) setSymbol(lcdBufOffset + vLcdDef.posInLine - 1,'-');
                }
                else
                {
                    setSymbol(lcdBufOffset + vLcdDef.posInLine + vLcdDef.symbBeforeDot - 1 - i, '?');
                }
                j++;
            }
        }
    }
}

void goToNextStrings(void)
{
    if(isStrVarEnable(0)==0) {strNum[0]++;if(strNum[0]>=getStrMaxNum(0)) strNum[0]--;}
    if(isStrVarEnable(1)==0) {strNum[1]++;if(strNum[1]>=getStrMaxNum(1)) strNum[1]--;}
    if(isStrVarEnable(2)==0) {strNum[2]++;if(strNum[2]>=getStrMaxNum(2)) strNum[2]--;}
    if(isStrVarEnable(3)==0) {strNum[3]++;if(strNum[3]>=getStrMaxNum(3)) strNum[3]--;}
}

void goToPrevStrings(void)
{
    if((isStrVarEnable(0)==0)&&(strNum[0])) strNum[0]--;
    if((isStrVarEnable(1)==0)&&(strNum[1])) strNum[1]--;
    if((isStrVarEnable(2)==0)&&(strNum[2])) strNum[2]--;
    if((isStrVarEnable(3)==0)&&(strNum[3])) strNum[3]--;
}

void getStrNumFromVariables(void)
{
    unsigned char tmp;
    for(tmp=0;tmp<4;tmp++)
    {
        if(isStrVarEnable(tmp))
        {

            getVar(&vDef,getStrVarNum(tmp));

            switch(vDef.typeCode)
            {
                case 0:// coil
                    if(validateCoil(vDef.netAddress,vDef.memAddress))
                    {
                        vValue = getCoil(vDef.netAddress,vDef.memAddress);
                        if(vValue<getStrMaxNum(tmp)) strNum[tmp] = vValue;else strNum[tmp] = getStrMaxNum(tmp)-1;
                    }
                    break;
                case 1:// discrete input
                    if(validateDiscrInput(vDef.netAddress,vDef.memAddress))
                    {
                        vValue = getDiscrInput(vDef.netAddress,vDef.memAddress);
                        if(vValue<getStrMaxNum(tmp)) strNum[tmp] = vValue;else strNum[tmp] = getStrMaxNum(tmp)-1;
                    }
                    break;
                case 2: // holding registers

                    if(validateHoldReg(vDef.netAddress,vDef.memAddress))
                    {
                        vValue = getHoldReg(vDef.netAddress,vDef.memAddress);
                        if(vDef.flags & 0x02)
                        {
                            if(vValue>=32768) vValue = 0;
                        }
                        if(vValue<getStrMaxNum(tmp)) strNum[tmp] = vValue;else strNum[tmp] = getStrMaxNum(tmp)-1;
                    }
                    break;
                case 3: // input registers
                    if(validateInpReg(vDef.netAddress,vDef.memAddress))
                    {
                        vValue = getInpReg(vDef.netAddress,vDef.memAddress);
                        if(vDef.flags & 0x02)
                        {
                            if(vValue>=32768) vValue = 0;
                        }
                        if(vValue<getStrMaxNum(tmp)) strNum[tmp] = vValue;else strNum[tmp] = getStrMaxNum(tmp)-1;
                    }
                    break;
                case 4: // double hold registers
                    if(validateHoldReg(vDef.netAddress,vDef.memAddress+1))
                    {
                        vValue = getHoldReg(vDef.netAddress,vDef.memAddress+1);
                        vValue = vValue << 16;
                        if(validateHoldReg(vDef.netAddress,vDef.memAddress))
                        {
                            vValue |= getHoldReg(vDef.netAddress,vDef.memAddress);
                            if(vDef.flags & 0x02)
                            {
                                if(vValue>=0x80000000ul) vValue = 0;
                            }
                            if(vValue<getStrMaxNum(tmp)) strNum[tmp] = vValue;else strNum[tmp] = getStrMaxNum(tmp)-1;
                        }
                    }
                    break;
                case 5: // double input registers
                    if(validateInpReg(vDef.netAddress,vDef.memAddress+1))
                    {
                        vValue = getInpReg(vDef.netAddress,vDef.memAddress+1);
                        vValue = vValue << 16;
                        if(validateInpReg(vDef.netAddress,vDef.memAddress))
                        {
                            vValue |= getInpReg(vDef.netAddress,vDef.memAddress);
                            if(vDef.flags & 0x02)
                            {
                                if(vValue>=0x80000000ul) vValue = 0;
                            }
                            if(vValue<getStrMaxNum(tmp)) strNum[tmp] = vValue;else strNum[tmp] = getStrMaxNum(tmp)-1;
                        }
                    }
                    break;
            }
        }
    }
}

unsigned char getLcdBufOffset(unsigned char lineNum)
{
    unsigned char offsetValue;
    switch(lineNum)
    {
        case 0: offsetValue = 0;break;
        case 1: offsetValue = 40;break;
        case 2: offsetValue = 20;break;
        case 3: offsetValue = 60;break;
        default:offsetValue = 0;break;
    }
    return offsetValue;
}

unsigned long getAbsValueForSignVar(unsigned long varValue,unsigned char bytesCount, unsigned char* isNegative)
{
    if(bytesCount==2)
    {
        if(varValue>=0x8000) {varValue = 0xFFFF - varValue + 1; *isNegative = 1;}else *isNegative=0;
    }
    else if(bytesCount==4)
    {
        if(varValue>=0x80000000ul) {varValue = 0xFFFFFFFFul - varValue + 1; *isNegative = 1;}else *isNegative=0;
    }
    return varValue;
}

unsigned char isVarSign(unsigned short flagsValue)
{
    if(flagsValue & 0x02) return 1;
    return 0;
}

unsigned short formDigits(unsigned long value)
{
    unsigned char i;
    for(i=0;i<sizeof(varDigits);i++) varDigits[i]=0;
    while(value>=1000000) {varDigits[6]++;value-=1000000;}
    while(value>=100000) {varDigits[5]++;value-=100000;}
    while(value>=10000) {varDigits[4]++;value-=10000;}
    while(value>=1000) {varDigits[3]++;value-=1000;}
    while(value>=100) {varDigits[2]++;value-=100;}
    while(value>=10) {varDigits[1]++;value-=10;}
    varDigits[0] = value;
}

void buttonManagerForEditMode(void)
{
    switch(butState)
    {
        case 0:
            buttons = getFilteredKeys();
            if(buttons == 0) butState++;
            break;
        case 1: // wait press
            buttons = getFilteredKeys();
            if(buttons)
            {
                keyTmr++;if(keyTmr >= keyTimeLimit) butState++;
            }else keyTmr=0;
            break;
        case 2: // reaction
            keyTmr = 0;
            if(buttons)
            {
                if(buttons == 1<<Key_Right) {edit_right();}
                else if(buttons == 1<<Key_Left) {edit_left();}
                else if(buttons == 1<<Key_Up) {edit_up();}
                else if(buttons == 1<<Key_Down) {edit_down();}
                else if(buttons == 1<<Key_Enter) {addWriteRequest();lcdMode = LCD_WORK;keyTmr=0;butState=0;break;}
                else if(buttons == 1<<Key_Shift) {lcdMode = LCD_WORK;keyTmr=0;butState=0;break;}
                butState++;
            }else butState = 0;
            break;
        case 3:
            keyTmr++;if(keyTmr>=keyTimeLimit) {keyTmr=0;butState=1;}
            if(buttons==0) {keyTmr=0;butState=0;}
            break;
    }
}

void buttonManagerForWorkMode(void)
{
    switch(butState)
    {
        case 0:
            buttons = getFilteredKeys();
            if(buttons==0) butState++;
            break;
        case 1: // wait press
            buttons = getFilteredKeys();
            if(buttons)
            {
                keyTmr++;if(keyTmr >= keyTimeLimit) butState++;

            }else keyTmr=0;
            break;
        case 2: // reaction
            keyTmr = 0;
            if(buttons)
            {
                if(buttons == 1<<Key_Right) {goToNextStrings();}
                else if(buttons == 1<<Key_Left) {goToPrevStrings();}
                else if(buttons == 1<<Key_Enter) {lcdMode=LCD_EDIT;keyTmr=0;butState=0;createEditBuf();break;}
                else if(buttons == (1<<Key_Shift)+(1<<Key_Enter))
                    {
                        lcdMode=LCD_CONF;keyTmr=0;butState=0;
                        setMode(UART_PROG_MODE);
                        break;
                    }
                butState++;
            }else butState = 0;
            break;
        case 3:
            keyTmr++;if(keyTmr>=keyTimeLimit) {keyTmr=0;butState=1;}
            if(buttons==0) {keyTmr=0;butState=0;}
            break;
    }
}

void buttonManagerForServiceMode(void)
{
    switch(butState)
    {
        case 0:
            buttons = getFilteredKeys();
            if(buttons==0) butState++;
            break;
        case 1: // wait press
            buttons = getFilteredKeys();
            if(buttons)
            {
                keyTmr++;if(keyTmr >= keyTimeLimit) butState++;

            }else keyTmr=0;
            break;
        case 2: // reaction
            keyTmr = 0;
            if(buttons)
            {
                if(buttons == 1<<Key_Right) {;}
                else if(buttons == 1<<Key_Left) {;}
                else if(buttons == 1<<Key_Enter) {lcdMode=LCD_EDIT;keyTmr=0;butState=0;createEditBuf();break;}
                else if(buttons == (1<<Key_Shift)+(1<<Key_Enter))
                    {
                        lcdMode=LCD_WORK;keyTmr=0;butState=0;
                        setMode(UART_WORK_MODE);
                        break;
                    }
                butState++;
            }else butState = 0;
            break;
        case 3:
            keyTmr++;if(keyTmr>=keyTimeLimit) {keyTmr=0;butState=1;}
            if(buttons==0) {keyTmr=0;butState=0;}
            break;
    }
}

void updateLcdForWorkMode(void)
{
    unsigned char tmp;
    getStrNumFromVariables();
    for(tmp=0;tmp<20;tmp++)
    {
        setSymbol(tmp,getSymbol(0,strNum[0],tmp));
        setSymbol(40+tmp,getSymbol(1,strNum[1],tmp));
        setSymbol(20+tmp,getSymbol(2,strNum[2],tmp));
        setSymbol(60+tmp,getSymbol(3,strNum[3],tmp));
    }
    printVars();
}

void updateLcdForEditMode(void)
{
    unsigned char tmp;

    if(markerPos<80)
    {
        for(tmp=0;tmp<80;tmp++)
        {
            setSymbol(tmp,lcdEdit[tmp]);
        }
        if(markerTmr<markerOnTime) setSymbol(getLcdBufOffset(markerPos/20) + (markerPos%20),0xFF);
        if(markerTmr>=markerOnTime+markerOffTime) markerTmr=0;
        markerTmr++;
    }
    else
    {
        for(tmp=0;tmp<20;tmp++)
        {
            if(tmp<sizeof(noEditVars1)-1) setSymbol(tmp,noEditVars1[tmp]); else setSymbol(tmp,' ');
            if(tmp<sizeof(noEditVars2)-1) setSymbol(40+tmp,noEditVars2[tmp]); else setSymbol(40+tmp,' ');
            if(tmp<sizeof(noEditVars3)-1) setSymbol(20+tmp,noEditVars3[tmp]); else setSymbol(20+tmp,' ');
            if(tmp<sizeof(noEditVars4)-1) setSymbol(60+tmp,noEditVars4[tmp]); else setSymbol(60+tmp,' ');
        }
    }
}

void updateLcdForConfMode(void)
{
    unsigned char tmp;
    for(tmp=0;tmp<20;tmp++)
    {
        setSymbol(tmp,'E');
        setSymbol(40+tmp,'E');
        setSymbol(20+tmp,'E');
        setSymbol(60+tmp,'E');
    }
}

void createEditBuf(void)
{
    unsigned char tmp;
    unsigned char* ptr = getLcdBuf();
    for(tmp=0;tmp<80;tmp++)
    {
        lcdEdit[tmp] = ptr[tmp];
    }
    searchEditVars();
    markerPos = 80;
    for(tmp=0;tmp<20;tmp++) if(editMask[tmp]) {markerPos = tmp;}else {if(markerPos!=80) break;}
    if(markerPos==80) for(tmp=0;tmp<20;tmp++) if(editMask[tmp+20]) {markerPos = tmp+20;}else {if(markerPos!=80) break;}
    if(markerPos==80) for(tmp=0;tmp<20;tmp++) if(editMask[tmp+40]) {markerPos = tmp+40;}else {if(markerPos!=80) break;}
    if(markerPos==80) for(tmp=0;tmp<20;tmp++) if(editMask[tmp+60]) {markerPos = tmp+60;}else {if(markerPos!=80) break;}
}

void searchEditVars(void)
{
    unsigned char tmp;
    unsigned short vNum;
    unsigned char varDigitsCount = 0;
    char isVarHere = 0;
    unsigned short vCount = getVarsOnLcdCount();

    for(tmp=0;tmp<80;tmp++) editMask[tmp] = 0;
    for(vNum=0;vNum<vCount;vNum++)
    {
        isVarHere = 0;
        getVarOnLcd(&vLcdDef,vNum);
        for(tmp=0;tmp<4;tmp++) if((vLcdDef.rowNum == tmp)&&(vLcdDef.lineNum == strNum[tmp])) isVarHere = 1;
        if(isVarHere)
        {
            getVar(&vDef,vLcdDef.idInVarDef);
            if(vDef.flags & 1)
            {
                varDigitsCount = vLcdDef.symbBeforeDot;
                if(vLcdDef.symbAfterDot) varDigitsCount += 1 + vLcdDef.symbAfterDot;
                for(tmp=0;tmp<varDigitsCount;tmp++) editMask[vLcdDef.rowNum * 20 + vLcdDef.posInLine + tmp] = vNum+1;
            }
        }
    }
}

void edit_right(void)
{
    unsigned char tmp;
    if(markerPos!=80)
    {
        for(tmp=markerPos;tmp<79;tmp++)
        {
            if(editMask[tmp+1]) {markerPos = tmp+1;break;}
        }
    }
}

void edit_left(void)
{

    if(markerPos!=80)
    {
        unsigned char tmp = markerPos;
        while(1)
        {
            if(tmp)
            {
                if(editMask[tmp-1]) {markerPos = tmp-1;break;}
                tmp--;
            }else break;
        }
    }
}

void edit_up(void)
{
    unsigned char strNum;
    unsigned char posInStr;
    unsigned char offset;
    if(markerPos != 80)
    {
        strNum = markerPos/20;
        posInStr = markerPos % 20;
        offset = getLcdBufOffset(strNum) + posInStr;
        if((lcdEdit[offset] >='0')&&(lcdEdit[offset]<'9'))
        {
            lcdEdit[offset]++;
        }
    }
}

void edit_down(void)
{
    unsigned char strNum;
    unsigned char posInStr;
    unsigned char offset;
    if(markerPos != 80)
    {
        strNum = markerPos/20;
        posInStr = markerPos % 20;
        offset = getLcdBufOffset(strNum) + posInStr;
        if((lcdEdit[offset] >='1')&&(lcdEdit[offset]<='9'))
        {
            lcdEdit[offset]--;
        }
    }
}

void addWriteRequest(void)
{
    if(markerPos!=80)
    {
        // calculate variable's value and add to write queue
    }
}
