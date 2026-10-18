#ifndef FOC_MATH_H
#define FOC_MATH_H

#include <stdint.h>

/* ============================================================================
 *  FOC 坐标变换（骨架）
 *  ---------------------------------------------------------------------------
 *  全部是纯函数（给输入→出输出，不带状态），最好写也最好测。
 *  公式见每个函数的 note 注释（和旧工程 FOC-3.0t 的 foc_math.h 一致）。
 * ========================================================================== */

/* 三相电流 → 两相静止 */
void FOC_Clarke(float ia, float ib, float ic, float *ialpha, float *ibeta);

/* 两相静止 + 电角度 → 旋转坐标系 dq */
void FOC_Park(float ialpha, float ibeta, float theta, float *id, float *iq);

/* 旋转坐标系 dq + 电角度 → 两相静止 */
void FOC_InvPark(float ud, float uq, float theta, float *ualpha, float *ubeta);

/* 角度归一到 [-π, π]（PI 调节器/比较角度时用） */
float FOC_WrapPi(float angle);

/* 角度归一到 [0, 2π)（SVPWM/显示时用） */
float FOC_Wrap2Pi(float angle);

#endif /* FOC_MATH_H */
