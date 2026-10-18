#ifndef BSP_LED_H
#define BSP_LED_H

#include <stdint.h>

/* ============================================================================
 *  指示灯 BSP —— 非阻塞闪烁
 *  ---------------------------------------------------------------------------
 *  用法：
 *      上电 / 校准失败 → Bsp_Led1_Blink(100U)  快闪
 *      正常运行       → Bsp_Led1_Blink(500U)  1Hz 心跳
 *      停机           → Bsp_Led1_On()/Off()
 *  每 1ms 调一次 Bsp_Led_Task()（放在 BSP_Task1kHz() 里）。
 * ========================================================================== */

void Bsp_Led_Init(void);

void Bsp_Led1_On(void);
void Bsp_Led1_Off(void);

/* half_period_ms = 半周期（ms），传 0 表示常灭 */
void Bsp_Led1_Blink(uint16_t half_period_ms);

/* 1kHz 周期调用 */
void Bsp_Led_Task(void);

#endif /* BSP_LED_H */
