# BSP 层复刻说明（FOC-3.0t → FOC_3.6 / ABZ 版）

> 本文件记录：把旧工程 `FOC-3.0t` 的 BSP 层复刻到本工程时，**哪些是原样搬的、
> 哪些因为新硬件必须改、哪些顺手做了优化**。写 FOC/APP 前请先读一遍。

---

## 0. 分层现状

```
L4 集成层  Core/Src/main.c        ← 只留 BSP_Init() / 1kHz 调度 / 4 个中断转发
L1 BSP 层  BSP/                   ← 本次已完成（复刻旧工程 8 个模块）
L2 FOC 层  FOC/                   ← 待实现（目录已建）
L3 APP 层  APP/                   ← 待实现（目录已建）
L0 平台层  Core/Src/{adc,tim,...} ← CubeMX 生成，只填 USER CODE
```

main.c 里已留 3 个挂载点（注释 `← 下一层`）：
`FOC_CurrentLoop()`（10kHz 中断）、`FOC_Task1kHz()`、`App_Motor_Task1kHz()` / `App_Cmd_OnRxFrame()`。

---

## 1. 文件对照

| 旧工程（FOC-3.0t / SPI 版） | 新工程（FOC_3.6 / ABZ 版） | 处理方式 |
|---|---|---|
| `MDK-ARM/bsp_config.h` | `BSP/bsp_config.h` | 🔧 重写数值（硬件全变了） |
| `MDK-ARM/bsp_tim.c/.h` | `BSP/bsp_tim.c/.h` | ✅ 直接移植（DWT 与硬件无关） |
| `MDK-ARM/bsp_io.c/.h` | `BSP/bsp_io.c/.h` | 🔧 移植，单 RGB 灯 → 双 LED（LED1=PA15 / LED2=PB9） |
| `MDK-ARM/bsp_led.c/.h` | `BSP/bsp_led.c/.h` | 🔧 移植 + **修闪烁 bug** |
| `MDK-ARM/bsp_pwm.c/.h` | `BSP/bsp_pwm.c/.h` | 🔧 移植，40kHz 边沿对齐 → 10kHz 中心对齐 |
| `MDK-ARM/bsp_adc.c/.h` | `BSP/bsp_adc.c/.h` | 🔁 接口不变、**实现重写**（扫描+DMA → 注入组 JDR 直读） |
| `MDK-ARM/bsp_encoder.c/.h` | `BSP/bsp_encoder.c/.h` | 🔁 接口不变、**实现重写**（MT6701 SPI → MT6835 ABZ） |
| `MDK-ARM/bsp_protect.c/.h` | `BSP/bsp_protect.c/.h` | 🔧 移植，删过温、编码器离线改可选 |
| `MDK-ARM/BSP.c`（+README 里的 BSP.h） | `BSP/bsp.c` + `BSP/bsp.h` | 🔧 移植，新增 12.5kHz 任务、Init 顺序改适配 |

Keil 工程已加 `Application/BSP` 组（8 个 `.c`）与 include path `../BSP`，可直接编译。

---

## 2. 逐模块改动与原因

### 2.1 `bsp_config.h`（数值全改）

| 宏 | 旧值 | 新值 | 原因 |
|---|---|---|---|
| `BSP_PWM_ARR` | 4250 | **7999** | 频率 40kHz → 10kHz，中心对齐 |
| `BSP_PWM_FREQ_HZ` | 40000 | **10000** | 同上（160MHz/(2×8000)） |
| `BSP_MAX_DUTY` | 0.85 | **0.95** | 中心对齐 10kHz 的顶部采样窗口比 40kHz 宽裕 |
| 电流采样 | ADC1 3ch 扫描 + DMA | **ADC1/2 注入组 2+1** | 新板是三运放输出，硬件触发采样点 |
| 编码器 | MT6701 SPI 14bit / 16384 | **MT6835 ABZ 16bit / 65536** | 新板用 A/B/Z |
| 极对数 | 10 | **11** | 新电机 |
| 母线 | 24V 系统 / PC5 | **待实测 / PA0** | 新板分压脚不同，默认仍给 24V、×11 |
| NTC/过温 | 有 | **删除** | 新板没有 NTC 采样通道 |
| SPI 编码器参数 | `BSP_ENC_SPI_*` / CS | **删除** | 不再用 SPI |
| 死区寄存器值 | `BSP_DEAD_TIME_REG=0x95` | **删除** | 死区 120 已在 CubeMX 里配 |
| 新增 | — | `BSP_MOTOR_POLE_PAIRS`、`BSP_ENC_RPM_PER_CNT`、`BSP_ENC_SPEED_DECIM`、`BSP_ADC_OFFSET_TIMEOUT_MS`、`BSP_USART_*`、`BSP_LED_HEARTBEAT_MS` | 新实现需要 |

