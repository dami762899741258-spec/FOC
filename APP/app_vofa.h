#ifndef APP_VOFA_H
#define APP_VOFA_H

#include <stdint.h>

/* ============================================================================
 *  APP 层：VOFA+ 上位机上传（骨架，JustFloat 协议）
 *  ---------------------------------------------------------------------------
 *  USART1 = 921600 8N1，DMA 发送（bsp 里 usart.c 已配好 DMA）
 *  JustFloat 帧格式：float 数组（小端）+ 帧尾 0x00 0x00 0x80 0x7F
 *  ========================================================================== */

void App_Vofa_Init(void);

/* 1kHz 调一次：把关心的通道打成 JustFloat 帧发出去（建议 100Hz~1kHz） */
void App_Vofa_Task1kHz(void);

/* 通用发送：ch 通道数组，n 通道数（n ≤ 16） */
void App_Vofa_SendFloats(const float *ch, uint8_t n);

#endif /* APP_VOFA_H */
