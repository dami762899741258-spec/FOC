#include "foc_state.h"
#include "foc_config.h"
#include "foc_math.h"
#include "foc_loop.h"
#include "foc_svpwm.h"
#include "bsp.h"

/* ============================================================================
 *  FOC 顶层状态机（骨架：入口函数已接好，内部逻辑由你实现）
 *  ---------------------------------------------------------------------------
 *  依赖关系（这是本层唯一允许调 BSP 的地方，算法文件保持纯函数）：
 *      Bsp_Adc_GetCur()   → ia/ib/ic
 *      Bsp_Enc_Get()      → mech_rad / elec_rad / speed_rpm
 *      Bsp_Pwm_SetDuty()  → 输出占空比
 *      Bsp_Protect_*      → 故障判断/清故障
 * ========================================================================== */

FOC_t g_foc; /* 全局共享数据（VOFA/调试都看它） */

static FOC_State_t s_state = FOC_STATE_IDLE;
static float s_open_elec_rad = 0.0f; /* 开环虚拟电角度 */
static uint16_t s_align_ms = 0U;     /* 对齐计时 */

//note : FOC 初始化：清共享数据、清 PI、进入 IDLE
//need : 必须在 BSP_Init() 之后调用（BSP 的电流/角度要先就绪）
//return : none
//TODO : 你来实现（至少清 g_foc 和 PI，状态置 IDLE）
void FOC_Init(void)
{
    s_state = FOC_STATE_IDLE;
    s_open_elec_rad = 0.0f;
    s_align_ms = 0U;
    FOC_Loop_Init();
}

//note : 10kHz 电流环入口：读反馈 → 按状态算出占空比 → 写 PWM
//need : 在 ADC 注入完成中断里调（main.c 已接）
//return : none
//TODO : 你来实现（建议结构：
//        1) 读 Bsp_Adc_GetCur()->ia/ib/ic 和 Bsp_Enc_Get()->elec_rad
//        2) 按 s_state 分支：IDLE 关桥 / OPEN 用 s_open_elec_rad 开环
//           / CLOSED 走 Clarke→Park→PI→InvPark
//        3) FOC_SVPWM() → Bsp_Pwm_SetDuty()）
void FOC_CurrentLoop(void)
{
    /* 骨架占位：保证"未使用"的静态变量不报警；实现时删掉这两行 */
    (void)s_open_elec_rad;
}

//note : 1kHz 任务入口：状态机推进 + 速度环 + 保护联动
//need : 在 main.c 的 1kHz 软定时里调
//return : none
//TODO : 你来实现（建议结构：
//        1) 若 Bsp_Protect_IsTripped() → s_state = FAULT，关桥
//        2) 按 s_state 推进 ALIGN 计时 / OPEN / CLOSED 速度环
//        3) CLOSED 时 s_iq_ref = FOC_Loop_Speed(...)）
void FOC_Task1kHz(void)
{
    /* 骨架占位：保证"未使用"的静态变量不报警；实现时删掉这一行 */
    (void)s_align_ms;
}

//note : 启动：IDLE → ALIGN（开始零点标定）
//need : 无
//return : none
//TODO : 你来实现
void FOC_Start(void)
{
    s_align_ms = 0U;
    s_state = FOC_STATE_ALIGN;
}

//note : 停机：任意状态 → IDLE（关桥 + 清积分）
//need : 无
//return : none
//TODO : 你来实现
void FOC_Stop(void)
{
    Bsp_Pwm_OutputEnable(0U);
    FOC_Loop_Reset();
    s_state = FOC_STATE_IDLE;
}

//note : 切到开环旋转（验证功率级/相序/方向）
//need : 无
//return : none
//TODO : 你来实现
void FOC_SetOpenLoop(void)
{
    s_state = FOC_STATE_OPEN;
}

//note : 切到闭环（电流环 + 速度环）
//need : 零点已标定（ALIGN 完成）
//return : none
//TODO : 你来实现
void FOC_SetClosedLoop(void)
{
    s_state = FOC_STATE_CLOSED;
}

//note : 清故障：FAULT → IDLE（同时清 BSP 故障位）
//need : 故障原因已消失
//return : none
//TODO : 你来实现
void FOC_ClearFault(void)
{
    Bsp_Protect_Clear();
    s_state = FOC_STATE_IDLE;
}

//note : 设速度给定（闭环用，内部按 FOC_RPM_MAX 限幅）
//need : rpm 目标转速
//return : none
//TODO : 你来实现
void FOC_SetSpeed(float rpm)
{
    (void)rpm;
}

//note : 设力矩给定（直接给 iq_ref，按 FOC_IQ_MAX 限幅）
//need : iq_a q 轴电流 A
//return : none
//TODO : 你来实现
void FOC_SetTorque(float iq_a)
{
    (void)iq_a;
}

FOC_State_t FOC_GetState(void)
{
    return s_state;
}

//note : 取状态名字符串（调试打印/VOFA 上传用）
//need : 无
//return : "IDLE"/"ALIGN"/"OPEN"/"CLOSED"/"FAULT"
const char *FOC_GetStateName(void)
{
    switch (s_state)
    {
    case FOC_STATE_IDLE:
        return "IDLE";
    case FOC_STATE_ALIGN:
        return "ALIGN";
    case FOC_STATE_OPEN:
        return "OPEN";
    case FOC_STATE_CLOSED:
        return "CLOSED";
    case FOC_STATE_FAULT:
        return "FAULT";
    default:
        return "?";
    }
}
