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
*文件名   : os.h
*作用     : RTOS头文件
*创建时间 : 2013.7.21
********************************************************************************
*/

#ifndef __PID_H
#define __PID_H

typedef struct 
{
    float P;                // 比例系数
    float I;                // 积分系数
    float Acc;              // 积分实际值，使用前清零
    //float D;              // 微分系数，一般不用
    
    float ErrorLimit;       // 积分时，用于限制前期误差值太大导致积分值过大而引起过冲
    
    float Target;           // 期望目标值
    float Input;            // 输入值
    float Output;           // 输出值
}PidStruct;

extern void InitPid(void);

#endif






















