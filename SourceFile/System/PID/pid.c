/*
********************************************************************************
*                                嵌入式微系统
*                                   msOS
*
*                            硬件平台:msPLC DEMO
*                          主芯片:STM32F103R8T6/RBT6
*                           深圳市雨滴科技有限公司
*
*                                作者:王绍伟
*                                网名:凤舞天
*                                标识:wangsw
*
*                                QQ:26033613
*                               QQ群:291235815
*                        论坛:http://bbs.huayusoft.com
*                        淘宝店:http://52edk.taobao.com
*                博客:http://forum.eet-cn.com/BLOG_wangsw317_1268.HTM
********************************************************************************
*文件名   : pid.c
*创建时间 : 2018.9.8
********************************************************************************
*/





#include "system.h"

#define Pid AppDataPointer->Pid

static void PidRoutine(void)
{
    static float I = 0;
    float error, errorLimit;

    // 输入赋值
    // Pid.Input = xxx;

    error = Pid.Target - Pid.Input;
    
    if (error > Pid.ErrorLimit) errorLimit = Pid.ErrorLimit;
    else if(error < -Pid.ErrorLimit) errorLimit = -Pid.ErrorLimit;
    else errorLimit = error;
    
    I += errorLimit;
    
    Pid.Output = Pid.P * error + Pid.I * I;

    // 输出赋值
    // xxx = Pid.Output;
}



void InitPid(void)
{    
    Pid.Acc = 0;
    System.Device.Systick.Register(Systick100, PidRoutine); // 利用10mS系统节拍，一般够用了
}

