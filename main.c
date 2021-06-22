/**************************************************************
 *    MSP432P401Rͨ�����󰴼�ģ������������SSD1306 OLED��ʾ����
 * ������������ʼ״̬��OLED��ʾ��ӭ���棬����������

 *
 *�������ӣ�
 *                MSP432P401R
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST               |
 *            |                  |
 *    OLED    |                  |     ���󰴼�
 *            |            P2.7  |<--  C4
 *    CS <--- |P1.5        P2.6  |<--  C3
 *    DC <--- |P6.0        P2.5  |<--  C2
 *    RES<--- |P6.1        P2.4  |<--  C1
 *    SDA<--- |P3.2        P5.0  |<--  R1
 *    SCK<--- |P4.0        P5.1  |<--  R2
 *    VDD<--- |3.3v        P5.2  |<--  R3
 *    GND<--- |GND         P5.6  |<--  R4
 *
 *���󰴼�ԭ��
 *4������ӵ͵�ƽ��4�н�����������Ϊ�ж����룬
 *����δ����ʱ�����ź�����Ϊ�ߣ���������ʱ�����ź�����Ϊ�͡�
 *��������������͵�ƽ��4�н�����������Ϊ�ж����룬ĳ���������£���ᴥ�����������е��жϡ�
 *���ж��У���1������͵�ƽ������������ߵ�ƽ����ȡ�����źţ�
 *������Ϊ�ߣ��򰴼����ڵ�1�У�������Ϊ�ͣ��򰴼��ڵ�һ�У��ټ�������һ�е��ж����룬
 *�Ϳ��ж��ǡ�1�� ��5�� ��9�� ��13���ĸ��������¡�
 *��2��������͵�ƽ���ж��ǡ�2�� ��6�� ��10�� ��14���ĸ��������¡�
 *��3��������͵�ƽ���ж��ǡ�3�� ��7�� ��11�� ��15���ĸ��������¡�
 *��4��������͵�ƽ���ж��ǡ�4�� ��8�� ��12�� ��16���ĸ��������¡�
 *
 *������˵����
 *�����̲���4x4���󰴼���Ϊ���룬SSD1306 OLED��ʾ����Ϊ��������� ��������ѭ�� + �ж����� + OLED���� ��̡�
 *main.c
 *������������Ϊ���ɨ���źţ���Ϊ�ж������źš�
 *�����жϷ�����OledInterrupt��1��������ѭ��OledInterrupt��0������������MoudleKey��ֵѡ���Ӧ��ʾ���ݡ�
 *oled.c
 *����OLED�йصĸ��ֺ���
 *oled.h
 *OLED�õ���ͷ�ļ��ͺ�����������
 *oledfont.h
 *OLED���ַ���ͺ��ֿ⣬���ֿ����Լ�ȡģ
 *bmp.h
 *OLED��ͼƬ�⣬ͼƬ�����Լ�ȡģ
****************************************************************/
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <stdint.h>
#include <stdbool.h>
#include"oled.h"
#include"bmp.h"

#define ButOutHighC1   {MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN4);}    //��1��C1����ߵ�ƽ
#define ButOutLowC1    {MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN4);}     //��1��C1����͵�ƽ
#define ButOutHighC2   {MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN5);}    //��2��C2����ߵ�ƽ
#define ButOutLowC2    {MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN5);}     //��2��C2����͵�ƽ
#define ButOutHighC3   {MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN6);}    //��3��C3����ߵ�ƽ
#define ButOutLowC3    {MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN6);}     //��3��C3����͵�ƽ
#define ButOutHighC4   {MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN7);}    //��4��C4����ߵ�ƽ
#define ButOutLowC4    {MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN7);}     //��4��C4����͵�ƽ
int DebugCount =0;//debug��
int Mode2 =0;//debug��
int Mode = 0;                  //OLED��ʾģʽѡ���������Ӧ�ڼ���OLEDģʽ
int Count = 0;                 //���������������м�λ
int Password[6] = {1,1,1,1,1,1};  //�������������룬��Ϊ��ʼ����111111
int State = 0;					//����������״̬��0Ϊ�����������������1Ϊ���Ը��������������ԭ���룻2Ϊ����������
int Input[6] = {0,0,0,0,0,0};		//��������
int Key_Value = 100;
/*�ж������־����OledInterrupt���а���������OLED������������ʾ�������жϷ�������OledInterrupt����1��
 * ����ѭ����OLED������OledInterrupt����0������OLED��ʾ���ݸ��ǡ�*/
