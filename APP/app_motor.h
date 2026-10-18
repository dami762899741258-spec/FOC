#ifndef APP_MOTOR_H
#define APP_MOTOR_H

#include <stdint.h>

/* ============================================================================
 *  APP 层：电机业务接口（骨架）
 *  ---------------------------------------------------------------------------
 *  职责：把"人/上位机想干什么"翻译成 FOC 层调用，只做参数检查与限幅，
 *        不做算法、不碰寄存器。
 *  典型调用链：
 *      App_Motor_SetSpeed(rpm) → FOC_SetSpeed(rpm) → 速度环 → 电流环 → PWM
 * ========================================================================== */

/* 初始化（BSP_Init() / FOC_Init() 之后调用） */
void App_Motor_Init(void);

/* 1kHz 任务（放在 main.c 的 1kHz 软定时里） */
void App_Motor_Task1kHz(void);

/* 使能 / 失能（失能 = 关桥自由停车 + 清故障） */
void App_Motor_Enable(void);
void App_Motor_Disable(void);

/* 启动 / 停机（启动 = 对齐 → 闭环；停机 = 关桥回 IDLE） */
void App_Motor_Start(void);
void App_Motor_Stop(void);

/* 给定（内部限幅） */
void App_Motor_SetSpeed(float rpm);     /* 速度给定 rpm */
void App_Motor_SetTorque(float iq_a);   /* 力矩给定 A */
void App_Motor_SetPosition(float rad);  /* 位置给定 rad（可选/待实现） */

/* 查询 */
uint8_t App_Motor_GetState(void);       /* 对应 FOC_State_t */
uint16_t App_Motor_GetFault(void);      /* BSP 故障位 */

#endif /* APP_MOTOR_H */
