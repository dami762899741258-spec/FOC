#ifndef BSP_ENCODER_H
#define BSP_ENCODER_H

#include <stdint.h>

/* ============================================================================
 *  ABZ 编码器 BSP（MT6835 的 A/B/Z 输出 + TIM3 编码器接口）
 *  ---------------------------------------------------------------------------
 *  硬件设定（全部在 CubeMX 里）：
 *      TIM3 : Encoder Mode TI1+TI2，ARR=65535，PB4=A，PB5=B
 *      PB3  : Z 信号，EXTI3 上升沿（gpio.c 里已开中断）
 *      一圈计数 = BSP_ENC_CPR = 65536（ABZ 16384 线 × 4 倍频）
 *
 *  单位约定：
 *      raw       = TIM3->CNT 原始值 0..65535
 *      cnt       = 多圈累计计数（已乘方向）
 *      mech_rad  = 单圈机械角 0..2π
 *      cont_turns= 连续圈数（位置环用）
 *      elec_rad  = 电角度 0..2π（减零点、乘极对数）
 *      speed_rpm = 机械转速 rpm（窗口测速 + 一阶低通）
 *
 *  零点语义：机械角 = zero 时，电角度 = 0。
 *      标定流程（FOC 层做）：给 elec=0 的电压矢量 → 等转子稳定 →
 *      读 Bsp_Enc_Get()->mech_rad → Bsp_Enc_SetElecZeroMech(该值)。
 *      也可以改用 Z 信号做每圈校正（Bsp_Enc_GetZCount/GetZRaw）。
 * ========================================================================== */

typedef struct
{
    uint16_t raw;         /* TIM3->CNT 原始计数 0..65535 */
    int32_t  cnt;         /* 累计多圈计数（已乘方向） */
    float    mech_rad;    /* 单圈机械角 0..2π */
    float    cont_turns;  /* 连续圈数（已乘方向），位置环用 */
    float    elec_rad;    /* 电角度 0..2π */
    float    speed_rad_s; /* 机械角速度 rad/s（已滤波） */
    float    speed_rpm;   /* 机械转速 rpm（已滤波） */
    uint8_t  online;      /* 1 = 见过 Z 信号（编码器物理在线） */
} Bsp_Enc_t;

/* 初始化：启动 TIM3 编码器、清累计量 */
void Bsp_Enc_Init(void);

/* 12.5kHz 调一次（BSP_Task12k5Hz 里）：读 CNT、差分、累计、算角度/转速 */
void Bsp_Enc_Update(void);

/* EXTI3（Z 上升沿）中断里调 */
void Bsp_Enc_ZIsr(void);

const Bsp_Enc_t *Bsp_Enc_Get(void);

/* ---------------- 电角度零点（FOC 对齐后写进来） ---------------- */
void  Bsp_Enc_SetElecZeroMech(float mech_rad);
float Bsp_Enc_GetElecZeroMech(void);
float Bsp_Enc_ElecFromMech(float mech_rad);

/* ---------------- 多圈 / Z 查询 ---------------- */
void     Bsp_Enc_ResetMultiTurn(void);  /* 位置环重新定零用 */
uint32_t Bsp_Enc_GetZCount(void);       /* 上电以来 Z 次数 */
uint16_t Bsp_Enc_GetZRaw(void);         /* 最近一次 Z 时的 CNT */
uint8_t  Bsp_Enc_IsOnline(void);

#endif /* BSP_ENCODER_H */
