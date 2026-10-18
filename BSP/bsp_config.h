#ifndef BSP_CONFIG_H
#define BSP_CONFIG_H

/* ============================================================================
 *  BSP 参数总表（全工程唯一的硬件参数源）
 *  ---------------------------------------------------------------------------
 *  目标：STM32G431CBT6（FOC_3.6）+ MT6835(ABZ) + 三运放电流采样 + USART1
 *
 *  ★★ CubeMX 里已经配好，本文件不再重复配置，只作注释备查：
 *      TIM1  : PA8/PA9/PA10 = HIN1/2/3，PB13/PB14/PB15 = LIN1/2/3
 *              中心对齐1 / PSC=0 / ARR=7999 / CKD=Div2 / RCR=1 / 死区=120
 *              CH4 = PWM Generation4 No Output，PWM2，Pulse=7998
 *              → 计数顶点附近产生 T1_CC4 上升沿，供 ADC 注入组触发
 *      ADC1  : 注入组 Rank1 = IN3(PA2) = IA、Rank2 = IN12(PB1) = IC
 *              触发 = T1_CC4 上升沿，采样 6.5 cycle
 *      ADC2  : 注入组 Rank1 = IN3(PA6) = IB（同上触发）
 *              规则组 IN1(PA0) = 母线电压分压，软件触发
 *      OPAMP : OPAMP1(PA1/2/3) = IA、OPAMP2(PA5/6/7) = IB、OPAMP3(PB0/1/2) = IC
 *      TIM3  : ABZ 编码器（PB4 = A，PB5 = B），Encoder TI1+TI2，ARR=65535
 *      EXTI3 : PB3 = Z 信号，上升沿 + 上拉
 *      TIM2  : PSC=1599 / ARR=7 → 12.5 kHz（编码器/测速节拍）
 *      USART1: PB6 = TX，PB7 = RX，921600，DMA1_Ch1 = RX / Ch2 = TX
 *      LED1  : PA15（心跳）        LED2 : PB9（状态）
 * ========================================================================== */

/* ==========================================================================
 * 1. PWM / 三相逆变桥（TIM1，中心对齐）
 * ========================================================================== */
#define BSP_PWM_ARR 7999U          /* TIM1 自动重装值（中心对齐：一个周期 = 2×(ARR+1) 计数） */
#define BSP_PWM_FREQ_HZ 10000U     /* 载波频率 = 160MHz / (2×(ARR+1)) = 10 kHz */
#define BSP_MAX_DUTY 0.95f         /* 占空比上限：给顶部零矢量采样窗口留余量 */
#define BSP_MIN_DUTY 0.00f         /* 占空比下限 */
/* 为什么留 5%：ADC 在 CH4=ARR-1 触发，此时三相下管全开（000 矢量）。
 * 若某相占空比太接近 1，它的上管会一直到计数顶点才关断 → 采样点落进死区，
 * 电流读数失真。MAX_DUTY 越大余量越小，别超过 0.97。 */

/* ==========================================================================
 * 2. 电流采样（三运放 + ADC 注入组）
 * ========================================================================== */
#define BSP_ADC_VREF 3.3f          /* ADC 参考电压 V */
#define BSP_SHUNT_OHM 0.001f       /* ★采样电阻 Ω —— 必须按实物核对 */
#define BSP_AMP_GAIN 50.0f         /* ★电流运放增益 —— 必须按实物核对 */
#define BSP_I_GAIN_A_PER_LSB \
    (BSP_ADC_VREF / 4096.0f / (BSP_SHUNT_OHM * BSP_AMP_GAIN))  /* 每 LSB 对应安培 */

#define BSP_ADC_OFFSET_TIMES 1000U /* 上电零漂平均次数（10kHz 中断里累计 → 100ms） */
#define BSP_ADC_OFFSET_TIMEOUT_MS 500U /* 等零漂校准完成的超时 */

#define BSP_ADC_IA_SIGN (+1.0f)    /* 相电流符号：实测"正转 Ia 为负"就改 -1.0f */
#define BSP_ADC_IB_SIGN (+1.0f)
#define BSP_ADC_IC_SIGN (+1.0f)

