#ifndef BSP_PWM_H
#define BSP_PWM_H

#include <stdint.h>

/* ============================================================================
 *  三相 PWM 逆变桥 BSP（TIM1，6 路互补 + 死区）
 *  ---------------------------------------------------------------------------
 *  引脚：PA8/PA9/PA10 = HIN1/2/3（上管，TIM1_CH1/2/3）
 *        PB13/PB14/PB15 = LIN1/2/3（下管，TIM1_CH1N/2N/3N）
 *  死区、互补、中心对齐、ARR 全部由 CubeMX 配好（见 bsp_config.h 顶部注释）。
 *
 *  本模块只做三件事：
 *    1) 启动 TIM1 计数 + CH4（ADC 触发源）—— 此时六路桥臂仍然关断
 *    2) 桥臂使能/关断（关断 = 自由停车，没有刹车）
 *    3) 占空比 0~1 → CCR
 *
 *  ★ FOC 层只允许通过本模块输出，禁止直接写 TIM1->CCR。
 * ========================================================================== */

/* 初始化：启动 TIM1 计数与 CH4 触发（六路输出保持关断，必须显式使能才有输出） */
void Bsp_Pwm_Init(void);

/* 桥臂输出使能/关断（0 = 六管全关 = 自由停车） */
void Bsp_Pwm_OutputEnable(uint8_t en);
uint8_t Bsp_Pwm_OutputIsEnabled(void);

/* 设置三相占空比 0~1（内部按 BSP_MIN_DUTY/BSP_MAX_DUTY 限幅） */
void Bsp_Pwm_SetDuty(float da, float db, float dc);

/* 直接写比较值（调试/高级用，单位 counts，范围 0..ARR+1） */
void Bsp_Pwm_SetDutyRaw(uint16_t cra, uint16_t crb, uint16_t crc);

/* 输出零电压矢量（三相 50%） */
void Bsp_Pwm_SetZero(void);

/* 取 TIM1 ARR（SVPWM 换算用） */
uint16_t Bsp_Pwm_GetArr(void);

#endif /* BSP_PWM_H */
