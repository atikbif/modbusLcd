#include "lcd.h"
#include "stm32f10x_conf.h"

#define SET_CMD     1
#define SET_DATA    0

static void init_lcd(void);
static void write_data(char dataType, unsigned char data);
static void write_short_data(unsigned char data);
static void clear_buf(void);

#define LCD_BUF_SIZE    80

unsigned char lcd_buf[LCD_BUF_SIZE];

portTickType lcd_xLastExecutionTime;

#define LCD_DB4_PIN     GPIO_Pin_1
#define LCD_DB5_PIN     GPIO_Pin_2
#define LCD_DB6_PIN     GPIO_Pin_10
#define LCD_DB7_PIN     GPIO_Pin_11
#define LCD_E_PIN       GPIO_Pin_0
#define LCD_RS_PIN      GPIO_Pin_7

#define LCD_DB4_PORT    GPIOB
#define LCD_DB5_PORT    GPIOB
#define LCD_DB6_PORT    GPIOB
#define LCD_DB7_PORT    GPIOB
#define LCD_E_PORT      GPIOB
#define LCD_RS_PORT     GPIOA



void write_short_data(unsigned char data)
{
    GPIO_ResetBits(LCD_RS_PORT,LCD_RS_PIN);
    if(data & 0x80) GPIO_SetBits(LCD_DB7_PORT,LCD_DB7_PIN);else GPIO_ResetBits(LCD_DB7_PORT,LCD_DB7_PIN);
    if(data & 0x40) GPIO_SetBits(LCD_DB6_PORT,LCD_DB6_PIN);else GPIO_ResetBits(LCD_DB6_PORT,LCD_DB6_PIN);
    if(data & 0x20) GPIO_SetBits(LCD_DB5_PORT,LCD_DB5_PIN);else GPIO_ResetBits(LCD_DB5_PORT,LCD_DB5_PIN);
    if(data & 0x10) GPIO_SetBits(LCD_DB4_PORT,LCD_DB4_PIN);else GPIO_ResetBits(LCD_DB4_PORT,LCD_DB4_PIN);
    GPIO_SetBits(LCD_E_PORT,LCD_E_PIN);
    GPIO_ResetBits(LCD_E_PORT,LCD_E_PIN);

    //lcd_xLastExecutionTime = xTaskGetTickCount();
	vTaskDelayUntil( &lcd_xLastExecutionTime, ( ( portTickType ) 1 / portTICK_RATE_MS ) );

    GPIO_SetBits(LCD_DB7_PORT,LCD_DB7_PIN);
    GPIO_SetBits(LCD_DB6_PORT,LCD_DB6_PIN);
    GPIO_SetBits(LCD_DB5_PORT,LCD_DB5_PIN);
    GPIO_SetBits(LCD_DB4_PORT,LCD_DB4_PIN);
}

void write_data(char dataType, unsigned char data)
{
    if(dataType == SET_CMD)
    {
        GPIO_ResetBits(LCD_RS_PORT,LCD_RS_PIN);
    }
    else
    {
        GPIO_SetBits(LCD_RS_PORT,LCD_RS_PIN);
    }
    if(data & 0x80) GPIO_SetBits(LCD_DB7_PORT,LCD_DB7_PIN);else GPIO_ResetBits(LCD_DB7_PORT,LCD_DB7_PIN);
    if(data & 0x40) GPIO_SetBits(LCD_DB6_PORT,LCD_DB6_PIN);else GPIO_ResetBits(LCD_DB6_PORT,LCD_DB6_PIN);
    if(data & 0x20) GPIO_SetBits(LCD_DB5_PORT,LCD_DB5_PIN);else GPIO_ResetBits(LCD_DB5_PORT,LCD_DB5_PIN);
    if(data & 0x10) GPIO_SetBits(LCD_DB4_PORT,LCD_DB4_PIN);else GPIO_ResetBits(LCD_DB4_PORT,LCD_DB4_PIN);
    GPIO_SetBits(LCD_E_PORT,LCD_E_PIN);
    GPIO_ResetBits(LCD_E_PORT,LCD_E_PIN);

    //xLastExecutionTime = xTaskGetTickCount();
	//vTaskDelayUntil( &xLastExecutionTime, ( ( portTickType ) 1 / portTICK_RATE_MS ) );

    if(data & 0x08) GPIO_SetBits(LCD_DB7_PORT,LCD_DB7_PIN);else GPIO_ResetBits(LCD_DB7_PORT,LCD_DB7_PIN);
    if(data & 0x04) GPIO_SetBits(LCD_DB6_PORT,LCD_DB6_PIN);else GPIO_ResetBits(LCD_DB6_PORT,LCD_DB6_PIN);
    if(data & 0x02) GPIO_SetBits(LCD_DB5_PORT,LCD_DB5_PIN);else GPIO_ResetBits(LCD_DB5_PORT,LCD_DB5_PIN);
    if(data & 0x01) GPIO_SetBits(LCD_DB4_PORT,LCD_DB4_PIN);else GPIO_ResetBits(LCD_DB4_PORT,LCD_DB4_PIN);
    GPIO_SetBits(LCD_E_PORT,LCD_E_PIN);
    GPIO_ResetBits(LCD_E_PORT,LCD_E_PIN);

    //xLastExecutionTime = xTaskGetTickCount();
	vTaskDelayUntil( &lcd_xLastExecutionTime, ( ( portTickType ) 1 / portTICK_RATE_MS ) );

    GPIO_SetBits(LCD_DB7_PORT,LCD_DB7_PIN);
    GPIO_SetBits(LCD_DB6_PORT,LCD_DB6_PIN);
    GPIO_SetBits(LCD_DB5_PORT,LCD_DB5_PIN);
    GPIO_SetBits(LCD_DB4_PORT,LCD_DB4_PIN);
}

