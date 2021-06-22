/**************************************************************
 *    MSP432P401R通过矩阵按键模拟密码锁，在SSD1306 OLED显示内容
 * 功能描述：初始状态，OLED显示欢迎界面，请输入密码

 *
 *引脚连接：
 *                MSP432P401R
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST               |
 *            |                  |
 *    OLED    |                  |     矩阵按键
 *            |            P2.7  |<--  C4
 *    CS <--- |P1.5        P2.6  |<--  C3
 *    DC <--- |P6.0        P2.5  |<--  C2
 *    RES<--- |P6.1        P2.4  |<--  C1
 *    SDA<--- |P3.2        P5.0  |<--  R1
 *    SCK<--- |P4.0        P5.1  |<--  R2
 *    VDD<--- |3.3v        P5.2  |<--  R3
 *    GND<--- |GND         P5.6  |<--  R4
 *
 *矩阵按键原理：
 *4列输出接低电平，4行接上拉电阻作为中断输入，
 *按键未按下时，行信号输入为高；按键按下时，行信号输入为低。
 *首先所有列输出低电平，4行接上拉电阻作为中断输入，某个按键按下，则会触发按键所在行的中断。
 *在中断中，第1列输出低电平，其他列输出高电平，读取输入信号，
 *若输入为高，则按键不在第1列；若输入为低，则按键在第一列，再加上是哪一行的中断输入，
 *就可判断是‘1’ ‘5’ ‘9’ ‘13’哪个按键按下。
 *第2列再输出低电平，判断是‘2’ ‘6’ ‘10’ ‘14’哪个按键按下。
 *第3列再输出低电平，判断是‘3’ ‘7’ ‘11’ ‘15’哪个按键按下。
 *第4列再输出低电平，判断是‘4’ ‘8’ ‘12’ ‘16’哪个按键按下。
 *
 *程序框架说明：
 *本工程采用4x4矩阵按键作为输入，SSD1306 OLED显示屏作为输出，利用 主程序死循环 + 中断输入 + OLED例程 编程。
 *main.c
 *主程序配置列为输出扫描信号，行为中断输入信号。
 *进入中断服务函数OledInterrupt置1，返回死循环OledInterrupt置0并清屏，根据MoudleKey的值选择对应显示内容。
 *oled.c
 *定义OLED有关的各种函数
 *oled.h
 *OLED用到的头文件和函数进行声明
 *oledfont.h
 *OLED的字符库和汉字库，汉字库需自己取模
 *bmp.h
 *OLED的图片库，图片库需自己取模
****************************************************************/
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <stdint.h>
#include <stdbool.h>
#include"oled.h"
#include"bmp.h"

#define ButOutHighC1   {MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN4);}    //第1列C1输出高电平
#define ButOutLowC1    {MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN4);}     //第1列C1输出低电平
#define ButOutHighC2   {MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN5);}    //第2列C2输出高电平
#define ButOutLowC2    {MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN5);}     //第2列C2输出低电平
#define ButOutHighC3   {MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN6);}    //第3列C3输出高电平
#define ButOutLowC3    {MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN6);}     //第3列C3输出低电平
#define ButOutHighC4   {MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN7);}    //第4列C4输出高电平
#define ButOutLowC4    {MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN7);}     //第4列C4输出低电平
int DebugCount =0;//debug用
int Mode2 =0;//debug用
int Mode = 0;                  //OLED显示模式选择参数，对应第几个OLED模式
int Count = 0;                 //输入密码数现在有几位
int Password[6] = {1,1,1,1,1,1};  //密码锁内置密码，现为初始密码111111
int State = 0;					//密码锁三种状态：0为正常输入密码解锁；1为尝试改密码过程中输入原密码；2为输入新密码
int Input[6] = {0,0,0,0,0,0};		//输入密码
int Key_Value = 100;
/*中断输入标志参数OledInterrupt。有按键按下则OLED需清屏重新显示，进入中断服务函数，OledInterrupt被置1；
 * 返回循环，OLED清屏，OledInterrupt被清0。避免OLED显示内容覆盖。*/
