#include "bsp.h"
#include "main.h"
#include "tim.h"

/* ============================================================================
 *  BSP 初始化顺序（顺序不能乱，原因写在每一步后面）
 * ========================================================================== */
//note : BSP 一次性初始化（顺序不能乱：DWT→灯→PWM→ADC→编码器→保护→启动 TIM2）
//need : CubeMX 的 MX_xxx_Init() 已经全部跑完
//return : none
void BSP_Init(void)
{
    /* 1. DWT 计时：测速窗口、零漂超时、1kHz 软定时都靠它 */
    Bsp_Time_Init();

    /* 2. IO / 指示灯 */
    Bsp_Io_Init();
    Bsp_Led_Init();

    /* 3. PWM：启动 TIM1 计数 + CH4(ADC 触发源)
     *    —— 六路桥臂这时仍然是关断的，必须显式 Bsp_Pwm_OutputEnable(1) 才有输出 */
    Bsp_Pwm_Init();

    /* 4. ADC：运放 + 自校准 + 注入转换启动 + 零漂平均
     *    必须在 PWM 之后：注入组的触发源是 TIM1_CC4，定时器不跑就没数据；
     *    零漂校准时桥臂关断、电机静止，测到的就是纯零漂 */
    Bsp_Adc_Init();
    Bsp_Adc_Start();
    if (Bsp_Adc_CalibrateOffset() == 0U)
    {
        /* 校准失败（多半是注入触发没来）：电流不可信 → 快闪提示 */
        Bsp_Led1_Blink(100U);
    }
    else
    {
        Bsp_Led1_Blink(BSP_LED_HEARTBEAT_MS); /* 1Hz 心跳 */
    }

    /* 5. ABZ 编码器（TIM3 启动 + 清累计量；Z 的 EXTI 由 CubeMX 的 gpio.c 打开） */
    Bsp_Enc_Init();

    /* 6. 软件保护 */
    Bsp_Protect_Init();

    /* 7. TIM2：12.5kHz 编码器节拍（中断里只做读计数/差分/测速） */
    (void)HAL_TIM_Base_Start_IT(&htim2);
}

/* ============================================================================
 *  1kHz 任务
 * ========================================================================== */
//note : 1kHz 任务：心跳灯 + 母线电压采样 + 慢保护
//need : main.c 的 1kHz 软定时里调用
//return : none
void BSP_Task1kHz(void)
{
    Bsp_Led_Task();       /* 心跳灯 */
    Bsp_Adc_SampleAux();  /* 母线电压（ADC2 规则组软件触发） */
    Bsp_Protect_Update(); /* 过压/欠压/编码器离线 */
}

/* ============================================================================
 *  12.5kHz 任务（TIM2 更新中断）
 * ========================================================================== */
//note : 12.5kHz 任务：ABZ 编码器采样（角度/转速）
//need : TIM2 更新中断里调用（main.c 转发）
//return : none
void BSP_Task12k5Hz(void)
{
    Bsp_Enc_Update(); /* 读 TIM3->CNT → 多圈/机械角/电角度/转速 */
}
