#ifndef BSP_H
#define BSP_H

#include <stdint.h>

/* ============================================================================
 *  BSP 总入口（复刻自 FOC-3.0t 的 BSP.c / BSP.h）
 *  ---------------------------------------------------------------------------
 *  上层（main.c / APP）只需要这三个函数：
 *      BSP_Init()       上电初始化（顺序不能乱，见 bsp.c 注释）
 *      BSP_Task1kHz()   主循环 1kHz 软定时里调
 *      BSP_Task12k5Hz() TIM2 更新中断里调（编码器采样）
 *
 *  各子模块头文件全部在这里集中包含，main.c 只 include "bsp.h" 就够了。
 * ========================================================================== */

#include "bsp_config.h"
#include "bsp_tim.h"
#include "bsp_io.h"
#include "bsp_led.h"
#include "bsp_pwm.h"
#include "bsp_adc.h"
#include "bsp_encoder.h"
#include "bsp_protect.h"

/* BSP 一次性初始化 */
void BSP_Init(void);

/* 1kHz 任务：心跳灯 / 母线电压 / 慢速保护 */
void BSP_Task1kHz(void);

/* 12.5kHz 任务：ABZ 编码器（角度/转速） */
void BSP_Task12k5Hz(void);

#endif /* BSP_H */
