#ifndef FOC_LOOP_H
#define FOC_LOOP_H

#include <stdint.h>

/* ============================================================================
 *  PI 调节器 + 电流环/速度环（骨架）
 *  ---------------------------------------------------------------------------
 *  通用 PI（位置式，带抗积分饱和）：
 *      err       = ref - fdb
 *      integral += ki · err · ts
 *      out       = kp · err + integral
 *      若 out 越限：先限幅，再把这一次的积分"退回去"（防积分饱和）
 *  注意 ts 必须是"这个环自己的周期"（电流环 100µs / 速度环 1ms）。
 * ========================================================================== */

typedef struct
{
    float kp;       /* 比例增益 */
    float ki;       /* 积分增益（量纲带 /s） */
    float out_max;  /* 输出限幅（±out_max） */
    float integral; /* 积分累加器（内部状态） */
    float out;      /* 上次输出（调试观察用） */
} FOC_PI_t;

/* 初始化一个 PI（同时清零积分） */
void FOC_PI_Init(FOC_PI_t *pi, float kp, float ki, float out_max);

/* 清零积分（切换状态/停机时必须调用，否则残留旧积分） */
void FOC_PI_Reset(FOC_PI_t *pi);

/* 跑一拍：ref 给定、fdb 反馈、ts 周期，返回输出（已限幅） */
float FOC_PI_Run(FOC_PI_t *pi, float ref, float fdb, float ts);

/* ---------------- 环级接口 ---------------- */

/* 初始化电流环(id/iq)与速度环的 PI 参数（读 foc_config.h 的宏） */
void FOC_Loop_Init(void);

/* 电流环一拍：输入 id/iq 反馈与给定，输出 vd/vq（已按 FOC_VMAX 限幅） */
void FOC_Loop_Current(float id_ref, float iq_ref, float id_fdb, float iq_fdb,
                      float *vd_out, float *vq_out);

/* 速度环一拍：输入转速给定与反馈，返回 iq_ref（已按 FOC_IQ_MAX 限幅） */
float FOC_Loop_Speed(float rpm_ref, float rpm_fdb);

/* 清空所有积分（进 FAULT / 重新启动前调用） */
void FOC_Loop_Reset(void);

#endif /* FOC_LOOP_H */
