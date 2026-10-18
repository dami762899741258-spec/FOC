#include "foc_svpwm.h"

/* ============================================================================
 *  SVPWM（骨架：由你实现，推荐零序注入法）
 * ========================================================================== */

//note : 两相静止电压 → 三相占空比 0~1（零序注入法，等价七段式 SVPWM）
//need : ualpha/ubeta 电压 V；vdc 母线电压 V；duty 三个输出指针
//return : none
//TODO : 你来实现（步骤见 foc_svpwm.h 注释）
void FOC_SVPWM(float ualpha, float ubeta, float vdc,
               float *duty_a, float *duty_b, float *duty_c)
{
    (void)ualpha;
    (void)ubeta;
    (void)vdc;

    /* 骨架先给零矢量 0.5，保证输出安全（不会乱动） */
    if (duty_a != 0)
    {
        *duty_a = 0.5f;
    }
    if (duty_b != 0)
    {
        *duty_b = 0.5f;
    }
    if (duty_c != 0)
    {
        *duty_c = 0.5f;
    }
}