int OledInterrupt  =0;
int i = 0;                //ѭ����֤����ʱ�õ���index����
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
//����Ϊ��Ƶʱ��DCO 48MHz
void SystemClockInit()
{
 /* Halting the Watchdog */
	MAP_WDT_A_holdTimer();


      /* �����ⲿ����ʱ������ */
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

    SystemClockInit();			 //����Ϊ��Ƶʱ��HFXT 48MHz
    init();                      //OLED�˿ڶ���
    OLED_Init();                 //��ʼ��SSD1306  OLED
    //int last = 0;
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN4);     //��1��C1
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN5);     //��2��C2
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN6);     //��3��C3
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN7);     //��4��C4

    //���������low��ƽ

    ButOutLowC1;
    ButOutLowC2;
    ButOutLowC3;
    ButOutLowC4;


    //���õ�1��R1--P5.0Ϊ�����������벢ʹ���ж�
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5, GPIO_PIN0);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P5, GPIO_PIN0);
    MAP_GPIO_enableInterrupt(GPIO_PORT_P5, GPIO_PIN0);

    //���õ�2��R2--P5.1Ϊ�����������벢ʹ���ж�
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5, GPIO_PIN1);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P5, GPIO_PIN1);
    MAP_GPIO_enableInterrupt(GPIO_PORT_P5, GPIO_PIN1);

    //���õ�3��R3--P5.2Ϊ�����������벢ʹ���ж�
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5, GPIO_PIN2);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P5, GPIO_PIN2);
    MAP_GPIO_enableInterrupt(GPIO_PORT_P5, GPIO_PIN2);

    //���õ�4��R4--P5.6Ϊ�����������벢ʹ���ж�
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5, GPIO_PIN6);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P5, GPIO_PIN6);
    MAP_GPIO_enableInterrupt(GPIO_PORT_P5, GPIO_PIN6);
    MAP_Interrupt_enableInterrupt(INT_PORT5);

    //Enabling SRAM Bank Retention
    MAP_SysCtl_enableSRAMBankRetention(SYSCTL_SRAM_BANK1);

    //ʹ��ȫ���ж�
    MAP_Interrupt_enableMaster();

