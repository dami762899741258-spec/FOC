#include "bsp_tim.h"
#include "main.h"

/* ============================================================================
 *  DWT 微秒时基（拷自 FOC-3.0t 工程，与具体外设无关）
 * ========================================================================== */

void Bsp_Time_Init(void)
{
    /* 1. 使能跟踪调试模块（不开这个，DWT 根本不计数） */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    /* 2. 计数器清零 */
    DWT->CYCCNT = 0U;

    /* 3. 启动计数 */
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

//note : 取从上电到现在的微秒时间戳（DWT 周期计数换算）
//need : 无（依赖 Bsp_Time_Init 已使能 DWT 计数）
//return : 微秒数；32 位会回绕（约 4295s），差值必须用无符号减法
uint32_t Bsp_Time_MicroSec(void)
{
    uint32_t cycle_per_us = SystemCoreClock / 1000000U; /* 160MHz → 160 cycle/µs */

    if (cycle_per_us == 0U)
    {
        return 0U; /* 避免除以零 */
    }

    return DWT->CYCCNT / cycle_per_us;
}

//note : 微秒级忙等延时（轮询 DWT 计数差值，不占中断/定时器）
//need : us 延时长度，建议 ≥1µs；极短延时直接 __NOP 更快
//return : none
void Bsp_Time_DelayUs(uint32_t us)
{
    uint32_t cycle_per_us = SystemCoreClock / 1000000U;
    uint32_t start;
    uint32_t need;

    if (cycle_per_us == 0U)
    {
        return;
    }

    need = us * cycle_per_us;
    start = DWT->CYCCNT;
    while ((DWT->CYCCNT - start) < need)
    {
        /* 等待 */
    }
}
