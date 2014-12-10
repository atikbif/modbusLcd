#include "lcdManager.h"
#include "displayData.h"
#include "lcd.h"
#include "keyScan.h"
#include "settings.h"
#include "stm32f10x_conf.h"

static unsigned short strNum[4]={0,0,0,0};
const unsigned short keyTimeLimit = 5;
static unsigned char butState = 0;
static unsigned char varDigits[10];
static unsigned long vValue;
static varLcdDef vLcdDef;
static varDef vDef;
static unsigned char lcdBufOffset = 0; // start position for string in 80 bytes lcd data buffer

static void printVars(void);

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
    unsigned short tmp = 0;
    unsigned short buttons = 0;
    unsigned short keyTmr = 0;
    portTickType xLastExecutionTime;
    xLastExecutionTime = xTaskGetTickCount();
    vTaskDelayUntil( &xLastExecutionTime, ( ( portTickType ) 300 / portTICK_RATE_MS ) );

    // if normal mode (without program, menu or edit)
    for( ;; )
	{
	    switch(butState)
	    {
            case 0: // wait press
                buttons = getFilteredKeys();
                if(buttons) {keyTmr++;if(keyTmr >= keyTimeLimit) butState++;}else keyTmr=0;
                break;
            case 1: // reaction
                keyTmr = 0;
                if(buttons)
                {
                    if(buttons == 1<<Key_Right)
                    {
                        if(isStrVarEnable(0)==0) {strNum[0]++;if(strNum[0]>=getStrMaxNum(0)) strNum[0]--;}
                        if(isStrVarEnable(1)==0) {strNum[1]++;if(strNum[1]>=getStrMaxNum(1)) strNum[1]--;}
                        if(isStrVarEnable(2)==0) {strNum[2]++;if(strNum[2]>=getStrMaxNum(2)) strNum[2]--;}
                        if(isStrVarEnable(3)==0) {strNum[3]++;if(strNum[3]>=getStrMaxNum(3)) strNum[3]--;}
                    }else if(buttons == 1<<Key_Left)
                    {
                        if((isStrVarEnable(0)==0)&&(strNum[0])) strNum[0]--;
                        if((isStrVarEnable(1)==0)&&(strNum[1])) strNum[1]--;
                        if((isStrVarEnable(2)==0)&&(strNum[2])) strNum[2]--;
                        if((isStrVarEnable(3)==0)&&(strNum[3])) strNum[3]--;
                    }
                    butState++;
                }else butState = 0;
                break;
            case 2:
                keyTmr++;if(keyTmr>=keyTimeLimit*3) {keyTmr=0;butState=0;}
                if(buttons==0) {keyTmr=0;butState=0;}
                break;
	    }
	    // get str num values from variables
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

	    for(tmp=0;tmp<20;tmp++)
        {
            setSymbol(tmp,getSymbol(0,strNum[0],tmp));
            setSymbol(40+tmp,getSymbol(1,strNum[1],tmp));
            setSymbol(20+tmp,getSymbol(2,strNum[2],tmp));
            setSymbol(60+tmp,getSymbol(3,strNum[3],tmp));
        }
        //setSymbol(19,vValue + 0x30);
        printVars();
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
            switch(vLcdDef.rowNum)
            {
                case 0: lcdBufOffset = 0;break;
                case 1: lcdBufOffset = 40;break;
                case 2: lcdBufOffset = 20;break;
                case 3: lcdBufOffset = 60;break;
                default:lcdBufOffset = 0;break;
            }
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
                        if(vDef.flags & 0x02)
                        {
                            if(vValue>=32768) {vValue = 65535 - vValue + 1;isSign = 1;}
                        }
                    }
                break;
                case 3: // input registers
                    isValidete = validateInpReg(vDef.netAddress,vDef.memAddress);
                    if(isValidete)
                    {
                        vValue = getInpReg(vDef.netAddress,vDef.memAddress);
                        if(vDef.flags & 0x02)
                        {
                            if(vValue>=0x8000) {vValue = 0xFFFF - vValue + 1;isSign = 1;}
                        }
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
                            if(vDef.flags & 0x02)
                            {
                                if(vValue>=0x80000000ul) {vValue = 0xFFFFFFFFul - vValue + 1;isSign = 1;}
                            }
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
                            if(vDef.flags & 0x02)
                            {
                                if(vValue>=0x80000000ul) {vValue = 0xFFFFFFFFul - vValue + 1;isSign = 1;}
                            }
                        }
                    }
                break;
            }
            if(isValidete) // make digits from value
            {
                for(i=0;i<sizeof(varDigits);i++) varDigits[i]=0;
                while(vValue>=1000000) {varDigits[6]++;vValue-=1000000;}
                while(vValue>=100000) {varDigits[5]++;vValue-=100000;}
                while(vValue>=10000) {varDigits[4]++;vValue-=10000;}
                while(vValue>=1000) {varDigits[3]++;vValue-=1000;}
                while(vValue>=100) {varDigits[2]++;vValue-=100;}
                while(vValue>=10) {varDigits[1]++;vValue-=10;}
                varDigits[0] = vValue;
            }

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

