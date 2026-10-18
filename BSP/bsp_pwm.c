#include "bsp_pwm.h"
#include "bsp_config.h"
#include "main.h"
#include "tim.h"

/* ============================================================================
 *  三相 PWM（TIM1）
 *  ---------------------------------------------------------------------------
 *  ★ 死区/互补/中心对齐/ARR 由 CubeMX 配置，这里不再重配外设，
 *    只做"启动 + 占空比换算 + 桥臂使能"这三件运行时的事。
 * ========================================================================== */

static uint8_t s_out_en = 0U; /* 桥臂输出状态（0 = 六管全关） */

//note : 占空比限幅到 [BSP_MIN_DUTY, BSP_MAX_DUTY]，防止顶部采样窗口被吃掉
//need : d 原始占空比 0~1
//return : 限幅后的占空比
static float clamp_duty(float d)
{
    if (d < BSP_MIN_DUTY)
    {
        return BSP_MIN_DUTY;
    }
    if (d > BSP_MAX_DUTY)
    {
        return BSP_MAX_DUTY;
    }
    return d;
}

//note : 启动 TIM1 计数和 CH4（ADC 注入触发源）；六路桥臂保持关断
//need : MX_TIM1_Init() 已跑过（中心对齐/ARR/死区/互补由 CubeMX 配好）
//return : none
void Bsp_Pwm_Init(void)
{
    /* 只启动计数 + CH4（ADC 注入触发源），六路桥臂保持关断 */
    __HAL_TIM_SET_AUTORELOAD(&htim1, BSP_PWM_ARR);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, (uint32_t)(BSP_PWM_ARR - 1U));

    (void)HAL_TIM_Base_Start(&htim1);
    (void)HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);

    s_out_en = 0U;
}

//note : 桥臂使能/关断：1 = 3 路上管 + 3 路下管都启动；0 = 六管全关（自由停车）
//need : en 0/1
//return : none
void Bsp_Pwm_OutputEnable(uint8_t en)
{
    if (en != 0U)
    {
        (void)HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
        (void)HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
        (void)HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
        (void)HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
        (void)HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
        (void)HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);
        s_out_en = 1U;
    }
    else
    {
        (void)HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
        (void)HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
        (void)HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
        (void)HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
        (void)HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);
        (void)HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_3);
        s_out_en = 0U;
    }
}

uint8_t Bsp_Pwm_OutputIsEnabled(void)
{
    return s_out_en;
}

//note : 三相占空比 0~1 → 中心对齐 CCR（CCR = duty × (ARR+1)），内部限幅
//need : da/db/dc 三相占空比 0~1
//return : none
void Bsp_Pwm_SetDuty(float da, float db, float dc)
{
    uint32_t period = (uint32_t)BSP_PWM_ARR + 1U; /* 中心对齐：duty×period → CCR */

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, (uint32_t)(clamp_duty(da) * (float)period));
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, (uint32_t)(clamp_duty(db) * (float)period));
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, (uint32_t)(clamp_duty(dc) * (float)period));
}

void Bsp_Pwm_SetDutyRaw(uint16_t cra, uint16_t crb, uint16_t crc)
{
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, (uint32_t)cra);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, (uint32_t)crb);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, (uint32_t)crc);
}

void Bsp_Pwm_SetZero(void)
{
    Bsp_Pwm_SetDuty(0.5f, 0.5f, 0.5f);
}

uint16_t Bsp_Pwm_GetArr(void)
{
    return (uint16_t)BSP_PWM_ARR;
}