//    MAP_WDT_A_holdTimer();     //�رտ��Ź�

    //����ģʽѡ��ѭ��
    while (1)
    {
        if( OledInterrupt == 1)    //���һ���жϷ���OLED������
        {
            OLED_Clear();
            OledInterrupt = 0;
        }
        //ģʽ0, ��ʾ��ʼ����ӭ��Ϣ
		if(Mode == 0)
		{
			OLED_ShowCHinese(0,2,0);      //��
			OLED_ShowCHinese(16,2,1);     //ӭ
			OLED_ShowCHinese(32,2,2);     //ʹ
			OLED_ShowCHinese(48,2,3);     //��
			OLED_ShowCHinese(64,2,4);     //��
			OLED_ShowCHinese(80,2,5);     //��
			OLED_ShowCHinese(96,2,6);     //��
			OLED_ShowCHinese(0,4,7);      //��
			OLED_ShowCHinese(16,4,8);     //��
			OLED_ShowCHinese(32,4,9);     //��
			OLED_ShowCHinese(48,4,4);     //��
			OLED_ShowCHinese(64,4,5);     //��
			OLED_ShowChar(80,4,':');
			State = 0;
		}
        //ģʽ1*
		else if(Mode == 1)
        {
			OLED_ShowCHinese(0,2,7);      //��
			OLED_ShowCHinese(16,2,8);     //��
			OLED_ShowCHinese(32,2,9);     //��
			OLED_ShowCHinese(48,2,4);     //��
			OLED_ShowCHinese(64,2,5);     //��
			OLED_ShowChar(80,2,':');
			OLED_ShowChar(0,4,'*');
        }

        //ģʽ2**
        else if(Mode == 2)
        {
			Mode2 = 1;
        	OLED_ShowCHinese(0,2,7);      //��
			OLED_ShowCHinese(16,2,8);     //��
			OLED_ShowCHinese(32,2,9);     //��
			OLED_ShowCHinese(48,2,4);     //��
			OLED_ShowCHinese(64,2,5);     //��
			OLED_ShowChar(80,2,':');
			OLED_ShowChar(0,4,'*');
			OLED_ShowChar(16,4,'*');
        }

        //ģʽ3***
        else if(Mode == 3)
        {
			OLED_ShowCHinese(0,2,7);      //��
			OLED_ShowCHinese(16,2,8);     //��
			OLED_ShowCHinese(32,2,9);     //��
			OLED_ShowCHinese(48,2,4);     //��
			OLED_ShowCHinese(64,2,5);     //��
			OLED_ShowChar(80,2,':');
			OLED_ShowChar(0,4,'*');
			OLED_ShowChar(16,4,'*');
			OLED_ShowChar(32,4,'*');
        }

        //ģʽ4****
        else if(Mode == 4)
        {
			OLED_ShowCHinese(0,2,7);      //��
			OLED_ShowCHinese(16,2,8);     //��
			OLED_ShowCHinese(32,2,9);     //��
			OLED_ShowCHinese(48,2,4);     //��
			OLED_ShowCHinese(64,2,5);     //��
			OLED_ShowChar(80,2,':');
			OLED_ShowChar(0,4,'*');
			OLED_ShowChar(16,4,'*');
			OLED_ShowChar(32,4,'*');
			OLED_ShowChar(48,4,'*');
        }

        //ģʽ5*****
        else if(Mode == 5)
        {
			OLED_ShowCHinese(0,2,7);      //��
			OLED_ShowCHinese(16,2,8);     //��
			OLED_ShowCHinese(32,2,9);     //��
			OLED_ShowCHinese(48,2,4);     //��
			OLED_ShowCHinese(64,2,5);     //��
			OLED_ShowChar(80,2,':');
			OLED_ShowChar(0,4,'*');
			OLED_ShowChar(16,4,'*');
			OLED_ShowChar(32,4,'*');
			OLED_ShowChar(48,4,'*');
			OLED_ShowChar(64,4,'*');
        }

        //ģʽ6******������1s��ת��Mode0
        else if(Mode == 6)
        {
			//last = 1;
        	OLED_ShowCHinese(0,2,7);      //��
			OLED_ShowCHinese(16,2,8);     //��
			OLED_ShowCHinese(32,2,9);     //��
			OLED_ShowCHinese(48,2,4);     //��
			OLED_ShowCHinese(64,2,5);     //��
			OLED_ShowChar(80,2,':');
			OLED_ShowChar(0,4,'*');
			OLED_ShowChar(16,4,'*');
			OLED_ShowChar(32,4,'*');
			OLED_ShowChar(48,4,'*');
			OLED_ShowChar(64,4,'*');
			OLED_ShowChar(80,4,'*');
			//delay_ms(500);
			//��ʾ�������
			OLED_Clear();
			OledInterrupt = 0;
			OLED_ShowCHinese(0,2,4);      //��
			OLED_ShowCHinese(16,2,5);     //��
			OLED_ShowCHinese(32,2,14);    //��
			OLED_ShowCHinese(48,2,15);    //��
			OLED_ShowChar(64,2,'!');
			OLED_ShowChar(80,2,'!');
			OLED_ShowChar(96,2,'!');
			LED2_BlingBlingRed();
			OLED_ShowCHinese(0,4,7);      //��
			OLED_ShowCHinese(16,4,12);    //��
			OLED_ShowCHinese(32,4,10);    //��
			OLED_ShowCHinese(48,4,8);     //��
			OLED_ShowCHinese(64,4,9);     //��
			OLED_ShowCHinese(80,4,4);     //��
			OLED_ShowCHinese(96,4,5);     //��
			delay_ms(500);
			P2OUT &= ~BIT0;
			OLED_Clear();
			OledInterrupt = 0;
			Mode = 0;
        }

        //ģʽ7******����ȷ��1s��ת��Mode0
        else if(Mode == 7)
        {
			OLED_ShowCHinese(0,2,7);      //��
			OLED_ShowCHinese(16,2,8);     //��
			OLED_ShowCHinese(32,2,9);     //��
			OLED_ShowCHinese(48,2,4);     //��
			OLED_ShowCHinese(64,2,5);     //��
			OLED_ShowChar(80,2,':');
			OLED_ShowChar(0,4,'*');
			OLED_ShowChar(16,4,'*');
			OLED_ShowChar(32,4,'*');
			OLED_ShowChar(48,4,'*');
			OLED_ShowChar(64,4,'*');
			OLED_ShowChar(80,4,'*');
			//��ʾ������ȷ
			OLED_Clear();
			OledInterrupt = 0;
			OLED_ShowCHinese(0,2,4);      //��
			OLED_ShowCHinese(16,2,5);     //��
			OLED_ShowCHinese(32,2,16);    //��
			OLED_ShowCHinese(48,2,17);    //ȷ
			OLED_ShowChar(64,2,'!');
			OLED_ShowChar(80,2,'!');
			OLED_ShowChar(96,2,'!');
			LED2_BlingBlingGreen();
			OLED_ShowCHinese(0,4,18);      //��
			OLED_ShowCHinese(16,4,6);      //��
			OLED_ShowCHinese(32,4,23);     //��
			OLED_ShowCHinese(48,4,24);     //��
			OLED_ShowChar(64,4,'!');
			OLED_ShowChar(80,4,'!');
			OLED_ShowChar(96,4,'!');   /*�����пտ��Լ���LED0��LED1��˸ʲô��*/
			delay_ms(500);
			P2OUT &= ~BIT1;
			OLED_Clear();
			OledInterrupt = 0;
			Mode = 0;
        }

        //ģʽ8, �������룬������ԭ����:
        else if(Mode == 8)
        {
        	State = 1;
        	OLED_ShowCHinese(0,2,12);	  //��
        	OLED_ShowCHinese(16,2,25);	  //��
        	OLED_ShowCHinese(32,2,4);	  //��
        	OLED_ShowCHinese(48,2,5);     //��
        	OLED_ShowChar(64,2,',');
        	OLED_ShowCHinese(0,4,7);      //��
			OLED_ShowCHinese(16,4,8);     //��
			OLED_ShowCHinese(32,4,9);     //��
			OLED_ShowCHinese(48,4,11);    //ԭ
			OLED_ShowCHinese(64,4,4);	  //��
        	OLED_ShowCHinese(80,4,5);     //��
        	OLED_ShowChar(96,4,':');
        }

        //ģʽ9, �������룬����ԭ����*
        else if(Mode == 9)
        {
        	OLED_ShowCHinese(0,2,7);      //��
			OLED_ShowCHinese(16,2,8);     //��
			OLED_ShowCHinese(32,2,9);     //��
			OLED_ShowCHinese(48,2,11);    //ԭ
			OLED_ShowCHinese(64,2,4);	  //��
        	OLED_ShowCHinese(80,2,5);     //��
        	OLED_ShowChar(96,2,':');
        	OLED_ShowChar(0,4,'*');
        }

        //ģʽ10, �������룬����ԭ����**
        else if(Mode == 10)
        {
        	OLED_ShowCHinese(0,2,7);      //��
			OLED_ShowCHinese(16,2,8);     //��
			OLED_ShowCHinese(32,2,9);     //��
			OLED_ShowCHinese(48,2,11);    //ԭ
			OLED_ShowCHinese(64,2,4);	  //��
        	OLED_ShowCHinese(80,2,5);     //��
        	OLED_ShowChar(96,2,':');
        	OLED_ShowChar(0,4,'*');
        	OLED_ShowChar(16,4,'*');
        }

        //ģʽ11, �������룬����ԭ����***
        else if(Mode == 11)
        {
        	OLED_ShowCHinese(0,2,7);      //��
			OLED_ShowCHinese(16,2,8);     //��
			OLED_ShowCHinese(32,2,9);     //��
			OLED_ShowCHinese(48,2,11);    //ԭ
			OLED_ShowCHinese(64,2,4);	  //��
        	OLED_ShowCHinese(80,2,5);     //��
        	OLED_ShowChar(96,2,':');
        	OLED_ShowChar(0,4,'*');
        	OLED_ShowChar(16,4,'*');
        	OLED_ShowChar(32,4,'*');
        }

        //ģʽ12, �������룬����ԭ����****
        else if(Mode == 12)
        {
        	OLED_ShowCHinese(0,2,7);      //��
			OLED_ShowCHinese(16,2,8);     //��
			OLED_ShowCHinese(32,2,9);     //��
			OLED_ShowCHinese(48,2,11);    //ԭ
			OLED_ShowCHinese(64,2,4);	  //��
        	OLED_ShowCHinese(80,2,5);     //��
        	OLED_ShowChar(96,2,':');
        	OLED_ShowChar(0,4,'*');
        	OLED_ShowChar(16,4,'*');
        	OLED_ShowChar(32,4,'*');
        	OLED_ShowChar(48,4,'*');
        }

        //ģʽ13, ��������,����ԭ����*****
        else if(Mode == 13)
        {
        	OLED_ShowCHinese(0,2,7);      //��
			OLED_ShowCHinese(16,2,8);     //��
			OLED_ShowCHinese(32,2,9);     //��
			OLED_ShowCHinese(48,2,11);    //ԭ
			OLED_ShowCHinese(64,2,4);	  //��
        	OLED_ShowCHinese(80,2,5);     //��
        	OLED_ShowChar(96,2,':');
        	OLED_ShowChar(0,4,'*');
        	OLED_ShowChar(16,4,'*');
        	OLED_ShowChar(32,4,'*');
        	OLED_ShowChar(48,4,'*');
        	OLED_ShowChar(64,4,'*');
        }

        //ģʽ14, �������룬����ԭ����******,ԭ������󣬸���ʧ�ܣ��ص�Mode0
        else if(Mode == 14)
        {
        	OLED_ShowCHinese(0,2,7);      //��
			OLED_ShowCHinese(16,2,8);     //��
			OLED_ShowCHinese(32,2,9);     //��
			OLED_ShowCHinese(48,2,11);    //ԭ
			OLED_ShowCHinese(64,2,4);	  //��
        	OLED_ShowCHinese(80,2,5);     //��
        	OLED_ShowChar(96,2,':');
        	OLED_ShowChar(0,4,'*');
        	OLED_ShowChar(16,4,'*');
        	OLED_ShowChar(32,4,'*');
        	OLED_ShowChar(48,4,'*');
        	OLED_ShowChar(64,4,'*');
        	OLED_ShowChar(80,4,'*');
			//��ʾ��֤ԭ�������
			OLED_Clear();

			OledInterrupt = 0;
			LED2_BlingBlingRed();
			OLED_ShowCHinese(0,2,11);     //ԭ
			OLED_ShowCHinese(16,2,4);     //��
			OLED_ShowCHinese(32,2,5);     //��
			OLED_ShowCHinese(48,2,14);    //��
			OLED_ShowCHinese(64,2,15);    //��
			OLED_ShowChar(80,2,'!');
			OLED_ShowChar(96,2,'!');
			OLED_ShowChar(112,2,'!');/*�����пտ��Լ���LED0��LED1��˸ʲô��*/

			delay_ms(1000);
			P2OUT &= ~BIT0;
			OLED_Clear();
			OledInterrupt = 0;
			Mode = 0;
        }
        //ģʽ15, �������룬����ԭ����******,ԭ������ȷ����������������״̬��Mode16
        else if(Mode == 15)
        {
        	OLED_ShowCHinese(0,2,7);      //��
			OLED_ShowCHinese(16,2,8);     //��
			OLED_ShowCHinese(32,2,9);     //��
			OLED_ShowCHinese(48,2,11);    //ԭ
			OLED_ShowCHinese(64,2,4);	  //��
        	OLED_ShowCHinese(80,2,5);     //��
        	OLED_ShowChar(96,2,':');
        	OLED_ShowChar(0,4,'*');
        	OLED_ShowChar(16,4,'*');
        	OLED_ShowChar(32,4,'*');
        	OLED_ShowChar(48,4,'*');
        	OLED_ShowChar(64,4,'*');
        	OLED_ShowChar(80,4,'*');
			//��ʾ��֤ԭ������ȷ����ʼ����������
			OLED_Clear();
			OledInterrupt = 0;
			LED2_BlingBlingGreen();
			OLED_ShowCHinese(0,2,11);     //ԭ
			OLED_ShowCHinese(16,2,4);     //��
			OLED_ShowCHinese(32,2,5);     //��
			OLED_ShowCHinese(48,2,16);    //��
			OLED_ShowCHinese(64,2,17);    //ȷ
			OLED_ShowChar(80,2,'!');
			OLED_ShowChar(96,2,'!');
			OLED_ShowChar(112,2,'!');
			/*�����пտ��Լ���LED0��LED1��˸ʲô��*/
			OLED_ShowCHinese(0,4,7);      //��
			OLED_ShowCHinese(16,4,8);     //��
			OLED_ShowCHinese(32,4,9);     //��
			OLED_ShowCHinese(48,4,10);    //��
			OLED_ShowCHinese(64,4,4);	  //��
        	OLED_ShowCHinese(80,4,5);     //��
        	OLED_ShowChar(96,4,':');
			delay_ms(500);
			P2OUT &= ~BIT1;
			OLED_Clear();
			OledInterrupt = 0;
			Mode = 16;
			State = 2;
        }
		//ģʽ16, ��������,����������(��)
		else if(Mode == 16)
		{
			OLED_ShowCHinese(0,2,7);      //��
			OLED_ShowCHinese(16,2,8);     //��
			OLED_ShowCHinese(32,2,9);     //��
			OLED_ShowCHinese(48,2,10);    //��
			OLED_ShowCHinese(64,2,4);	  //��
			OLED_ShowCHinese(80,2,5);     //��
			OLED_ShowChar(96,2,':');
		}
        //ģʽ17, ��������,����������*
        else if(Mode == 17)
        {
        	OLED_ShowCHinese(0,2,7);      //��
			OLED_ShowCHinese(16,2,8);     //��
			OLED_ShowCHinese(32,2,9);     //��
			OLED_ShowCHinese(48,2,10);    //��
			OLED_ShowCHinese(64,2,4);	  //��
        	OLED_ShowCHinese(80,2,5);     //��
        	OLED_ShowChar(96,2,':');
        	OLED_ShowChar(0,4,'*');
        }
        //ģʽ18, ��������,����������**
        else if(Mode == 18)
        {
        	OLED_ShowCHinese(0,2,7);      //��
			OLED_ShowCHinese(16,2,8);     //��
			OLED_ShowCHinese(32,2,9);     //��
			OLED_ShowCHinese(48,2,10);    //��
			OLED_ShowCHinese(64,2,4);	  //��
        	OLED_ShowCHinese(80,2,5);     //��
        	OLED_ShowChar(96,2,':');
        	OLED_ShowChar(0,4,'*');
        	OLED_ShowChar(16,4,'*');
        }
        //ģʽ19, ��������,����������***
        else if(Mode == 19)
        {
        	OLED_ShowCHinese(0,2,7);      //��
			OLED_ShowCHinese(16,2,8);     //��
			OLED_ShowCHinese(32,2,9);     //��
			OLED_ShowCHinese(48,2,10);    //��
			OLED_ShowCHinese(64,2,4);	  //��
        	OLED_ShowCHinese(80,2,5);     //��
        	OLED_ShowChar(96,2,':');
        	OLED_ShowChar(0,4,'*');
        	OLED_ShowChar(16,4,'*');
        	OLED_ShowChar(32,4,'*');
        }
        //ģʽ20, ��������,����������****
        else if(Mode == 20)
        {
        	OLED_ShowCHinese(0,2,7);      //��
			OLED_ShowCHinese(16,2,8);     //��
			OLED_ShowCHinese(32,2,9);     //��
			OLED_ShowCHinese(48,2,10);    //��
			OLED_ShowCHinese(64,2,4);	  //��
        	OLED_ShowCHinese(80,2,5);     //��
        	OLED_ShowChar(96,2,':');
        	OLED_ShowChar(0,4,'*');
        	OLED_ShowChar(16,4,'*');
        	OLED_ShowChar(32,4,'*');
        	OLED_ShowChar(48,4,'*');
        }
        //ģʽ21, ��������,����������*****
        else if(Mode == 21)
        {
        	OLED_ShowCHinese(0,2,7);      //��
			OLED_ShowCHinese(16,2,8);     //��
			OLED_ShowCHinese(32,2,9);     //��
			OLED_ShowCHinese(48,2,10);    //��
			OLED_ShowCHinese(64,2,4);	  //��
        	OLED_ShowCHinese(80,2,5);     //��
        	OLED_ShowChar(96,2,':');
        	OLED_ShowChar(0,4,'*');
        	OLED_ShowChar(16,4,'*');
        	OLED_ShowChar(32,4,'*');
        	OLED_ShowChar(48,4,'*');
        	OLED_ShowChar(64,4,'*');
        }
        //ģʽ22, ��������,����������*****
        else if(Mode == 22)
        {
        	OLED_ShowCHinese(0,2,7);      //��
			OLED_ShowCHinese(16,2,8);     //��
			OLED_ShowCHinese(32,2,9);     //��
			OLED_ShowCHinese(48,2,10);    //��
			OLED_ShowCHinese(64,2,4);	  //��
        	OLED_ShowCHinese(80,2,5);     //��
        	OLED_ShowChar(96,2,':');
        	OLED_ShowChar(0,4,'*');
        	OLED_ShowChar(16,4,'*');
        	OLED_ShowChar(32,4,'*');
        	OLED_ShowChar(48,4,'*');
        	OLED_ShowChar(64,4,'*');
        	OLED_ShowChar(80,4,'*');
        	delay_ms(50);
        	/*�����пտ��Լ���LED0��LED1��˸ʲô��*/
			OLED_Clear();
			OledInterrupt = 0;
			LED2_BlingBlingBlue();
			//��ʾ ���������óɹ���ת��Mode0
			OLED_ShowCHinese(0,2,10);     //��
			OLED_ShowCHinese(16,2,4);	  //��
        	OLED_ShowCHinese(32,2,5);     //��
        	OLED_ShowCHinese(48,2,13);    //��
        	OLED_ShowCHinese(64,2,25);    //��
        	OLED_ShowCHinese(80,2,23);    //��
        	OLED_ShowCHinese(96,2,24);    //��
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


/* GPIO�жϷ������  */
/* ���ж��н�����ɨ���ж��ĸ��������£��жϽ�������������Ϊ0. */

void PORT5_IRQHandler(void)
{
    int Wrong = 0;                                                      //ȷ����֤����ʱ�õ���Flag����
	uint32_t Interupt_State;                              				    //����һ��״̬��־�����Ĵ��ж����
	//DebugCount++;

    //���ж����ĸ��������¡�S1-S10�ֱ��Ӧ������1-9��0,Key_Value=1-9��0
    //S11Ϊ������֤��������״̬,Key_Value=11;S12Ϊɾ��һλ����Key_Value=12;����Ϊ��Ч����
    //uint32_t Key_Value = 10;	//��ʼ��Ϊһ����Чֵ����ΪS10����0������Key_Value��0
    /**************************************************************/
    /**************************************************************/
    /**************************************************************/
    //���ɰ�������Key_Valueֵ
    //�жϵڶ��а����Ƿ���,��������Ϊ0
    //delay_ms(20);
	ButOutHighC1;
	ButOutLowC2;
	ButOutHighC3;
	ButOutHighC4;

	if(!(MAP_GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN0)))              //S2����
	{
		//delay_ms(100);
		delay_ms(20);
		Key_Value=2;
		//delay_ms(20);
	}
	else if(!(MAP_GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN1))  )       //S6����
	{

		//delay_ms(100);
		delay_ms(20);
		Key_Value=6;

	}
	else if(!(MAP_GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN2))  )        //S10����
	{
		delay_ms(20);
		Key_Value=0;
	}
	else if(!(MAP_GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN6))  )        //S14����
	{
		delay_ms(20);
		Key_Value=14;
	}
	//delay_ms(20);
    //�жϵ�һ�а����Ƿ���,��������Ϊ0
    ButOutLowC1;
    ButOutHighC2;
    ButOutHighC3;
    ButOutHighC4;

    if(!(MAP_GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN0)))              //S1����
    {
    	delay_ms(20);
    	Key_Value=1;
    	DebugCount++;
    }
    else if(!(MAP_GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN1))  )       //S5����
    {
    	delay_ms(20);
    	Key_Value=5;
    }
    else if(!(MAP_GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN2))  )        //S9����
    {
    	delay_ms(20);
    	Key_Value=9;
    }
    else if(!(MAP_GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN6))  )        //S13����
    {
    	delay_ms(20);
    	Key_Value=13;
    }

    //delay_ms(20);
    //�жϵ����а����Ƿ���,��������Ϊ0
    ButOutHighC1;
    ButOutHighC2;
    ButOutLowC3;
    ButOutHighC4;

    if(!(MAP_GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN0)))              //S3����
    {
    	delay_ms(20);
    	Key_Value=3;
    }
    else if(!(MAP_GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN1))  )       //S7����
    {
    	delay_ms(20);
    	Key_Value=7;

    }
    else if(!(MAP_GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN2)) )        //S11����
    {
    	delay_ms(20);
    	Key_Value=11;
    }
    else if(!(MAP_GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN6)) )        //S15����
    {
    	delay_ms(20);
    	Key_Value=15;
    }
    //delay_ms(20);
    //�жϵ����а����Ƿ���,��������Ϊ0
    ButOutHighC1;
    ButOutHighC2;
    ButOutHighC3;
    ButOutLowC4;

    if(!(MAP_GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN0)))              //S4����
    {
    	delay_ms(20);
    	Key_Value=4;
    }
    else if(!(MAP_GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN1)) )        //S8����
    {
    	delay_ms(20);
        Key_Value=8;
    }
    else if(!(MAP_GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN2)) )        //S12����
    {
        delay_ms(20);
        Key_Value=12;
    }
    else if(!(MAP_GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN6)) )        //S16����
    {
    	delay_ms(20);
    	Key_Value=16;
    }

    /**************************************************************/
    /**************************************************************/
    /**************************************************************/
    /**************************************************************/
    //����������Key_Valueֵ���ж�Mode��Count��GlobalVarientֵ

    if(Key_Value == 11)      //���������������룬�����State 1
    {
    	if(Mode == 0){
    		Mode = 8;
    		State = 1;
    	}
    }
    else if(Key_Value == 12) //ɾ��һλ���빦��
    {
    	if(Count > 0){
    		Count--;
    		Mode --;
    	}
    }
    else if(Key_Value < 10)  //��������Ϊ0-9
    {

    	Count++;
    	Input[Count - 1] = Key_Value;
    	//��������״̬Ϊ�������������߳��Ը���������е���������ʱ
    	if(State == 0 || State == 1)
    	{
    		if(Count == 6){
    			Count = 0;
    			//��֤Passport��Input�Ƿ���ͬ
    			for(i=0;i<6;i++){
    				if(Password[i] != Input[i]){
    					Wrong = 1;
    				}
    			}
    			//���������
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
    	//��������״̬Ϊ����������ʱ
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
    //��������0��������һ�ν����ж�
    ButOutLowC1;
    ButOutLowC2;
    ButOutLowC3;
    ButOutLowC4;
    //delay_ms(100);
    OledInterrupt = 1;                               						//�����жϷ������OledInterrupt��1
	Interupt_State = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P5);
	MAP_GPIO_clearInterruptFlag(GPIO_PORT_P5, Interupt_State);              //���������жϱ�־λ
}
