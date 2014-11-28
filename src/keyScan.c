#include "keyScan.h"
#include "stm32f10x_conf.h"

#define KEY_F1_Pin      GPIO_Pin_5
#define KEY_F1_Port     GPIOA
#define KEY_Left_Pin    GPIO_Pin_6
#define KEY_Left_Port   GPIOA
#define KEY_Up_Pin      GPIO_Pin_15
#define KEY_Up_Port     GPIOA
#define KEY_Right_Pin   GPIO_Pin_3
#define KEY_Right_Port  GPIOB
#define KEY_F2_Pin      GPIO_Pin_1
#define KEY_F2_Port     GPIOA
#define KEY_Shift_Pin   GPIO_Pin_0
#define KEY_Shift_Port  GPIOA
#define KEY_Down_Pin    GPIO_Pin_9
#define KEY_Down_Port   GPIOB
#define KEY_Enter_Pin   GPIO_Pin_8
#define KEY_Enter_Port  GPIOB

static void init_buttons(void);
static unsigned short getAllKeys(void);
static unsigned char getKey(unsigned char num);
static unsigned short fKeys;    // filtred value of keys
const unsigned char filterTime = 5; // value * 10 ms
static unsigned short prevKeysValue=0;
static unsigned short currentKeys = 0;

void KeyTask( void *pvParameters )
{
    unsigned char fCnt = 0;
    portTickType xLastExecutionTime;
    xLastExecutionTime = xTaskGetTickCount();
    init_buttons();
    // filter keys value
    for( ;; )
	{
	    currentKeys = getAllKeys();
	    if(prevKeysValue!=currentKeys)
        {
            fCnt = 0;
            prevKeysValue = currentKeys;
        }else
        {
            fCnt++;
            if(fCnt>=filterTime)
            {
                fCnt = 0;
                fKeys = prevKeysValue;
            }
        }
	    vTaskDelayUntil( &xLastExecutionTime, KEY_DELAY );
	}
}

unsigned short getFilteredKeys(void)
{
    return fKeys;
}

unsigned short getAllKeys(void)
{
    unsigned short buttons = 0;
    if(getKey(Key_F1)) buttons |= 0x01;
    if(getKey(Key_Left)) buttons |= 0x02;
    if(getKey(Key_Up)) buttons |= 0x04;
    if(getKey(Key_Right)) buttons |= 0x08;
    if(getKey(Key_F2)) buttons |= 0x10;
    if(getKey(Key_Shift)) buttons |= 0x20;
    if(getKey(Key_Down)) buttons |= 0x40;
    if(getKey(Key_Enter)) buttons |= 0x80;
    return buttons;
}

unsigned char getKey(unsigned char num)
{
    switch(num)
    {
        case Key_F1:return GPIO_ReadInputDataBit(KEY_F1_Port,KEY_F1_Pin)?0:1;
        case Key_Left:return GPIO_ReadInputDataBit(KEY_Left_Port,KEY_Left_Pin)?0:1;
        case Key_Up:return GPIO_ReadInputDataBit(KEY_Up_Port,KEY_Up_Pin)?0:1;
        case Key_Right:return GPIO_ReadInputDataBit(KEY_Right_Port,KEY_Right_Pin)?0:1;
        case Key_F2:return GPIO_ReadInputDataBit(KEY_F2_Port,KEY_F2_Pin)?0:1;
        case Key_Shift:return GPIO_ReadInputDataBit(KEY_Shift_Port,KEY_Shift_Pin)?0:1;
        case Key_Down:return GPIO_ReadInputDataBit(KEY_Down_Port,KEY_Down_Pin)?0:1;
        case Key_Enter:return GPIO_ReadInputDataBit(KEY_Enter_Port,KEY_Enter_Pin)?0:1;
    }
    return 0;
}

void init_buttons(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE );

    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    // F1
	GPIO_InitStructure.GPIO_Pin = KEY_F1_Pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(KEY_F1_Port, &GPIO_InitStructure );

	// left
	GPIO_InitStructure.GPIO_Pin = KEY_Left_Pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(KEY_Left_Port, &GPIO_InitStructure );

	// up
	GPIO_InitStructure.GPIO_Pin = KEY_Up_Pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(KEY_Up_Port, &GPIO_InitStructure );

	// right
	GPIO_InitStructure.GPIO_Pin = KEY_Right_Pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(KEY_Right_Port, &GPIO_InitStructure );

	// F2
	GPIO_InitStructure.GPIO_Pin = KEY_F2_Pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(KEY_F2_Port, &GPIO_InitStructure );

	// shift
	GPIO_InitStructure.GPIO_Pin = KEY_Shift_Pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(KEY_Shift_Port, &GPIO_InitStructure );

	// down
	GPIO_InitStructure.GPIO_Pin = KEY_Down_Pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(KEY_Down_Port, &GPIO_InitStructure );

	// enter
	GPIO_InitStructure.GPIO_Pin = KEY_Enter_Pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(KEY_Enter_Port, &GPIO_InitStructure );
}
