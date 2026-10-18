#include "app_motor.h"
#include "foc_state.h"
#include "bsp.h"

/* ============================================================================
 *  APP 层电机业务（骨架：由你实现）
 *  ---------------------------------------------------------------------------
 *  建议实现顺序：Init → Enable/Disable → Start/Stop → SetSpeed/SetTorque
 *  注意：所有函数体都只做"翻译 + 限幅"，真正的控制都在 FOC 层。
 * ========================================================================== */

//note : APP 层初始化
//need : BSP_Init() 与 FOC_Init() 已调用
//return : none
//TODO : 你来实现（如默认参数/状态灯）
void App_Motor_Init(void)
{
}

//note : 1kHz 业务任务（命令处理后的状态刷新、状态灯等）
//need : main.c 的 1kHz 软定时里调
//return : none
//TODO : 你来实现
void App_Motor_Task1kHz(void)
{
}

//note : 使能：清故障 + 开桥（是否立即输出由 FOC 状态决定）
//need : 无
//return : none
//TODO : 你来实现
void App_Motor_Enable(void)
{
}

//note : 失能：关桥自由停车 + 回 IDLE
//need : 无
//return : none
//TODO : 你来实现
void App_Motor_Disable(void)
{
}

//note : 启动：走 FOC 的对齐 → 闭环流程
//need : 电流零漂已完成、母线电压正常
//return : none
//TODO : 你来实现
void App_Motor_Start(void)
{
}

//note : 停机：关桥回 IDLE
//need : 无
//return : none
//TODO : 你来实现
void App_Motor_Stop(void)
{
}

//note : 速度给定（内部按 FOC_RPM_MAX 限幅后转给 FOC）
//need : rpm 目标转速
//return : none
//TODO : 你来实现
void App_Motor_SetSpeed(float rpm)
{
    (void)rpm;
}

//note : 力矩给定（内部按 FOC_IQ_MAX 限幅后转给 FOC）
//need : iq_a q 轴电流 A
//return : none
//TODO : 你来实现
void App_Motor_SetTorque(float iq_a)
{
    (void)iq_a;
}

//note : 位置给定（位置环，选做）
//need : rad 目标机械角
//return : none
//TODO : 你来实现
void App_Motor_SetPosition(float rad)
{
    (void)rad;
}

//note : 取当前 FOC 状态
//need : 无
//return : 状态枚举值（FOC_State_t）
uint8_t App_Motor_GetState(void)
{
    return (uint8_t)FOC_GetState();
}

//note : 取故障位
//need : 无
//return : BSP_FAULT_xxx 组合
uint16_t App_Motor_GetFault(void)
{
    return Bsp_Protect_GetFault();
}
