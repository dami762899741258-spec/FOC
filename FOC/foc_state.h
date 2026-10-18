#ifndef FOC_STATE_H
#define FOC_STATE_H

#include <stdint.h>

/* ============================================================================
 *  FOC 顶层：状态机 + 两个入口函数（骨架）
 *  ---------------------------------------------------------------------------
 *  状态迁移：
 *      IDLE ──Start──▶ ALIGN ──对齐完成──▶ OPEN ──(可选)──▶ CLOSED
 *        ▲                                    │                │
 *        └──────────── Stop ◀─────────────────┴─── Stop ◀──────┘
 *      任何状态 ──Bsp_Protect_IsTripped()──▶ FAULT ──ClearFault──▶ IDLE
 *
 *  各状态做什么：
 *      IDLE   : 桥臂关断，清 PI 积分
 *      ALIGN  : 给电角度=0 的固定电压矢量，等转子稳定后标定零点
 *               （FOC_ALIGN_USE_Z=1 用 Z 信号；=0 用电压吸转子）
 *      OPEN   : 电角度按 FOC_OPEN_ELEC_RPM 递增，给固定电压 → 平稳旋转
 *      CLOSED : 电流环 + 速度环（真正闭环）
 *      FAULT  : 已由保护关桥；等 ClearFault
 *
 *  ★ 两个入口必须放在正确位置（main.c 已接好）：
 *      FOC_CurrentLoop() → 10kHz ADC 注入完成中断
 *      FOC_Task1kHz()    → 1kHz 软定时
 * ========================================================================== */

typedef enum
{
    FOC_STATE_IDLE = 0, /* 空闲：桥臂关断，等待启动 */
    FOC_STATE_ALIGN,    /* 对齐：标定电角度零点 */
    FOC_STATE_OPEN,     /* 开环旋转：验证功率级/相序/方向 */
    FOC_STATE_CLOSED,   /* 闭环：电流环 + 速度环 */
    FOC_STATE_FAULT     /* 故障：已关桥，等清故障 */
} FOC_State_t;

/* ---------------- 入口 ---------------- */

/* 初始化（BSP_Init() 之后调用）：清状态/清 PI */
void FOC_Init(void);

/* 10kHz 电流环：读电流/角度 → Clarke/Park → PI → 反Park → SVPWM → 写 PWM */
void FOC_CurrentLoop(void);

/* 1kHz 任务：状态机 / 速度环 / 给定斜坡 */
void FOC_Task1kHz(void);

/* ---------------- 命令 ---------------- */

void FOC_Start(void);      /* IDLE → ALIGN */
void FOC_Stop(void);       /* 任意状态 → IDLE（关桥、清积分） */
void FOC_SetOpenLoop(void);  /* 切到 OPEN */
void FOC_SetClosedLoop(void);/* 切到 CLOSED */
void FOC_ClearFault(void);   /* FAULT → IDLE（同时清 BSP 故障） */

/* ---------------- 给定 ---------------- */

void FOC_SetSpeed(float rpm);   /* 速度给定（CLOSED 用），内部限幅 */
void FOC_SetTorque(float iq_a); /* 力矩给定（直接给 iq_ref），内部限幅 */

/* ---------------- 查询 ---------------- */

FOC_State_t FOC_GetState(void);
const char *FOC_GetStateName(void); /* "IDLE"/"ALIGN"/"OPEN"/"CLOSED"/"FAULT" */

#endif /* FOC_STATE_H */
