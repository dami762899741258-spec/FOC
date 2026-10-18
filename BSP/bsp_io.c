#include "bsp_io.h"
#include "main.h"

/* ============================================================================
 *  板载 IO（指示灯）
 * ========================================================================== */

void Bsp_Io_Init(void)
{
    Bsp_Io_Led1(0U);
    Bsp_Io_Led2(0U);
}

void Bsp_Io_Led1(uint8_t on)
{
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, (on != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void Bsp_Io_Led2(uint8_t on)
{
    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, (on != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
