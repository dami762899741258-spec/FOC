/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "opamp.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bsp.h"        /* L1 BSP：总入口 */
#include "foc_state.h"  /* L2 FOC：状态机 / 两个入口 */
#include "app_motor.h"  /* L3 APP：电机业务接口 */
#include "app_vofa.h"   /* L3 APP：VOFA+ 上传 */
#include "app_cmd.h"    /* L3 APP：串口命令 */
#include "usart.h"      /* UART_Rx_Buf / UART_RxCount / UART_HandleFlag */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static uint32_t s_tick_us = 0U; /* 1kHz 软定时基准 */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM1_Init();
  MX_OPAMP1_Init();
  MX_OPAMP2_Init();
  MX_OPAMP3_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_DAC3_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  /* ★ 四层初始化顺序：平台(MX_xxx_Init) → BSP → FOC → APP */
  BSP_Init();
  FOC_Init();
  App_Motor_Init();
  App_Vofa_Init();
  App_Cmd_Init();
  s_tick_us = Bsp_Time_MicroSec();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    {
      uint32_t now_us = Bsp_Time_MicroSec();
      if ((now_us - s_tick_us) >= 1000U) /* 1kHz 软定时 */
      {
        s_tick_us += 1000U;
        BSP_Task1kHz();        /* L1：心跳灯 / 母线 / 慢保护 */
        FOC_Task1kHz();        /* L2：状态机 / 速度环 */
        App_Motor_Task1kHz();  /* L3：业务 */
        App_Vofa_Task1kHz();   /* L3：上位机上传 */
        App_Cmd_Task1kHz();    /* L3：命令解析 */
      }
    }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 20;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/* ============================================================================
 *  中断入口 —— main.c 只做"转发一行"，逻辑都在 BSP/FOC/APP 层
 * ========================================================================== */

/* 10kHz：ADC1 注入组转换完成（电流环节拍） */
//note : 电流环节拍：更新三相电流 → 过流快检 →（下一层）FOC 电流环
//need : hadc 必须是 hadc1（只有 ADC1 的注入组开了中断）
//return : none
void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  if (hadc == &hadc1)
  {
    Bsp_Adc_UpdateCurrent();              /* 注入组 JDR → ia/ib/ic */
    if (Bsp_Protect_CheckFast() == 0U)
    {
      FOC_CurrentLoop();                  /* L2：电流环（10kHz） */
    }
  }
}

/* 12.5kHz：TIM2 更新中断（ABZ 编码器采样节拍） */
//note : 编码器节拍：转发给 BSP 采样 ABZ 角度/转速
//need : htim->Instance == TIM2
//return : none
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2)
  {
    BSP_Task12k5Hz();
  }
}

/* Z 信号上升沿 */
//note : 外部中断回调：PB3 的 Z 上升沿交给编码器 BSP 锁存（次数 + 当次 CNT）
//need : GPIO_Pin == Z_Pin
//return : none
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == Z_Pin)
  {
    Bsp_Enc_ZIsr();
  }
}

/* USART1 空闲中断 + DMA：一帧接收完成 */
//note : 串口帧回调：记录本次帧长并置标志，命令解析留给 APP 层
//need : huart->Instance == USART1
//return : none
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
  if (huart->Instance == USART1)
  {
    UART_RxCount = (uint8_t)Size;
    UART_HandleFlag = 1U;
    /* 解析不在这里做：App_Cmd_Task1kHz() 里处理并重新挂 DMA */
  }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