/* 三相在注入组里的顺序（对应 CubeMX 的 Rank）：
 *   ADC1 Rank1 = IA(PA2)   ADC1 Rank2 = IC(PB1)   ADC2 Rank1 = IB(PA6) */
#define BSP_ADC_IA_IDX 0U          /* 数组下标：IA */
#define BSP_ADC_IB_IDX 1U          /* 数组下标：IB */
#define BSP_ADC_IC_IDX 2U          /* 数组下标：IC */

/* ==========================================================================
 * 3. 母线电压（ADC2 规则组 IN1 / PA0，板上分压）
 * ========================================================================== */
#define BSP_VBUS_ENABLE 1          /* 1 = 采样母线电压；0 = 不采样 */
#define BSP_VBUS_NOMINAL_V 24.0f   /* ★标称母线电压 V */
#define BSP_VBUS_DIV_RATIO 11.0f   /* ★分压比 = (R上+R下)/R下 —— 必须实测校准 */
#define BSP_VBUS_GAIN_V_PER_LSB \
    (BSP_ADC_VREF / 4096.0f * BSP_VBUS_DIV_RATIO)  /* 每 LSB 对应多少伏 */
/* 校准（1 分钟）：1) 万用表量真实母线 Vbus  2) 读 Bsp_Adc_GetAux()->vbus_raw
 *                 3) BSP_VBUS_DIV_RATIO = Vbus × 4096 / (3.3 × vbus_raw)
 * ★ 分压后必须 < 3.3V，否则会打坏 ADC 引脚 */

/* ==========================================================================
 * 4. 编码器 MT6835（ABZ，TIM3 + Z 在 PB3）
 * ========================================================================== */
#define BSP_ENC_BITS 16U           /* TIM3 是 16bit 计数器 */
#define BSP_ENC_CPR 65536.0f       /* 一圈计数 = ABZ 线数 16384 × 4 */
#define BSP_ENC_DIR (+1.0f)        /* 方向：正转计数增为正；实测反了改 -1.0f */
#define BSP_ENC_SPEED_DECIM 10U    /* 每 N 次 12.5kHz 更新算一次速度 → 1250Hz */
#define BSP_ENC_SPEED_LPF 0.35f    /* 转速一阶低通 α（0~1，越大越灵敏） */
#define BSP_ENC_SPEED_MIN_RPM 5.0f /* 低于该转速按静止处理，避免零速抖动 */
/* 每次测速窗口（BSP_ENC_SPEED_DECIM 个 12.5kHz 周期）累计 1 个计数对应的 rpm */
#define BSP_ENC_RPM_PER_CNT \
    (60.0f * 12500.0f / (BSP_ENC_CPR * (float)BSP_ENC_SPEED_DECIM))

/* ==========================================================================
 * 5. 电机参数
 * ========================================================================== */
#define BSP_MOTOR_POLE_PAIRS 11U   /* ★极对数（样例电机 11 对极） */

/* ==========================================================================
 * 6. 通信（USART1 + DMA 空闲中断）
 * ========================================================================== */
#define BSP_USART_BAUD 921600U     /* ★上位机（VOFA+/串口助手）要一致 */
#define BSP_USART_RX_BUF_SIZE 64U  /* 单帧接收缓冲 */

/* ==========================================================================
 * 7. 软件保护阈值（故障动作只有一种：关断 PWM = 自由停车）
 *    调试期请先把 BSP_PROT_OC_A 降到 1~2A！
 * ========================================================================== */
#define BSP_PROT_OC_A 2.0f         /* 过流阈值 A（超了立即关断，无延时） */
#define BSP_PROT_OV_V 30.0f        /* 过压阈值 V */
#define BSP_PROT_UV_V 18.0f        /* 欠压阈值 V */
#define BSP_PROT_V_TIME_MS 200U    /* 过压/欠压持续多少 ms 才跳闸 */
#define BSP_PROT_ENC_TIME_MS 20U   /* 编码器离线持续多少 ms 才跳闸 */

/* ==========================================================================
 * 8. 指示灯
 * ========================================================================== */
#define BSP_LED_HEARTBEAT_MS 500U  /* 心跳灯半周期 ms */

#endif /* BSP_CONFIG_H */
