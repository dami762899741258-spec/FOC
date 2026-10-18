#include "app_vofa.h"
#include "foc_config.h"
#include "usart.h"

/* ============================================================================
 *  VOFA+ 上传（骨架：由你实现）
 *  ---------------------------------------------------------------------------
 *  发送用 HAL_UART_Transmit_DMA(&huart1, buf, n)；缓冲区必须是 static，
 *  且上一帧没发完不要覆盖（HAL_UART_GetState 判断 gState）。
 * ========================================================================== */

//note : 初始化（可选：预填帧尾 magic）
//need : 无
//return : none
//TODO : 你来实现
void App_Vofa_Init(void)
{
}

//note : 1kHz 任务：把 ia/ib/ic、角度、转速、母线电压等打包发送
//need : g_foc 已有数据
//return : none
//TODO : 你来实现
//   建议通道：ch[0]=ia ch[1]=ib ch[2]=ic ch[3]=g_foc.theta
//             ch[4]=g_foc.rpm ch[5]=Bsp_Adc_GetAux()->vbus ch[6]=FOC 状态
void App_Vofa_Task1kHz(void)
{
}

//note : JustFloat 发送：float 数组 + 帧尾 0x00 0x00 0x80 0x7F
//need : ch 通道数组；n 通道数
//return : none
//TODO : 你来实现
void App_Vofa_SendFloats(const float *ch, uint8_t n)
{
    (void)ch;
    (void)n;
}
