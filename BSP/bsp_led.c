#include "bsp_led.h"
#include "bsp_io.h"
#include "bsp_config.h"

/* ============================================================================
 *  指示灯闪烁（每 1ms 调一次 Task）
 *  ★ 旧工程的坑：s_on 赋值后从不翻转 → 灯只会常亮。这里每半周期取反。
 * ========================================================================== */

static uint16_t s_half_period_ms = 0U; /* 0 = 关闭闪烁 */
static uint16_t s_cnt = 0U;            /* 半周期计数 */
static uint8_t s_on = 0U;              /* 当前灯状态 */

void Bsp_Led_Init(void)
{
    s_half_period_ms = 0U;
    s_cnt = 0U;
    s_on = 0U;
    Bsp_Io_Led1(0U);
}

void Bsp_Led1_On(void)
{
    s_half_period_ms = 0U;
    Bsp_Io_Led1(1U);
}

void Bsp_Led1_Off(void)
{
    s_half_period_ms = 0U;
    Bsp_Io_Led1(0U);
}

//note : 设置心跳灯闪烁半周期；进来先点亮，传 0 表示关闭闪烁并熄灭
//need : half_period_ms 半周期，单位 ms
//return : none
void Bsp_Led1_Blink(uint16_t half_period_ms)
{
    if (half_period_ms == 0U)
    {
        Bsp_Led1_Off();
        return;
    }
    s_half_period_ms = half_period_ms;
    s_cnt = 0U;
    s_on = 1U;
    Bsp_Io_Led1(s_on);
}

//note : 1kHz 调一次的闪灯推进：累加到半周期就翻转一次 LED 电平
//need : 无（半周期由 Bsp_Led1_Blink 设好，0 表示不闪）
//return : none
void Bsp_Led_Task(void)
{
    if (s_half_period_ms == 0U)
    {
        return;
    }

    s_cnt++;
    if (s_cnt >= s_half_period_ms)
    {
        s_cnt = 0U;
        s_on = (s_on == 0U) ? 1U : 0U; /* 翻转 */
        Bsp_Io_Led1(s_on);
    }
}
