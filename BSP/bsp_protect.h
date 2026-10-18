#ifndef BSP_PROTECT_H
#define BSP_PROTECT_H

#include <stdint.h>

/* ============================================================================
 *  软件保护 BSP
 *  ---------------------------------------------------------------------------
 *  故障动作只有一种：立即关断 PWM 输出 → 六个 MOS 全关 → 电机自由停车(coast)
 *  不做刹车、不做短路制动。
 *
 *  保护项（对照旧工程 FOC-3.0t 的 BSP）：
 *      过流 OC    —— 10kHz 电流环里每周期检查，超过阈值立即跳闸（无延时）
 *      过压 OV    —— 1kHz 检查，持续 BSP_PROT_V_TIME_MS 后跳闸
 *      欠压 UV    —— 同上
 *      编码器离线 —— 可选（见下方开关），闭环后才建议打开
 *
 *  【与旧工程的区别】新板没有 NTC 温度采样通道 → 过温 OT 保护已删除，
 *  如以后加 NTC，按旧工程 bsp_protect.c 的 OT 那段照抄即可。
 * ========================================================================== */

#define BSP_FAULT_NONE 0x0000U
#define BSP_FAULT_OC 0x0001U
#define BSP_FAULT_OV 0x0002U
#define BSP_FAULT_UV 0x0004U
#define BSP_FAULT_ENC 0x0010U

/* 编码器离线保护开关：ABZ 的 online 定义是"见过 Z 信号"，开环寻零阶段
 * 还没见过 Z，打开会误跳闸 → 默认关闭，闭环稳定后再改成 1 */
#ifndef BSP_PROT_ENC_ENABLE
#define BSP_PROT_ENC_ENABLE 0
#endif

void Bsp_Protect_Init(void);

/* 10kHz 电流环内调用：只做过流检查，跳闸返回 1 */
uint8_t Bsp_Protect_CheckFast(void);

/* 1kHz 任务内调用：过压/欠压/编码器离线 */
void Bsp_Protect_Update(void);

/* 主动跳闸（任何层都可以调，例如 FOC 检测到电流环发散） */
void Bsp_Protect_Trip(uint16_t fault);

/* 清故障（故障原因消失且操作者确认后调用） */
void Bsp_Protect_Clear(void);

uint16_t Bsp_Protect_GetFault(void);
uint8_t Bsp_Protect_IsTripped(void);

#endif /* BSP_PROTECT_H */
