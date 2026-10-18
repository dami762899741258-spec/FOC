#ifndef BSP_ADC_H
#define BSP_ADC_H

#include <stdint.h>

/* ============================================================================
 *  电流 / 母线电压采样 BSP
 *  ---------------------------------------------------------------------------
 *  电流（三运放 + ADC 注入组，10kHz 硬件触发）：
 *      IA = OPAMP1_VOUT = PA2  = ADC1_IN3   → ADC1 注入 Rank1
 *      IC = OPAMP3_VOUT = PB1  = ADC1_IN12  → ADC1 注入 Rank2
 *      IB = OPAMP2_VOUT = PA6  = ADC2_IN3   → ADC2 注入 Rank1
 *      触发：TIM1_CC4 上升沿（计数顶点附近，三相下管全开）
 *
 *  母线电压（ADC2 规则组，1kHz 任务里软件触发轮询）：
 *      VBUS = PA0 = ADC2_IN1（板上分压，见 BSP_VBUS_DIV_RATIO）
 *
 *  ★ 外设（注入组/触发源/时钟/GPIO）全部在 CubeMX 里配好，本模块只做：
 *      1) 启动运放 + ADC 校准
 *      2) 启动注入转换
 *      3) 上电零漂平均
 *      4) 原始值 → 安培/伏特
 * ========================================================================== */

typedef struct
{
    uint16_t raw[3];    /* 最近一帧原始值：[0]=IA [1]=IB [2]=IC */
    float offset[3];    /* 零漂（上电校准得到的原始值） */
    float ia;           /* A */
    float ib;           /* A */
    float ic;           /* A */
    uint8_t valid;      /* 1 = ia/ib/ic 可用（零漂校准完成后才会置 1） */
} Bsp_Adc_Cur_t;

typedef struct
{
    uint16_t vbus_raw;  /* 母线电压原始值 */
    float vbus;         /* V */
    uint8_t vbus_valid; /* 1 = vbus 可用 */
} Bsp_Adc_Aux_t;

/* 初始化：启动 3 路运放 + ADC 自校准 + 清状态（此时还没有电流数据） */
void Bsp_Adc_Init(void);

/* 启动注入转换：ADC1 带中断（10kHz 电流环节拍），ADC2 不带中断 */
void Bsp_Adc_Start(void);

/* 阻塞等待上电零漂平均完成（电机必须静止、桥臂必须关断）。成功返回 1 */
uint8_t Bsp_Adc_CalibrateOffset(void);

/* 零漂是否完成（1 = 完成）。用电流之前先查这个 */
uint8_t Bsp_Adc_IsCalibrated(void);

/* 在 10kHz 注入完成中断里调用：零漂阶段累加，正常阶段换算 ia/ib/ic */
void Bsp_Adc_UpdateCurrent(void);

const Bsp_Adc_Cur_t *Bsp_Adc_GetCur(void);

/* 在 1kHz 任务里调用：软件触发 ADC2 规则组读母线电压（阻塞 ~几µs） */
void Bsp_Adc_SampleAux(void);

const Bsp_Adc_Aux_t *Bsp_Adc_GetAux(void);

/* 原始值 → 安培（用当前零漂） */
float Bsp_Adc_RawToAmp(uint16_t raw, uint8_t phase_idx);

#endif /* BSP_ADC_H */