int OledInterrupt  =0;
int i = 0;                //循环验证密码时用到的index变量
int time ;
void LED2_BlingBlingRed()
{
	for(time =20; time>0; time--){
		GPIO_setAsOutputPin(GPIO_PORT_P2,GPIO_PIN0);
		GPIO_toggleOutputOnPin(GPIO_PORT_P2,GPIO_PIN0);
		for(i=200000; i>0; i--);
	}
}
void LED2_BlingBlingGreen()
{
	for(time =20; time>0; time--){
		GPIO_setAsOutputPin(GPIO_PORT_P2,GPIO_PIN1);
		GPIO_toggleOutputOnPin(GPIO_PORT_P2,GPIO_PIN1);
		for(i=200000; i>0; i--);
	}
}
void LED2_BlingBlingBlue()
{
	for(time =20; time>0; time--){
		GPIO_setAsOutputPin(GPIO_PORT_P2,GPIO_PIN2);
		GPIO_toggleOutputOnPin(GPIO_PORT_P2,GPIO_PIN2);
		for(i=200000; i>0; i--);
	}
}
//更改为高频时钟DCO 48MHz
void SystemClockInit()
{
 /* Halting the Watchdog */
	MAP_WDT_A_holdTimer();


      /* 配置外部高速时钟引脚 */
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_PJ,
            GPIO_PIN3 | GPIO_PIN2, GPIO_PRIMARY_MODULE_FUNCTION);

    /* Just in case the user wants to use the getACLK, getMCLK, etc. functions,
     * let's set the clock frequency in the code.
     */
    CS_setExternalClockSourceFrequency(32000,24000000);

    /* Starting HFXT in non-bypass mode without a timeout. Before we start
     * we have to change VCORE to 1 to support the 48MHz frequency */
    PCM_setCoreVoltageLevel(PCM_VCORE1);
    FlashCtl_setWaitState(FLASH_BANK0, 2);
    FlashCtl_setWaitState(FLASH_BANK1, 2);
    CS_startHFXT(false);//false

    /* Initializing MCLK to HFXT (effectively 48MHz) */
    MAP_CS_initClockSignal(CS_MCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_1);


}


int main(void)
{

    SystemClockInit();			 //更改为高频时钟HFXT 48MHz
    init();                      //OLED端口定义
    OLED_Init();                 //初始化SSD1306  OLED
    //int last = 0;
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN4);     //第1列C1
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN5);     //第2列C2
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN6);     //第3列C3
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN7);     //第4列C4

    //所有列输出low电平

    ButOutLowC1;
    ButOutLowC2;
    ButOutLowC3;
    ButOutLowC4;


    //配置第1行R1--P5.0为上拉电阻输入并使能中断
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5, GPIO_PIN0);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P5, GPIO_PIN0);
    MAP_GPIO_enableInterrupt(GPIO_PORT_P5, GPIO_PIN0);

    //配置第2行R2--P5.1为上拉电阻输入并使能中断
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5, GPIO_PIN1);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P5, GPIO_PIN1);
    MAP_GPIO_enableInterrupt(GPIO_PORT_P5, GPIO_PIN1);

    //配置第3行R3--P5.2为上拉电阻输入并使能中断
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5, GPIO_PIN2);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P5, GPIO_PIN2);
    MAP_GPIO_enableInterrupt(GPIO_PORT_P5, GPIO_PIN2);

    //配置第4行R4--P5.6为上拉电阻输入并使能中断
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5, GPIO_PIN6);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P5, GPIO_PIN6);
    MAP_GPIO_enableInterrupt(GPIO_PORT_P5, GPIO_PIN6);
    MAP_Interrupt_enableInterrupt(INT_PORT5);

    //Enabling SRAM Bank Retention
    MAP_SysCtl_enableSRAMBankRetention(SYSCTL_SRAM_BANK1);

    //使能全局中断
    MAP_Interrupt_enableMaster();

