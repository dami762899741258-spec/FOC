#include "bsp_encoder.h"
#include "bsp_config.h"
#include "main.h"
#include "tim.h"
#include <math.h>

/* ============================================================================
 *  ABZ 编码器（TIM3）—— 把 16bit 计数变成多圈位置 / 机械角 / 电角度 / 转速
 *  ---------------------------------------------------------------------------
 *  核心技巧（沿用样例）：差分用 (int16_t)(now - last)，这样 65535↔0 的
 *  回绕会被自动解释成 ±1，不需要额外判断方向。
 *  ★ Z 与转子磁极的相位关系需要实测，零点标定策略由 FOC 层决定。
 * ========================================================================== */

#define TWO_PI_F 6.28318530718f

static Bsp_Enc_t s_enc;

static uint16_t s_last_cnt;       /* 上次 CNT，用于差分 */
static int32_t  s_cnt_multi;      /* 多圈累计计数 */
static int32_t  s_speed_acc;      /* 测速窗口内累计计数 */
static uint8_t  s_speed_decim;    /* 测速窗口分频计数 */
static float    s_speed_lpf;      /* 转速一阶低通状态 */

static float    s_zero_mech_rad;  /* 电角度零点对应的机械角 */
static uint8_t  s_z_seen;         /* 是否见过 Z */
static uint32_t s_z_count;        /* Z 次数 */
static uint16_t s_z_raw;          /* 最近一次 Z 时的 CNT */

//note : 把任意角度归一到 [0, 2π)
//need : a 任意弧度
//return : 归一化后的角度 0~2π
static float wrap_2pi(float a)
{
    a = fmodf(a, TWO_PI_F);
    if (a < 0.0f)
    {
        a += TWO_PI_F;
    }
    return a;
}

//note : 启动 TIM3 编码器接口，并清零多圈/测速/零点/Z 状态
//need : CubeMX 已配好 TIM3 编码器模式和 PB3 的 Z 中断（gpio.c 里已开）
//return : none
void Bsp_Enc_Init(void)
{
    (void)HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);

    s_last_cnt = (uint16_t)TIM3->CNT;
    s_cnt_multi = 0;
    s_speed_acc = 0;
    s_speed_decim = 0U;
    s_speed_lpf = 0.0f;
    s_zero_mech_rad = 0.0f;
    s_z_seen = 0U;
    s_z_count = 0U;
    s_z_raw = 0U;

    s_enc.raw = s_last_cnt;
    s_enc.cnt = 0;
    s_enc.mech_rad = 0.0f;
    s_enc.cont_turns = 0.0f;
    s_enc.elec_rad = 0.0f;
    s_enc.speed_rad_s = 0.0f;
    s_enc.speed_rpm = 0.0f;
    s_enc.online = 0U;
}

//note : Z 信号上升沿中断里调用：记录 Z 次数和当次 CNT，供 FOC 层做零点校正
//need : 无（在 main.c 的 HAL_GPIO_EXTI_Callback 里转发）
//return : none
void Bsp_Enc_ZIsr(void)
{
    s_z_count++;
    s_z_seen = 1U;
    s_z_raw = (uint16_t)TIM3->CNT;
}

//note : 12.5kHz 调一次：读 CNT → (int16_t)差分解回绕 → 多圈累计 → 角度/测速
//need : 无（在 BSP_Task12k5Hz 里调）
//return : none（结果写进 Bsp_Enc_Get() 的结构体）
void Bsp_Enc_Update(void)
{
    uint16_t now = (uint16_t)TIM3->CNT;
    int16_t  d = (int16_t)(uint16_t)(now - s_last_cnt); /* 回绕自动处理 */
    int32_t  diff;

    s_last_cnt = now;

    /* 方向：BSP_ENC_DIR = +1 / -1 */
    diff = (BSP_ENC_DIR > 0.0f) ? (int32_t)d : -(int32_t)d;

    s_cnt_multi += diff;

    s_enc.raw = now;
    s_enc.cnt = s_cnt_multi;
    s_enc.mech_rad = wrap_2pi((float)s_cnt_multi * TWO_PI_F / BSP_ENC_CPR);
    s_enc.cont_turns = (float)s_cnt_multi / BSP_ENC_CPR;
    s_enc.elec_rad = Bsp_Enc_ElecFromMech(s_enc.mech_rad);
    s_enc.online = s_z_seen;

    /* ---- 窗口测速 + 一阶低通 ---- */
    s_speed_acc += diff;
    s_speed_decim++;
    if (s_speed_decim >= BSP_ENC_SPEED_DECIM)
    {
        float rpm = (float)s_speed_acc * BSP_ENC_RPM_PER_CNT;

        s_speed_acc = 0;
        s_speed_decim = 0U;

        if (fabsf(rpm) < BSP_ENC_SPEED_MIN_RPM)
        {
            rpm = 0.0f;
        }

        s_speed_lpf += BSP_ENC_SPEED_LPF * (rpm - s_speed_lpf);
        s_enc.speed_rpm = s_speed_lpf;
        s_enc.speed_rad_s = s_speed_lpf * TWO_PI_F / 60.0f;
    }
}

const Bsp_Enc_t *Bsp_Enc_Get(void)
{
    return &s_enc;
}

void Bsp_Enc_SetElecZeroMech(float mech_rad)
{
    s_zero_mech_rad = wrap_2pi(mech_rad);
}

float Bsp_Enc_GetElecZeroMech(void)
{
    return s_zero_mech_rad;
}

//note : 机械角 → 电角度：先减零点、再乘极对数，最后归一到 [0,2π)
//need : mech_rad 单圈机械角 0~2π
//return : 电角度 0~2π
float Bsp_Enc_ElecFromMech(float mech_rad)
{
    float mech_delta = wrap_2pi(mech_rad - s_zero_mech_rad);
    return wrap_2pi(mech_delta * (float)BSP_MOTOR_POLE_PAIRS);
}

//note : 多圈计数/测速状态清零（位置环重新定零用，不动零点标定值）
//need : 无
//return : none
void Bsp_Enc_ResetMultiTurn(void)
{
    s_cnt_multi = 0;
    s_speed_acc = 0;
    s_speed_decim = 0U;
    s_speed_lpf = 0.0f;
    s_last_cnt = (uint16_t)TIM3->CNT;
}

uint32_t Bsp_Enc_GetZCount(void)
{
    return s_z_count;
}

uint16_t Bsp_Enc_GetZRaw(void)
{
    return s_z_raw;
}

uint8_t Bsp_Enc_IsOnline(void)
{
    return s_enc.online;
}
