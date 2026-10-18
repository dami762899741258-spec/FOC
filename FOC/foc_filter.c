#include "foc_filter.h"

/* ============================================================================
 *  一阶低通（骨架：由你实现）
 * ========================================================================== */

//note : 一阶低通一步迭代：y ← y + α·(x − y)
//need : x 本次输入；state 内部状态指针（初始化成第一个输入值）；alpha 0~1
//return : 滤波后的值（同时更新 *state）
//TODO : 你来实现（3 行）
float FOC_LPF(float x, float *state, float alpha)
{
    (void)alpha;

    if (state == 0)
    {
        return x;
    }
    /* TODO: *state += alpha * (x - *state); return *state; */
    return *state;
}
