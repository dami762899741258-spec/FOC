#include "bsp_adc.h"
#include "bsp_config.h"
#include "bsp_tim.h"
#include "main.h"
#include "adc.h"
#include "opamp.h"

/* ============================================================================
 *  电流（注入组）+ 母线电压（规则组）
 *  ---------------------------------------------------------------------------
 *  ★ 外设配置全在 CubeMX：注入组 2+1 通道、T1_CC4 触发、6.5cyc、软件触发规则组
 *    本文件只做运行时行为，不改外设配置。
 * ========================================================================== */

static Bsp_Adc_Cur_t s_cur;
static Bsp_Adc_Aux_t s_aux;
static uint8_t s_calibrated = 0U;
static uint16_t s_cal_cnt = 0U;
static float s_off_acc[3];

/* 注入组读数：ADC1 JDR1 = IA(PA2)、ADC1 JDR2 = IC(PB1)、ADC2 JDR1 = IB(PA6) */
#define ADC_RAW_IA ((uint16_t)hadc1.Instance->JDR1)
#define ADC_RAW_IC ((uint16_t)hadc1.Instance->JDR2)
#define ADC_RAW_IB ((uint16_t)hadc2.Instance->JDR1)

//note : 电流采样链初始化：清状态 → 启动 3 路运放 → ADC 自校准 → 清标志
//need : CubeMX 已配好注入组/触发/GPIO；TIM1 已由 Bsp_Pwm_Init() 启动
//return : none（此时还没有有效电流，valid 仍为 0）
void Bsp_Adc_Init(void)
{
    uint8_t i;

    for (i = 0U; i < 3U; i++)
    {
        s_cur.raw[i] = 0U;
        s_cur.offset[i] = 0.0f;
        s_off_acc[i] = 0.0f;
    }
    s_cur.ia = 0.0f;
    s_cur.ib = 0.0f;
    s_cur.ic = 0.0f;
    s_cur.valid = 0U;

    s_aux.vbus_raw = 0U;
    s_aux.vbus = 0.0f;
    s_aux.vbus_valid = 0U;

    s_calibrated = 0U;
    s_cal_cnt = 0U;

    /* 电流链路第一级：三路运放 */
    (void)HAL_OPAMP_Start(&hopamp1);
    (void)HAL_OPAMP_Start(&hopamp2);
    (void)HAL_OPAMP_Start(&hopamp3);

    /* ADC 自校准（必须在启动转换之前） */
    (void)HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
    (void)HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);

    __HAL_ADC_CLEAR_FLAG(&hadc1, ADC_FLAG_JEOC);
    __HAL_ADC_CLEAR_FLAG(&hadc1, ADC_FLAG_EOC);
    __HAL_ADC_CLEAR_FLAG(&hadc2, ADC_FLAG_JEOC);
    __HAL_ADC_CLEAR_FLAG(&hadc2, ADC_FLAG_EOC);
}

//note : 启动注入转换：ADC1 开中断（10kHz 电流环节拍），ADC2 从机不开中断
//need : 必须先调 Bsp_Adc_Init()
//return : none
void Bsp_Adc_Start(void)
{
    (void)HAL_ADCEx_InjectedStart_IT(&hadc1); /* 10kHz 中断：电流环节拍 */
    (void)HAL_ADCEx_InjectedStart(&hadc2);    /* 从 ADC：不产生中断 */
}

//note : 阻塞等待 10kHz 中断里累计的零漂平均完成（要求电机静止、桥臂关断）
//need : 无（依赖注入中断在跑）
//return : 1 = 校准完成；0 = 超时（多半是 TIM1/CH4 没启动，注入没被触发）
uint8_t Bsp_Adc_CalibrateOffset(void)
{
    uint32_t t0 = Bsp_Time_MicroSec();

    while (s_calibrated == 0U)
    {
        if ((Bsp_Time_MicroSec() - t0) > (BSP_ADC_OFFSET_TIMEOUT_MS * 1000U))
        {
            return 0U; /* 超时：多半是注入转换没被触发（TIM1/CH4 没启动） */
        }
    }
    return 1U;
}

uint8_t Bsp_Adc_IsCalibrated(void)
{
    return s_calibrated;
}

