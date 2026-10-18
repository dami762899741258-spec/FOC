#include "app_cmd.h"
#include "app_motor.h"
#include "usart.h"

/* ============================================================================
 *  串口命令解析（骨架：由你实现）
 *  ---------------------------------------------------------------------------
 *  关键点（样例里踩过的坑）：
 *    1) IDLE 事件后 DMA 已经停了，处理完必须重新 HAL_UARTEx_ReceiveToIdle_DMA
 *    2) 解析在 1kHz 任务里做，不要在中断里做
 * ========================================================================== */

//note : 初始化命令解析（清标志）
//need : 无
//return : none
//TODO : 你来实现
void App_Cmd_Init(void)
{
}

//note : 1kHz 任务：若 UART_HandleFlag=1 → 解析一帧 → 清标志 → 重新挂 DMA
//need : main.c 的中断回调已置 UART_HandleFlag / UART_RxCount
//return : none
//TODO : 你来实现
void App_Cmd_Task1kHz(void)
{
}

//note : 解析一帧文本命令，调用 App_Motor_xxx 执行
//need : buf 帧首地址；len 帧长
//return : none
//TODO : 你来实现（可用 strstr/strtof，帧头 '!' 帧尾 0x0A）
void App_Cmd_OnRxFrame(const uint8_t *buf, uint16_t len)
{
    (void)buf;
    (void)len;
}
