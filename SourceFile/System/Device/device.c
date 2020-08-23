/*
********************************************************************************
*                                Ƕ��ʽ΢ϵͳ
*                                   msOS
*
*                            Ӳ��ƽ̨:msPLC DEMO
*                          ��оƬ:STM32F103R8T6/RBT6
*                           ��������οƼ����޹�˾
*
*                                ����:����ΰ
*                                ����:������
*                                ��ʶ:Wangsw
*
*                                QQ:26033613
*                               QQȺ:291235815
*                        ��̳:http://bbs.huayusoft.com
*                        �Ա���:http://52edk.taobao.com
*                ����:http://forum.eet-cn.com/BLOG_wangsw317_1268.HTM
********************************************************************************
*�ļ���       : device.c
*����         : �豸�ӿ�
*ԭ��         : ��
********************************************************************************
*�汾     ����            ����            ˵��
*V0.1    Wangsw        2013/07/21       ��ʼ�汾
********************************************************************************
*/



#include "drive.h"
#include "system.h"

extern void InitUsart1(void);

extern void InitUsart2(void);

extern void InitAdc(void);

extern void InitDI(void);

extern void InitDO(void);

extern void InitKey(void);

extern void InitLcd(void);

extern void InitTimer(void);

extern void InitStorage(void);

extern void InitMisc(void);

extern void InitSystick(void);
/*******************************************************************************
* ����	    : ��ʼ���豸
*******************************************************************************/
void InitDevice(void)
{    
    InitUsart1();	
    
    InitUsart2();

    InitAdc();

    InitDI();

    InitDO();

    InitKey();	

    InitLcd();

    InitTimer();

    InitStorage();

    InitMisc();

    InitSystick();

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);     // ��ռ���ȼ�|����ռ���ȼ�,0Ϊ��
                                                        // bit3 bit2 | bit1 bit0 
                                                        //  0 ~ 3    |    0 ~ 3 
	NVIC_SetPriority(SysTick_IRQn, 0x0E);               //    3      |      2
 	NVIC_SetPriority(PendSV_IRQn, 0x0F);                //    3      |      3
}