### 2.2 `bsp_tim` ✅ 无改动
纯 DWT 时基，与 MCU/外设无关；函数名统一为 `Bsp_Time_DelayUs`（旧头文件/实现曾不一致）。

### 2.3 `bsp_io` / `bsp_led`
- 旧：一块 RGB 灯（PA5）；新：**LED1=PA15（心跳）+ LED2=PB9（状态）**，接口拆成 `Bsp_Io_Led1/Led2`。
- **优化①**：修掉旧 `bsp_led.c` 的 flashing bug —— 旧实现 `s_on` 赋过一次后从不翻转，灯只会常亮；现在每半周期取反。
- **优化②**：`Bsp_Led1_Blink(0)` 的语义统一为"关闭常灭"，避免旧实现里局部关闭的歧义。

### 2.4 `bsp_pwm`（薄封装，逻辑几乎没变）
- 旧版在代码里配 CH4 触发点并写死区；新工程**死区/互补/中心对齐/CH4 全在 CubeMX**，本模块只做：
  `Init()` = 启动 TIM1 base + CH4（此时六路桥臂仍关断，和样例 `main.c` 一致）。
- **优化③**：旧版用「读 MOE 位」判断输出状态 —— 不可靠，因为启动 CH4 时 HAL 也会置 MOE；
  新版用模块内 `s_out_en` 标志。
- `SetDuty(0~1)`：中心对齐换算 `CCR = duty × (ARR+1)`，并内部按 `BSP_MAX_DUTY` 限幅。

### 2.5 `bsp_adc`（接口不变，实现重写）
| 项 | 旧（ADC1 扫描 + DMA） | 新（注入组） |
|---|---|---|
| 读电流 | DMA 缓冲 `buf[3]` | `ADC1->JDR1`(A) / `ADC1->JDR2`(C) / `ADC2->JDR1`(B) |
| 触发 | TIM1_TRGO2 下降沿 | **T1_CC4 上升沿**（CubeMX 配） |
| 采样时刻 | 40kHz 零矢量 | 10kHz 计数顶点（三相下管全开） |
| 零漂 | `Bsp_Adc_CalibrateOffset()` 阻塞读 DMA | 在 10kHz 注入中断里平均，`CalibrateOffset()` 只等完成+超时 |
| 母线 | ADC2 轮询（NTC 共用） | ADC2 规则组软件触发轮询（**不再有 NTC**） |

- 接口 `Bsp_Adc_Cur_t / Aux_t / GetCur / GetAux / UpdateCurrent / SampleAux / RawToAmp` 全部保留，
  写 FOC 时用法不变。
- **优化④**：保留 `valid` 门控 —— 零漂没完成前电流不可信，`Bsp_Protect_CheckFast()` 直接跳过，避免上电误跳闸。
- 母线用 HAL 的"规则组可单独启动"特性（注入组在跑也不冲突），不需要改 CubeMX。

### 2.6 `bsp_encoder`（改动最大）
| 项 | 旧（SPI 绝对角） | 新（ABZ） |
|---|---|---|
| 读角度 | SPI1 读 MT6701 24bit 帧取 14bit | **读 `TIM3->CNT`（16bit）** |
| 圈数 | 由 raw 跳变判断 | **差分 `(int16_t)(now-last)` 自动处理 65535↔0 回绕** |
| 角度 | raw/16384×2π | `cnt/65536×2π`（多圈累计计数） |
| 测速 | 固定窗口 + 低通 | **12.5kHz 周期累加（每 10 次算一次）+ 低通** |
| 零点 | `SetElecZeroMech` + SPI 绝对角 | 保留同接口，`SetElecZeroMech` 语义不变 |
| Z 信号 | 无 | **新增 `Bsp_Enc_ZIsr()`（PB3 EXTI）、`GetZCount()/GetZRaw()`** |
| online | SPI 通信成功 | **见过 Z 即在线**（见 TODO） |

