#ifndef FOC_SVPWM_H
#define FOC_SVPWM_H

#include <stdint.h>

/* ============================================================================
 *  SVPWM（骨架）—— 两相静止电压 (uα,uβ) → 三相占空比 0~1
 *  ---------------------------------------------------------------------------
 *  推荐用"零序注入法"（结果与七段式 SVPWM 等价）：
 *      1) 反 Clarke：a = uα, b = -0.5uα + (√3/2)uβ, c = -0.5uα - (√3/2)uβ
 *      2) 零序注入：offset = -(max(a,b,c) + min(a,b,c)) / 2
 *      3) 占空比：d = 0.5 + (a+offset)/Vdc（三相同式）
 *  自测：uα = Vmax, uβ = 0 时 duty ≈ 0.933 / 0.067 / 0.067
 *  ★ 输出 0~1 的占空比，交给 Bsp_Pwm_SetDuty()，本层不碰寄存器。
 * ========================================================================== */

//note : 两相静止电压 → 三相占空比（零序注入 SVPWM）
//need : ualpha/ubeta 电压 V；vdc 母线电压 V（来自 Bsp_Adc_GetAux()->vbus）
//return : none（结果写 *duty_a/*duty_b/*duty_c，范围 0~1）
void FOC_SVPWM(float ualpha, float ubeta, float vdc,
               float *duty_a, float *duty_b, float *duty_c);

#endif /* FOC_SVPWM_H */
