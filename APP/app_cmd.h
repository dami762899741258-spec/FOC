#ifndef APP_CMD_H
#define APP_CMD_H

#include <stdint.h>

/* ============================================================================
 *  APP 层：串口命令解析（骨架）
 *  ---------------------------------------------------------------------------
 *  沿用样例的帧格式（方便复用你的上位机脚本）：
 *      !pos_ref:3.14\n
 *      !spd_ref:500\n
 *      !iq_ref:0.5\n
 *      !start:1\n / !start:0\n
 *  流程：
 *      HAL_UARTEx_RxEventCallback → 只置标志（main.c 已做）
 *      App_Cmd_Task1kHz          → 判标志 → 解析 → 重新挂 DMA 接收
 * ========================================================================== */

void App_Cmd_Init(void);

/* 1kHz 调一次：处理"收到一帧"标志 → 解析 → 重新启动 ReceiveToIdle_DMA */
void App_Cmd_Task1kHz(void);

/* 直接解析一帧（Task 里调用；也可单独测试） */
void App_Cmd_OnRxFrame(const uint8_t *buf, uint16_t len);

#endif /* APP_CMD_H */