- **优化⑤**：把"零点标定策略"留在 FOC 层，BSP 只提供 `SetElecZeroMech()` 和 Z 原始数据 ——
  Z 与转子磁极的相位关系必须实测，不能写死在 BSP 里。
- **TODO**：闭环稳定后可加"给了输出但计数长时间不变"的失速检测（现在 online 只是"见过 Z"）。

### 2.7 `bsp_protect`
- 保留：过流（10kHz 快检，立即关桥）/ 过压 / 欠压（1kHz 延时跳闸）/ 故障位 / **唯一动作 = 关桥自由停车**。
- 删除：过温 OT（板无 NTC）。
- 改动：编码器离线保护改成编译开关 `BSP_PROT_ENC_ENABLE`，**默认 0** —— 因为 ABZ 的 `online` 定义为"见过 Z"，
  开环寻零阶段还没见过 Z，一打开就会误跳闸。闭环稳定后再改成 1。
- **优化⑥**：`Bsp_Protect_Update()` 在输出关断时只清计时器不跳闸（沿用旧工程的好设计，调试更友好）。

### 2.8 `bsp.c` / `bsp.h`
- 新增 `BSP_Task12k5Hz()`（TIM2 中断里调，做编码器采样），旧工程只有 1kHz 任务。
- **优化⑦**：旧工程里 `HAL_OPAMP_Start / ADC 校准 / TIM1 启动 / TIM3 启动 / TIM2 启动` 散落在 `main.c`；
  本次**全部下沉进 BSP**，`main.c` 只剩 `BSP_Init()` 一行。
- 初始化顺序（顺序不能乱，代码里有注释）：
  DWT → IO/LED → PWM(启动 TIM1) → ADC(运放/校准/注入/零漂) → 编码器 → 保护 → 启动 TIM2。

---

## 3. 同名接口的"语义变化"（写 FOC/APP 时注意）

| 接口 | 旧语义 | 新语义 |
|---|---|---|
| `Bsp_Adc_UpdateCurrent()` | 40kHz TIM1 中断里读 DMA 缓冲 | **10kHz ADC 注入中断里读 JDR** |
| `Bsp_Enc_Update()` | 1kHz 任务里读 SPI | **12.5kHz TIM2 中断里算 CNT** |
| `Bsp_Pwm_GetArr()` | 返回 4250 | 返回 **7999**（中心对齐，`duty = CCR/(ARR+1)`） |
| `BSP_MAX_DUTY` | 0.85（40kHz 边沿对齐） | **0.95**（10kHz 中心对齐） |
| `Bsp_Enc_Get()->raw` | 14bit 0..16383 | **16bit TIM3->CNT 0..65535** |
| `Bsp_Enc_Get()->online` | SPI 通信正常 | **见过 Z 信号** |

---

## 4. 编译状态

Keil `UV4.exe -b`（AC5 V5.06 update 5）：

```
Program Size: Code=23784 RO-data=568 RW-data=76 ZI-data=2772
"FOC_3.6\FOC_3_6.axf" - 0 Error(s), 0 Warning(s).
```

---

## 5. 待你按实物确认的 6 个参数（都在 `bsp_config.h`）

1. `BSP_SHUNT_OHM` / `BSP_AMP_GAIN` —— 采样电阻与运放增益（算 `A_PER_LSB`）
2. `BSP_VBUS_DIV_RATIO` / `BSP_VBUS_NOMINAL_V` —— 且确认母线分压真的接在 **PA0(ADC2_IN1)**
3. `BSP_MOTOR_POLE_PAIRS = 11`
4. `BSP_ENC_DIR` —— 手转电机看 `speed_rpm` 符号再定 `+1/-1`
5. **ABZ 一圈是否真 65536 计数** —— MT6835 的 ABZ 分辨率要配成 16384 线
6. `BSP_PROT_OC_A` —— 先降到 1~2A 调试

---

## 6. 下一层（未做）

- `FOC/`：`foc_math / foc_filter / foc_svpwm / foc_loop / foc_state`（可参考旧工程同名头文件，`FOC_TS_I` 改成 100µs）
- `APP/`：`app_motor / app_vofa / app_cmd`
- `main.c` 里 3 个 TODO 挂载点接上即可。
