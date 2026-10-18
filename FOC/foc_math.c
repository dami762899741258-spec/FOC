#include "foc_math.h"
#include <math.h>

/* ============================================================================
 *  FOC 坐标变换（骨架：只有 TODO 框架，公式写注释里，由你实现）
 * ========================================================================== */

//note : 三相电流 → 两相静止（等幅值 Clarke）
//need : ia/ib/ic 三相电流 A；ialpha/ibeta 输出指针
//return : none（结果写 *ialpha / *ibeta）
//TODO : 你来实现
//   iα = ia
//   iβ = (ia + 2·ib) / √3
void FOC_Clarke(float ia, float ib, float ic, float *ialpha, float *ibeta)
{
    (void)ia;
    (void)ib;
    (void)ic;
    if (ialpha != 0)
    {
        *ialpha = 0.0f;
    }
    if (ibeta != 0)
    {
        *ibeta = 0.0f;
    }
}

//note : 两相静止 + 电角度 → 两相旋转 dq
//need : ialpha/ibeta 静止量；theta 电角度 rad；id/iq 输出指针
//return : none
//TODO : 你来实现
//   id =  ialpha·cosθ + ibeta·sinθ
//   iq = -ialpha·sinθ + ibeta·cosθ
void FOC_Park(float ialpha, float ibeta, float theta, float *id, float *iq)
{
    (void)ialpha;
    (void)ibeta;
    (void)theta;
    if (id != 0)
    {
        *id = 0.0f;
    }
    if (iq != 0)
    {
        *iq = 0.0f;
    }
}

//note : 两相旋转 dq + 电角度 → 两相静止（反 Park）
//need : ud/uq 电压矢量 V；theta 电角度 rad；ualpha/ubeta 输出指针
//return : none
//TODO : 你来实现
//   uα = ud·cosθ - uq·sinθ
//   uβ = ud·sinθ + uq·cosθ
void FOC_InvPark(float ud, float uq, float theta, float *ualpha, float *ubeta)
{
    (void)ud;
    (void)uq;
    (void)theta;
    if (ualpha != 0)
    {
        *ualpha = 0.0f;
    }
    if (ubeta != 0)
    {
        *ubeta = 0.0f;
    }
}

//note : 角度归一到 [-π, π]（取最短方向，比较角度误差时用）
//need : angle 任意弧度
//return : [-π, π] 内的角度
//TODO : 你来实现（提示：fmodf + 加/减 2π）
float FOC_WrapPi(float angle)
{
    (void)angle;
    return 0.0f;
}

//note : 角度归一到 [0, 2π)
//need : angle 任意弧度
//return : [0, 2π) 内的角度
//TODO : 你来实现
float FOC_Wrap2Pi(float angle)
{
    (void)angle;
    return 0.0f;
}
