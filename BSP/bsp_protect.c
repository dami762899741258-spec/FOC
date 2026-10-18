#include "bsp_protect.h"
#include "bsp_config.h"
#include "bsp_adc.h"
#include "bsp_pwm.h"
#include "bsp_encoder.h"
#include <math.h>

/* ============================================================================
 *  软件保护（复刻自 FOC-3.0t BSP，去掉过温、保留过流/过压/欠压/编码器）
 * ========================================================================== */

static uint16_t s_fault = BSP_FAULT_NONE;
static uint16_t s_ov_ms = 0U;
static uint16_t s_uv_ms = 0U;
#if (BSP_PROT_ENC_ENABLE != 0)
static uint16_t s_enc_ms = 0U;
#endif

void Bsp_Protect_Init(void)
{
    s_fault = BSP_FAULT_NONE;
    s_ov_ms = 0U;
    s_uv_ms = 0U;
#if (BSP_PROT_ENC_ENABLE != 0)
    s_enc_ms = 0U;
#endif
}

//note : 记录故障位并立即关断 PWM（唯一故障动作 = 六管全关自由停车）
//need : fault 故障位（BSP_FAULT_xxx，可用 |= 组合）
//return : none
void Bsp_Protect_Trip(uint16_t fault)
{
    s_fault |= fault;

    /* 唯一故障动作：关断输出 = 自由停车 */
    Bsp_Pwm_OutputEnable(0U);
}

//note : 10kHz 电流环里每拍做过流检查，任一相超阈值立即跳闸
//need : 无（零漂没完成、电流不可信时直接放行）
//return : 1 = 已跳闸（调用方应停掉本拍控制）；0 = 正常
uint8_t Bsp_Protect_CheckFast(void)
{
    const Bsp_Adc_Cur_t *c = Bsp_Adc_GetCur();

    if (c->valid == 0U)
    {
        return 0U; /* 零漂还没完成，电流不可信 */
    }

    if ((fabsf(c->ia) > BSP_PROT_OC_A) ||
        (fabsf(c->ib) > BSP_PROT_OC_A) ||
        (fabsf(c->ic) > BSP_PROT_OC_A))
    {
        Bsp_Protect_Trip(BSP_FAULT_OC);
    }

    return (s_fault != BSP_FAULT_NONE) ? 1U : 0U;
}

//note : 1kHz 慢保护：过压/欠压延时跳闸、编码器离线（可选）；输出关断时只清计时器
//need : 无
//return : none
void Bsp_Protect_Update(void)
{
    /* 没在输出时只清计时器、不跳闸（调试阶段更友好） */
    if (Bsp_Pwm_OutputIsEnabled() == 0U)
    {
        s_ov_ms = 0U;
        s_uv_ms = 0U;
#if (BSP_PROT_ENC_ENABLE != 0)
        s_enc_ms = 0U;
#endif
        return;
    }

#if (BSP_VBUS_ENABLE != 0)
    {
        const Bsp_Adc_Aux_t *a = Bsp_Adc_GetAux();

        if (a->vbus_valid != 0U)
        {
            if (a->vbus > BSP_PROT_OV_V)
            {
                s_ov_ms++;
                if (s_ov_ms >= BSP_PROT_V_TIME_MS)
                {
                    Bsp_Protect_Trip(BSP_FAULT_OV);
                }
            }
            else
            {
                s_ov_ms = 0U;
            }

            if (a->vbus < BSP_PROT_UV_V)
            {
                s_uv_ms++;
                if (s_uv_ms >= BSP_PROT_V_TIME_MS)
                {
                    Bsp_Protect_Trip(BSP_FAULT_UV);
                }
            }
            else
            {
                s_uv_ms = 0U;
            }
        }
    }
#endif

#if (BSP_PROT_ENC_ENABLE != 0)
    if (Bsp_Enc_IsOnline() == 0U)
    {
        s_enc_ms++;
        if (s_enc_ms >= BSP_PROT_ENC_TIME_MS)
        {
            Bsp_Protect_Trip(BSP_FAULT_ENC);
        }
    }
    else
    {
        s_enc_ms = 0U;
    }
#endif
}

void Bsp_Protect_Clear(void)
{
    s_fault = BSP_FAULT_NONE;
    s_ov_ms = 0U;
    s_uv_ms = 0U;
#if (BSP_PROT_ENC_ENABLE != 0)
    s_enc_ms = 0U;
#endif
}

uint16_t Bsp_Protect_GetFault(void)
{
    return s_fault;
}

uint8_t Bsp_Protect_IsTripped(void)
{
    return (s_fault != BSP_FAULT_NONE) ? 1U : 0U;
}