//    MAP_WDT_A_holdTimer();     //关闭看门狗

    //进入模式选择循环
    while (1)
    {
        if( OledInterrupt == 1)    //完成一次中断服务，OLED需清屏
        {
            OLED_Clear();
            OledInterrupt = 0;
        }
        //模式0, 显示初始化欢迎信息
		if(Mode == 0)
		{
			OLED_ShowCHinese(0,2,0);      //欢
			OLED_ShowCHinese(16,2,1);     //迎
			OLED_ShowCHinese(32,2,2);     //使
			OLED_ShowCHinese(48,2,3);     //用
			OLED_ShowCHinese(64,2,4);     //密
			OLED_ShowCHinese(80,2,5);     //码
			OLED_ShowCHinese(96,2,6);     //锁
			OLED_ShowCHinese(0,4,7);      //请
			OLED_ShowCHinese(16,4,8);     //输
			OLED_ShowCHinese(32,4,9);     //入
			OLED_ShowCHinese(48,4,4);     //密
			OLED_ShowCHinese(64,4,5);     //码
			OLED_ShowChar(80,4,':');
			State = 0;
		}
        //模式1*
		else if(Mode == 1)
        {
			OLED_ShowCHinese(0,2,7);      //请
			OLED_ShowCHinese(16,2,8);     //输
			OLED_ShowCHinese(32,2,9);     //入
			OLED_ShowCHinese(48,2,4);     //密
			OLED_ShowCHinese(64,2,5);     //码
			OLED_ShowChar(80,2,':');
			OLED_ShowChar(0,4,'*');
        }

        //模式2**
        else if(Mode == 2)
        {
			Mode2 = 1;
        	OLED_ShowCHinese(0,2,7);      //请
			OLED_ShowCHinese(16,2,8);     //输
			OLED_ShowCHinese(32,2,9);     //入
			OLED_ShowCHinese(48,2,4);     //密
			OLED_ShowCHinese(64,2,5);     //码
			OLED_ShowChar(80,2,':');
			OLED_ShowChar(0,4,'*');
			OLED_ShowChar(16,4,'*');
        }

        //模式3***
        else if(Mode == 3)
        {
			OLED_ShowCHinese(0,2,7);      //请
			OLED_ShowCHinese(16,2,8);     //输
			OLED_ShowCHinese(32,2,9);     //入
			OLED_ShowCHinese(48,2,4);     //密
			OLED_ShowCHinese(64,2,5);     //码
			OLED_ShowChar(80,2,':');
			OLED_ShowChar(0,4,'*');
			OLED_ShowChar(16,4,'*');
			OLED_ShowChar(32,4,'*');
        }

        //模式4****
        else if(Mode == 4)
        {
			OLED_ShowCHinese(0,2,7);      //请
			OLED_ShowCHinese(16,2,8);     //输
			OLED_ShowCHinese(32,2,9);     //入
			OLED_ShowCHinese(48,2,4);     //密
			OLED_ShowCHinese(64,2,5);     //码
			OLED_ShowChar(80,2,':');
			OLED_ShowChar(0,4,'*');
			OLED_ShowChar(16,4,'*');
			OLED_ShowChar(32,4,'*');
			OLED_ShowChar(48,4,'*');
        }

        //模式5*****
        else if(Mode == 5)
        {
			OLED_ShowCHinese(0,2,7);      //请
			OLED_ShowCHinese(16,2,8);     //输
			OLED_ShowCHinese(32,2,9);     //入
			OLED_ShowCHinese(48,2,4);     //密
			OLED_ShowCHinese(64,2,5);     //码
			OLED_ShowChar(80,2,':');
			OLED_ShowChar(0,4,'*');
			OLED_ShowChar(16,4,'*');
			OLED_ShowChar(32,4,'*');
			OLED_ShowChar(48,4,'*');
			OLED_ShowChar(64,4,'*');
        }

        //模式6******，错误，1s后转回Mode0
        else if(Mode == 6)
        {
			//last = 1;
        	OLED_ShowCHinese(0,2,7);      //请
			OLED_ShowCHinese(16,2,8);     //输
			OLED_ShowCHinese(32,2,9);     //入
			OLED_ShowCHinese(48,2,4);     //密
			OLED_ShowCHinese(64,2,5);     //码
			OLED_ShowChar(80,2,':');
			OLED_ShowChar(0,4,'*');
			OLED_ShowChar(16,4,'*');
			OLED_ShowChar(32,4,'*');
			OLED_ShowChar(48,4,'*');
			OLED_ShowChar(64,4,'*');
			OLED_ShowChar(80,4,'*');
			//delay_ms(500);
			//显示密码错误
			OLED_Clear();
			OledInterrupt = 0;
			OLED_ShowCHinese(0,2,4);      //密
			OLED_ShowCHinese(16,2,5);     //码
			OLED_ShowCHinese(32,2,14);    //错
			OLED_ShowCHinese(48,2,15);    //误
			OLED_ShowChar(64,2,'!');
			OLED_ShowChar(80,2,'!');
			OLED_ShowChar(96,2,'!');
			LED2_BlingBlingRed();
			OLED_ShowCHinese(0,4,7);      //请
			OLED_ShowCHinese(16,4,12);    //重
			OLED_ShowCHinese(32,4,10);    //新
			OLED_ShowCHinese(48,4,8);     //输
			OLED_ShowCHinese(64,4,9);     //入
			OLED_ShowCHinese(80,4,4);     //密
			OLED_ShowCHinese(96,4,5);     //码
			delay_ms(500);
			P2OUT &= ~BIT0;
			OLED_Clear();
			OledInterrupt = 0;
			Mode = 0;
        }

        //模式7******，正确，1s后转回Mode0
        else if(Mode == 7)
        {
			OLED_ShowCHinese(0,2,7);      //请
			OLED_ShowCHinese(16,2,8);     //输
			OLED_ShowCHinese(32,2,9);     //入
			OLED_ShowCHinese(48,2,4);     //密
			OLED_ShowCHinese(64,2,5);     //码
			OLED_ShowChar(80,2,':');
			OLED_ShowChar(0,4,'*');
			OLED_ShowChar(16,4,'*');
			OLED_ShowChar(32,4,'*');
			OLED_ShowChar(48,4,'*');
			OLED_ShowChar(64,4,'*');
			OLED_ShowChar(80,4,'*');
			//显示密码正确
			OLED_Clear();
			OledInterrupt = 0;
			OLED_ShowCHinese(0,2,4);      //密
			OLED_ShowCHinese(16,2,5);     //码
			OLED_ShowCHinese(32,2,16);    //正
			OLED_ShowCHinese(48,2,17);    //确
			OLED_ShowChar(64,2,'!');
			OLED_ShowChar(80,2,'!');
			OLED_ShowChar(96,2,'!');
			LED2_BlingBlingGreen();
			OLED_ShowCHinese(0,4,18);      //解
			OLED_ShowCHinese(16,4,6);      //锁
			OLED_ShowCHinese(32,4,23);     //成
			OLED_ShowCHinese(48,4,24);     //功
			OLED_ShowChar(64,4,'!');
			OLED_ShowChar(80,4,'!');
			OLED_ShowChar(96,4,'!');   /*后面有空可以加上LED0和LED1闪烁什么的*/
			delay_ms(500);
			P2OUT &= ~BIT1;
			OLED_Clear();
			OledInterrupt = 0;
			Mode = 0;
        }

        //模式8, 重设密码，请输入原密码:
        else if(Mode == 8)
        {
        	State = 1;
        	OLED_ShowCHinese(0,2,12);	  //重
        	OLED_ShowCHinese(16,2,25);	  //置
        	OLED_ShowCHinese(32,2,4);	  //密
        	OLED_ShowCHinese(48,2,5);     //码
        	OLED_ShowChar(64,2,',');
        	OLED_ShowCHinese(0,4,7);      //请
			OLED_ShowCHinese(16,4,8);     //输
			OLED_ShowCHinese(32,4,9);     //入
			OLED_ShowCHinese(48,4,11);    //原
			OLED_ShowCHinese(64,4,4);	  //密
        	OLED_ShowCHinese(80,4,5);     //码
        	OLED_ShowChar(96,4,':');
        }

        //模式9, 重设密码，输入原密码*
        else if(Mode == 9)
        {
        	OLED_ShowCHinese(0,2,7);      //请
			OLED_ShowCHinese(16,2,8);     //输
			OLED_ShowCHinese(32,2,9);     //入
			OLED_ShowCHinese(48,2,11);    //原
			OLED_ShowCHinese(64,2,4);	  //密
        	OLED_ShowCHinese(80,2,5);     //码
        	OLED_ShowChar(96,2,':');
        	OLED_ShowChar(0,4,'*');
        }

        //模式10, 重设密码，输入原密码**
        else if(Mode == 10)
        {
        	OLED_ShowCHinese(0,2,7);      //请
			OLED_ShowCHinese(16,2,8);     //输
			OLED_ShowCHinese(32,2,9);     //入
			OLED_ShowCHinese(48,2,11);    //原
			OLED_ShowCHinese(64,2,4);	  //密
        	OLED_ShowCHinese(80,2,5);     //码
        	OLED_ShowChar(96,2,':');
        	OLED_ShowChar(0,4,'*');
        	OLED_ShowChar(16,4,'*');
        }

        //模式11, 重设密码，输入原密码***
        else if(Mode == 11)
        {
        	OLED_ShowCHinese(0,2,7);      //请
			OLED_ShowCHinese(16,2,8);     //输
			OLED_ShowCHinese(32,2,9);     //入
			OLED_ShowCHinese(48,2,11);    //原
			OLED_ShowCHinese(64,2,4);	  //密
        	OLED_ShowCHinese(80,2,5);     //码
        	OLED_ShowChar(96,2,':');
        	OLED_ShowChar(0,4,'*');
        	OLED_ShowChar(16,4,'*');
        	OLED_ShowChar(32,4,'*');
        }

        //模式12, 重设密码，输入原密码****
        else if(Mode == 12)
        {
        	OLED_ShowCHinese(0,2,7);      //请
			OLED_ShowCHinese(16,2,8);     //输
			OLED_ShowCHinese(32,2,9);     //入
			OLED_ShowCHinese(48,2,11);    //原
			OLED_ShowCHinese(64,2,4);	  //密
        	OLED_ShowCHinese(80,2,5);     //码
        	OLED_ShowChar(96,2,':');
        	OLED_ShowChar(0,4,'*');
        	OLED_ShowChar(16,4,'*');
        	OLED_ShowChar(32,4,'*');
        	OLED_ShowChar(48,4,'*');
        }

        //模式13, 重设密码,输入原密码*****
        else if(Mode == 13)
        {
        	OLED_ShowCHinese(0,2,7);      //请
			OLED_ShowCHinese(16,2,8);     //输
			OLED_ShowCHinese(32,2,9);     //入
			OLED_ShowCHinese(48,2,11);    //原
			OLED_ShowCHinese(64,2,4);	  //密
        	OLED_ShowCHinese(80,2,5);     //码
        	OLED_ShowChar(96,2,':');
        	OLED_ShowChar(0,4,'*');
        	OLED_ShowChar(16,4,'*');
        	OLED_ShowChar(32,4,'*');
        	OLED_ShowChar(48,4,'*');
        	OLED_ShowChar(64,4,'*');
        }

        //模式14, 重设密码，输入原密码******,原密码错误，更改失败，回到Mode0
        else if(Mode == 14)
        {
        	OLED_ShowCHinese(0,2,7);      //请
			OLED_ShowCHinese(16,2,8);     //输
			OLED_ShowCHinese(32,2,9);     //入
			OLED_ShowCHinese(48,2,11);    //原
			OLED_ShowCHinese(64,2,4);	  //密
        	OLED_ShowCHinese(80,2,5);     //码
        	OLED_ShowChar(96,2,':');
        	OLED_ShowChar(0,4,'*');
        	OLED_ShowChar(16,4,'*');
        	OLED_ShowChar(32,4,'*');
        	OLED_ShowChar(48,4,'*');
        	OLED_ShowChar(64,4,'*');
        	OLED_ShowChar(80,4,'*');
			//显示验证原密码错误
			OLED_Clear();

			OledInterrupt = 0;
			LED2_BlingBlingRed();
			OLED_ShowCHinese(0,2,11);     //原
			OLED_ShowCHinese(16,2,4);     //密
			OLED_ShowCHinese(32,2,5);     //码
			OLED_ShowCHinese(48,2,14);    //错
			OLED_ShowCHinese(64,2,15);    //误
			OLED_ShowChar(80,2,'!');
			OLED_ShowChar(96,2,'!');
			OLED_ShowChar(112,2,'!');/*后面有空可以加上LED0和LED1闪烁什么的*/

			delay_ms(1000);
			P2OUT &= ~BIT0;
			OLED_Clear();
			OledInterrupt = 0;
			Mode = 0;
        }
        //模式15, 重设密码，输入原密码******,原密码正确，进入输入新密码状态，Mode16
        else if(Mode == 15)
        {
        	OLED_ShowCHinese(0,2,7);      //请
			OLED_ShowCHinese(16,2,8);     //输
			OLED_ShowCHinese(32,2,9);     //入
			OLED_ShowCHinese(48,2,11);    //原
			OLED_ShowCHinese(64,2,4);	  //密
        	OLED_ShowCHinese(80,2,5);     //码
        	OLED_ShowChar(96,2,':');
        	OLED_ShowChar(0,4,'*');
        	OLED_ShowChar(16,4,'*');
        	OLED_ShowChar(32,4,'*');
        	OLED_ShowChar(48,4,'*');
        	OLED_ShowChar(64,4,'*');
        	OLED_ShowChar(80,4,'*');
			//显示验证原密码正确，开始输入新密码
			OLED_Clear();
			OledInterrupt = 0;
			LED2_BlingBlingGreen();
			OLED_ShowCHinese(0,2,11);     //原
			OLED_ShowCHinese(16,2,4);     //密
			OLED_ShowCHinese(32,2,5);     //码
			OLED_ShowCHinese(48,2,16);    //正
			OLED_ShowCHinese(64,2,17);    //确
			OLED_ShowChar(80,2,'!');
			OLED_ShowChar(96,2,'!');
			OLED_ShowChar(112,2,'!');
			/*后面有空可以加上LED0和LED1闪烁什么的*/
			OLED_ShowCHinese(0,4,7);      //请
			OLED_ShowCHinese(16,4,8);     //输
			OLED_ShowCHinese(32,4,9);     //入
			OLED_ShowCHinese(48,4,10);    //新
			OLED_ShowCHinese(64,4,4);	  //密
        	OLED_ShowCHinese(80,4,5);     //码
        	OLED_ShowChar(96,4,':');
			delay_ms(500);
			P2OUT &= ~BIT1;
			OLED_Clear();
			OledInterrupt = 0;
			Mode = 16;
			State = 2;
        }
		//模式16, 重设密码,输入新密码(空)
		else if(Mode == 16)
		{
			OLED_ShowCHinese(0,2,7);      //请
			OLED_ShowCHinese(16,2,8);     //输
			OLED_ShowCHinese(32,2,9);     //入
			OLED_ShowCHinese(48,2,10);    //新
			OLED_ShowCHinese(64,2,4);	  //密
			OLED_ShowCHinese(80,2,5);     //码
			OLED_ShowChar(96,2,':');
		}
        //模式17, 重设密码,输入新密码*
        else if(Mode == 17)
        {
        	OLED_ShowCHinese(0,2,7);      //请
			OLED_ShowCHinese(16,2,8);     //输
			OLED_ShowCHinese(32,2,9);     //入
			OLED_ShowCHinese(48,2,10);    //新
			OLED_ShowCHinese(64,2,4);	  //密
        	OLED_ShowCHinese(80,2,5);     //码
        	OLED_ShowChar(96,2,':');
        	OLED_ShowChar(0,4,'*');
        }
        //模式18, 重设密码,输入新密码**
        else if(Mode == 18)
        {
        	OLED_ShowCHinese(0,2,7);      //请
			OLED_ShowCHinese(16,2,8);     //输
			OLED_ShowCHinese(32,2,9);     //入
			OLED_ShowCHinese(48,2,10);    //新
			OLED_ShowCHinese(64,2,4);	  //密
        	OLED_ShowCHinese(80,2,5);     //码
        	OLED_ShowChar(96,2,':');
        	OLED_ShowChar(0,4,'*');
        	OLED_ShowChar(16,4,'*');
        }
        //模式19, 重设密码,输入新密码***
        else if(Mode == 19)
        {
        	OLED_ShowCHinese(0,2,7);      //请
			OLED_ShowCHinese(16,2,8);     //输
			OLED_ShowCHinese(32,2,9);     //入
			OLED_ShowCHinese(48,2,10);    //新
			OLED_ShowCHinese(64,2,4);	  //密
        	OLED_ShowCHinese(80,2,5);     //码
        	OLED_ShowChar(96,2,':');
        	OLED_ShowChar(0,4,'*');
        	OLED_ShowChar(16,4,'*');
        	OLED_ShowChar(32,4,'*');
        }
        //模式20, 重设密码,输入新密码****
        else if(Mode == 20)
        {
        	OLED_ShowCHinese(0,2,7);      //请
			OLED_ShowCHinese(16,2,8);     //输
			OLED_ShowCHinese(32,2,9);     //入
			OLED_ShowCHinese(48,2,10);    //新
			OLED_ShowCHinese(64,2,4);	  //密
        	OLED_ShowCHinese(80,2,5);     //码
        	OLED_ShowChar(96,2,':');
        	OLED_ShowChar(0,4,'*');
        	OLED_ShowChar(16,4,'*');
        	OLED_ShowChar(32,4,'*');
        	OLED_ShowChar(48,4,'*');
        }
        //模式21, 重设密码,输入新密码*****
        else if(Mode == 21)
        {
        	OLED_ShowCHinese(0,2,7);      //请
			OLED_ShowCHinese(16,2,8);     //输
			OLED_ShowCHinese(32,2,9);     //入
			OLED_ShowCHinese(48,2,10);    //新
			OLED_ShowCHinese(64,2,4);	  //密
        	OLED_ShowCHinese(80,2,5);     //码
        	OLED_ShowChar(96,2,':');
        	OLED_ShowChar(0,4,'*');
        	OLED_ShowChar(16,4,'*');
        	OLED_ShowChar(32,4,'*');
        	OLED_ShowChar(48,4,'*');
        	OLED_ShowChar(64,4,'*');
        }
        //模式22, 重设密码,输入新密码*****
        else if(Mode == 22)
        {
        	OLED_ShowCHinese(0,2,7);      //请
			OLED_ShowCHinese(16,2,8);     //输
			OLED_ShowCHinese(32,2,9);     //入
			OLED_ShowCHinese(48,2,10);    //新
			OLED_ShowCHinese(64,2,4);	  //密
        	OLED_ShowCHinese(80,2,5);     //码
        	OLED_ShowChar(96,2,':');
        	OLED_ShowChar(0,4,'*');
        	OLED_ShowChar(16,4,'*');
        	OLED_ShowChar(32,4,'*');
        	OLED_ShowChar(48,4,'*');
        	OLED_ShowChar(64,4,'*');
        	OLED_ShowChar(80,4,'*');
        	delay_ms(50);
        	/*后面有空可以加上LED0和LED1闪烁什么的*/
			OLED_Clear();
			OledInterrupt = 0;
			LED2_BlingBlingBlue();
			//显示 新密码设置成功，转回Mode0
			OLED_ShowCHinese(0,2,10);     //新
			OLED_ShowCHinese(16,2,4);	  //密
        	OLED_ShowCHinese(32,2,5);     //码
        	OLED_ShowCHinese(48,2,13);    //设
        	OLED_ShowCHinese(64,2,25);    //置
        	OLED_ShowCHinese(80,2,23);    //成
        	OLED_ShowCHinese(96,2,24);    //功
        	OLED_ShowChar(112,2,'!');
        	delay_ms(500);
        	P2OUT &= ~BIT2;
        	OLED_Clear();
        	OledInterrupt = 0;
        	Mode = 0;
        }
        else{

        }
    }
}