//note : 10kHz 注入完成中断里调用：零漂阶段累加原始值，正常阶段换算三相电流
//need : 无（直接读 ADC1 JDR1/JDR2、ADC2 JDR1）
//return : none（结果写进 Bsp_Adc_GetCur() 的结构体）
void Bsp_Adc_UpdateCurrent(void)
{
    uint16_t raw_a = ADC_RAW_IA;
    uint16_t raw_b = ADC_RAW_IB;
    uint16_t raw_c = ADC_RAW_IC;

    s_cur.raw[BSP_ADC_IA_IDX] = raw_a;
    s_cur.raw[BSP_ADC_IB_IDX] = raw_b;
    s_cur.raw[BSP_ADC_IC_IDX] = raw_c;

    if (s_calibrated == 0U)
    {
        /* 零漂平均阶段：只累加，不输出有效电流 */
        s_off_acc[BSP_ADC_IA_IDX] += (float)raw_a;
        s_off_acc[BSP_ADC_IB_IDX] += (float)raw_b;
        s_off_acc[BSP_ADC_IC_IDX] += (float)raw_c;
        s_cal_cnt++;

        if (s_cal_cnt >= BSP_ADC_OFFSET_TIMES)
        {
            s_cur.offset[BSP_ADC_IA_IDX] = s_off_acc[BSP_ADC_IA_IDX] / (float)s_cal_cnt;
            s_cur.offset[BSP_ADC_IB_IDX] = s_off_acc[BSP_ADC_IB_IDX] / (float)s_cal_cnt;
            s_cur.offset[BSP_ADC_IC_IDX] = s_off_acc[BSP_ADC_IC_IDX] / (float)s_cal_cnt;
            s_calibrated = 1U;
        }
        return;
    }

    s_cur.ia = ((float)raw_a - s_cur.offset[BSP_ADC_IA_IDX]) * BSP_I_GAIN_A_PER_LSB * BSP_ADC_IA_SIGN;
    s_cur.ib = ((float)raw_b - s_cur.offset[BSP_ADC_IB_IDX]) * BSP_I_GAIN_A_PER_LSB * BSP_ADC_IB_SIGN;
    s_cur.ic = ((float)raw_c - s_cur.offset[BSP_ADC_IC_IDX]) * BSP_I_GAIN_A_PER_LSB * BSP_ADC_IC_SIGN;
    s_cur.valid = 1U;
}

const Bsp_Adc_Cur_t *Bsp_Adc_GetCur(void)
{
    return &s_cur;
}

//note : 1kHz 调一次：软件触发 ADC2 规则组，轮询读母线电压原始值并换算
//need : BSP_VBUS_ENABLE=1；注入组同时在跑也不冲突（规则/注入是独立序列）
//return : none（结果写进 Bsp_Adc_GetAux()）
void Bsp_Adc_SampleAux(void)
{
#if (BSP_VBUS_ENABLE != 0)
    /* 注入组在跑，但规则组是软件触发，HAL 允许单独启动规则组 */
    if (HAL_ADC_Start(&hadc2) == HAL_OK)
    {
        if (HAL_ADC_PollForConversion(&hadc2, 2U) == HAL_OK)
        {
            s_aux.vbus_raw = (uint16_t)HAL_ADC_GetValue(&hadc2);
            s_aux.vbus = (float)s_aux.vbus_raw * BSP_VBUS_GAIN_V_PER_LSB;
            s_aux.vbus_valid = 1U;
        }
    }
#endif
}

const Bsp_Adc_Aux_t *Bsp_Adc_GetAux(void)
{
    return &s_aux;
}

//note : 原始 ADC 值 → 安培（用当前零漂换算，不含相电流符号修正）
//need : raw 原始值；phase_idx 0=IA 1=IB 2=IC
//return : 电流 A；下标越界返回 0
float Bsp_Adc_RawToAmp(uint16_t raw, uint8_t phase_idx)
{
    if (phase_idx > 2U)
    {
        return 0.0f;
    }
    return ((float)raw - s_cur.offset[phase_idx]) * BSP_I_GAIN_A_PER_LSB;
}