void init_lcd()
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE );

    // DB4 pin init
	GPIO_InitStructure.GPIO_Pin = LCD_DB4_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LCD_DB4_PORT, &GPIO_InitStructure );
	GPIO_SetBits(LCD_DB4_PORT,LCD_DB4_PIN);

	// DB5 pin init
	GPIO_InitStructure.GPIO_Pin = LCD_DB5_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LCD_DB5_PORT, &GPIO_InitStructure );
	GPIO_SetBits(LCD_DB5_PORT,LCD_DB5_PIN);

	// DB6 pin init
	GPIO_InitStructure.GPIO_Pin = LCD_DB6_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LCD_DB6_PORT, &GPIO_InitStructure );
	GPIO_SetBits(LCD_DB6_PORT,LCD_DB6_PIN);

	// DB7 pin init
	GPIO_InitStructure.GPIO_Pin = LCD_DB7_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LCD_DB7_PORT, &GPIO_InitStructure );
	GPIO_SetBits(LCD_DB7_PORT,LCD_DB7_PIN);

	// E pin init
	GPIO_InitStructure.GPIO_Pin = LCD_E_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LCD_E_PORT, &GPIO_InitStructure );
	GPIO_ResetBits(LCD_E_PORT,LCD_E_PIN);

	// RS pin init
	GPIO_InitStructure.GPIO_Pin = LCD_RS_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LCD_RS_PORT, &GPIO_InitStructure );
	GPIO_ResetBits(LCD_RS_PORT,LCD_RS_PIN);

	//xLastExecutionTime = xTaskGetTickCount();
	vTaskDelayUntil( &lcd_xLastExecutionTime, ( ( portTickType ) 20 / portTICK_RATE_MS ) );

	write_short_data(0x30);
	vTaskDelayUntil( &lcd_xLastExecutionTime, ( ( portTickType ) 10 / portTICK_RATE_MS ) );

	write_short_data(0x30);
	vTaskDelayUntil( &lcd_xLastExecutionTime, ( ( portTickType ) 5 / portTICK_RATE_MS ) );

	write_short_data(0x30);
	vTaskDelayUntil( &lcd_xLastExecutionTime, ( ( portTickType ) 1 / portTICK_RATE_MS ) );

	write_short_data(0x20);
	vTaskDelayUntil( &lcd_xLastExecutionTime, ( ( portTickType ) 1 / portTICK_RATE_MS ) );

	write_data(SET_CMD,0x2C);
	vTaskDelayUntil( &lcd_xLastExecutionTime, ( ( portTickType ) 1 / portTICK_RATE_MS ) );

	write_data(SET_CMD,0x06);
	vTaskDelayUntil( &lcd_xLastExecutionTime, ( ( portTickType ) 1 / portTICK_RATE_MS ) );

	write_data(SET_CMD,0x0C);
	vTaskDelayUntil( &lcd_xLastExecutionTime, ( ( portTickType ) 1 / portTICK_RATE_MS ) );

	write_data(SET_CMD,0x02);
	vTaskDelayUntil( &lcd_xLastExecutionTime, ( ( portTickType ) 3 / portTICK_RATE_MS ) );

	write_data(SET_CMD,0x01);
	vTaskDelayUntil( &lcd_xLastExecutionTime, ( ( portTickType ) 3 / portTICK_RATE_MS ) );
}

void LcdTask( void *pvParameters )
{
    unsigned char tst = 0x30;
    unsigned int tmp = 0;
    init_lcd();

    unsigned char updateCounter = 0;

    lcd_xLastExecutionTime = xTaskGetTickCount();
    vTaskDelayUntil( &lcd_xLastExecutionTime, ( ( portTickType ) 100 / portTICK_RATE_MS ) );
    clear_buf();
    for( ;; )
	{
	    updateCounter++;
	    if(updateCounter>=10)
        {
            // update display

            //setSymbol(0,tst++);
            //if(tst>0x39) tst=0x30;
            /*tmp = getFilteredKeys();
            setSymbol(0,tmp/100 + '0');
            setSymbol(1,(tmp%100)/10 + '0');
            setSymbol(2,(tmp%100)%10 + '0');*/

            for(tmp=0;tmp<LCD_BUF_SIZE;tmp++) write_data(SET_DATA, lcd_buf[tmp]);
            vTaskDelayUntil( &lcd_xLastExecutionTime, LCD_DELAY );
        }

    }
}

void setSymbol(unsigned char num, unsigned char value)
{
    if(num<LCD_BUF_SIZE) lcd_buf[num] = value;
}

void clear_buf(void)
{
    unsigned char tmp;
    for(tmp=0;tmp<LCD_BUF_SIZE;tmp++) lcd_buf[tmp] = 0x20;
}