/* GPIO中断服务程序  */
/* 在中断中进行列扫描判断哪个按键按下，中断结束将所有列置为0. */

void PORT5_IRQHandler(void)
{
    int Wrong = 0;                                                      //确认验证密码时用到的Flag变量
	uint32_t Interupt_State;                              				    //设置一个状态标志量，寄存中断情况
	//DebugCount++;

    //先判读是哪个按键按下。S1-S10分别对应密码锁1-9和0,Key_Value=1-9和0
    //S11为进入验证重设密码状态,Key_Value=11;S12为删除一位密码Key_Value=12;其他为无效按键
    //uint32_t Key_Value = 10;	//初始化为一个无效值，因为S10代表0按键的Key_Value是0
    /**************************************************************/
    /**************************************************************/
    /**************************************************************/
    //先由按键给出Key_Value值
    //判断第二列按键是否按下,按下输入为0
    //delay_ms(20);
	ButOutHighC1;
	ButOutLowC2;
	ButOutHighC3;
	ButOutHighC4;

	if(!(MAP_GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN0)))              //S2按下
	{
		//delay_ms(100);
		delay_ms(20);
		Key_Value=2;
		//delay_ms(20);
	}
	else if(!(MAP_GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN1))  )       //S6按下
	{

		//delay_ms(100);
		delay_ms(20);
		Key_Value=6;

	}
	else if(!(MAP_GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN2))  )        //S10按下
	{
		delay_ms(20);
		Key_Value=0;
	}
	else if(!(MAP_GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN6))  )        //S14按下
	{
		delay_ms(20);
		Key_Value=14;
	}
	//delay_ms(20);
    //判断第一列按键是否按下,按下输入为0
    ButOutLowC1;
    ButOutHighC2;
    ButOutHighC3;
    ButOutHighC4;

    if(!(MAP_GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN0)))              //S1按下
    {
    	delay_ms(20);
    	Key_Value=1;
    	DebugCount++;
    }
    else if(!(MAP_GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN1))  )       //S5按下
    {
    	delay_ms(20);
    	Key_Value=5;
    }
    else if(!(MAP_GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN2))  )        //S9按下
    {
    	delay_ms(20);
    	Key_Value=9;
    }
    else if(!(MAP_GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN6))  )        //S13按下
    {
    	delay_ms(20);
    	Key_Value=13;
    }

    //delay_ms(20);
    //判断第三列按键是否按下,按下输入为0
    ButOutHighC1;
    ButOutHighC2;
    ButOutLowC3;
    ButOutHighC4;

    if(!(MAP_GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN0)))              //S3按下
    {
    	delay_ms(20);
    	Key_Value=3;
    }
    else if(!(MAP_GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN1))  )       //S7按下
    {
    	delay_ms(20);
    	Key_Value=7;

    }
    else if(!(MAP_GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN2)) )        //S11按下
    {
    	delay_ms(20);
    	Key_Value=11;
    }
    else if(!(MAP_GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN6)) )        //S15按下
    {
    	delay_ms(20);
    	Key_Value=15;
    }
    //delay_ms(20);
    //判断第四列按键是否按下,按下输入为0
    ButOutHighC1;
    ButOutHighC2;
    ButOutHighC3;
    ButOutLowC4;

    if(!(MAP_GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN0)))              //S4按下
    {
    	delay_ms(20);
    	Key_Value=4;
    }
    else if(!(MAP_GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN1)) )        //S8按下
    {
    	delay_ms(20);
        Key_Value=8;
    }
    else if(!(MAP_GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN2)) )        //S12按下
    {
        delay_ms(20);
        Key_Value=12;
    }
    else if(!(MAP_GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN6)) )        //S16按下
    {
    	delay_ms(20);
    	Key_Value=16;
    }

    /**************************************************************/
    /**************************************************************/
    /**************************************************************/
    /**************************************************************/
    //接下来根据Key_Value值来判定Mode和Count等GlobalVarient值

    if(Key_Value == 11)      //尝试重新设置密码，想进入State 1
    {
    	if(Mode == 0){
    		Mode = 8;
    		State = 1;
    	}
    }
    else if(Key_Value == 12) //删除一位密码功能
    {
    	if(Count > 0){
    		Count--;
    		Mode --;
    	}
    }
    else if(Key_Value < 10)  //输入密码为0-9
    {

    	Count++;
    	Input[Count - 1] = Key_Value;
    	//当密码锁状态为正常解锁，或者尝试改密码过程中的正常解锁时
    	if(State == 0 || State == 1)
    	{
    		if(Count == 6){
    			Count = 0;
    			//验证Passport和Input是否相同
    			for(i=0;i<6;i++){
    				if(Password[i] != Input[i]){
    					Wrong = 1;
    				}
    			}
    			//若密码错误
    			if(Wrong == 1){
    				Mode = 8 * State + 6;
    			}
    			else{
    				Mode = 8 * State + 7;
    			}
    		}
    		else if(Count < 6){

    			Mode = 8 * State + Count;
    		}
    	}
    	//当密码锁状态为输入新密码时
    	else{
			Password[Count-1] = Input[Count-1];
    		if(Count == 6){
    			Count = 0;
    			Mode = 22;
    		}
    		else{
    			Mode = 16 + Count;
    		}
    	}

    }

	//delay_ms(50);
    //所有列置0，方便下一次进入中断
    ButOutLowC1;
    ButOutLowC2;
    ButOutLowC3;
    ButOutLowC4;
    //delay_ms(100);
    OledInterrupt = 1;                               						//进入中断服务程序，OledInterrupt置1
	Interupt_State = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P5);
	MAP_GPIO_clearInterruptFlag(GPIO_PORT_P5, Interupt_State);              //清零所有中断标志位
}
