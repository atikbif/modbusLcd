#include "lcdManager.h"
#include "displayData.h"
#include "lcd.h"
#include "keyScan.h"

static unsigned short strNum[4]={0,0,0,0};
const unsigned short keyTimeLimit = 5;
static unsigned char butState = 0;

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
    vTaskDelayUntil( &xLastExecutionTime, ( ( portTickType ) 100 / portTICK_RATE_MS ) );

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
                        if(isStrNumFromVar(0)==0) {strNum[0]++;if(strNum[0]>=getStrMaxNum(0)) strNum[0]--;}
                        if(isStrNumFromVar(1)==0) {strNum[1]++;if(strNum[1]>=getStrMaxNum(1)) strNum[1]--;}
                        if(isStrNumFromVar(2)==0) {strNum[2]++;if(strNum[2]>=getStrMaxNum(2)) strNum[2]--;}
                        if(isStrNumFromVar(3)==0) {strNum[3]++;if(strNum[3]>=getStrMaxNum(3)) strNum[3]--;}
                    }else if(buttons == 1<<Key_Left)
                    {
                        if((isStrNumFromVar(0)==0)&&(strNum[0])) strNum[0]--;
                        if((isStrNumFromVar(1)==0)&&(strNum[1])) strNum[1]--;
                        if((isStrNumFromVar(2)==0)&&(strNum[2])) strNum[2]--;
                        if((isStrNumFromVar(3)==0)&&(strNum[3])) strNum[3]--;
                    }
                    butState++;
                }else butState = 0;
                break;
            case 2:
                keyTmr++;if(keyTmr>=keyTimeLimit*3) {keyTmr=0;butState=0;}
                if(buttons==0) {keyTmr=0;butState=0;}
                break;
	    }
	    printVars();

	    for(tmp=0;tmp<20;tmp++)
        {
            setSymbol(tmp,getSymbol(0,strNum[0],tmp));
            setSymbol(40+tmp,getSymbol(1,strNum[1],tmp));
            setSymbol(20+tmp,getSymbol(2,strNum[2],tmp));
            setSymbol(60+tmp,getSymbol(3,strNum[3],tmp));
        }

	    vTaskDelayUntil( &xLastExecutionTime, LCD_MANAGER_DELAY );
	}
}

void printVars(void)
{

}

unsigned short getStrVarAddress(unsigned char sNum)
{
    return 0xFFFF;
}

void setStrVarAddress(unsigned char sNum, unsigned short vAddress)
{

}

unsigned char isStrNumFromVar(unsigned char sNum)
{
    return 0;
}
