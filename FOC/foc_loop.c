#include "foc_loop.h"
#include "foc_config.h"

/* ============================================================================
 *  PI + 电流环/速度环（骨架：由你实现）
 *  ---------------------------------------------------------------------------
 *  建议实现顺序：
 *      1) FOC_PI_Init / FOC_PI_Reset  （简单赋值）
 *      2) FOC_PI_Run                  （含抗积分饱和，见头文件公式）
 *      3) FOC_Loop_Init               （按 foc_config.h 的宏初始化 3 个 PI）
 *      4) FOC_Loop_Current / FOC_Loop_Speed（组合 PI + 限幅）
 * ========================================================================== */

/* 电流环 PI 与速度环 PI（内部状态，外部不用访问） */
static FOC_PI_t s_pi_id;
static FOC_PI_t s_pi_iq;
static FOC_PI_t s_pi_spd;
static float s_iq_ref = 0.0f; /* 速度环输出（调试用） */

//note : 初始化一个 PI（同时清零积分）
//need : pi 指针；kp 比例；ki 积分（带 /s）；out_max 输出限幅
//return : none
//TODO : 你来实现
void FOC_PI_Init(FOC_PI_t *pi, float kp, float ki, float out_max)
{
    (void)pi;
    (void)kp;
    (void)ki;
    (void)out_max;
}

//note : 清零积分（切状态/停机时调用，避免旧积分残留）
//need : pi 指针
//return : none
//TODO : 你来实现
void FOC_PI_Reset(FOC_PI_t *pi)
{
    (void)pi;
}

//note : PI 跑一拍（位置式 + 抗积分饱和）
//need : pi 指针；ref 给定；fdb 反馈；ts 本环周期 s
//return : 限幅后的输出
//TODO : 你来实现（公式见 foc_loop.h）
float FOC_PI_Run(FOC_PI_t *pi, float ref, float fdb, float ts)
{
    (void)pi;
    (void)ref;
    (void)fdb;
    (void)ts;
    return 0.0f;
}

//note : 按 foc_config.h 的宏初始化电流环(id/iq)与速度环 PI
//need : 无
//return : none
//TODO : 你来实现（调 FOC_PI_Init 三次）
void FOC_Loop_Init(void)
{
    FOC_PI_Init(&s_pi_id, FOC_KP_ID, FOC_KI_ID, FOC_VMAX);
    FOC_PI_Init(&s_pi_iq, FOC_KP_IQ, FOC_KI_IQ, FOC_VMAX);
    FOC_PI_Init(&s_pi_spd, FOC_KP_SPD, FOC_KI_SPD, FOC_IQ_MAX);
}

//note : 电流环一拍：id/iq 的 PI → vd/vq（电压矢量限幅）
//need : id_ref/iq_ref 给定 A；id_fdb/iq_fdb 反馈 A；vd_out/vq_out 输出指针
//return : none
//TODO : 你来实现
void FOC_Loop_Current(float id_ref, float iq_ref, float id_fdb, float iq_fdb,
                      float *vd_out, float *vq_out)
{
    (void)id_ref;
    (void)iq_ref;
    (void)id_fdb;
    (void)iq_fdb;
    if (vd_out != 0)
    {
        *vd_out = 0.0f;
    }
    if (vq_out != 0)
    {
        *vq_out = 0.0f;
    }
}

//note : 速度环一拍：rpm 误差 → iq_ref（带死区与限幅）
//need : rpm_ref 给定 rpm；rpm_fdb 反馈 rpm
//return : iq_ref（已限幅到 ±FOC_IQ_MAX）
//TODO : 你来实现
float FOC_Loop_Speed(float rpm_ref, float rpm_fdb)
{
    (void)rpm_ref;
    (void)rpm_fdb;
    s_iq_ref = 0.0f;
    return s_iq_ref;
}

//note : 清空所有积分（进 FAULT / 重新启动前调用）
//need : 无
//return : none
void FOC_Loop_Reset(void)
{
    FOC_PI_Reset(&s_pi_id);
    FOC_PI_Reset(&s_pi_iq);
    FOC_PI_Reset(&s_pi_spd);
    s_iq_ref = 0.0f;
}
