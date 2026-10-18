# FOC_3.6

基于 STM32G431CBTx（Cortex-M4F）的 FOC 电机控制工程，CubeMX 生成 + 分层手写代码，同时支持 Keil MDK 与 VS Code（EIDE）两套开发环境。

## 目录结构

| 目录 | 说明 |
| --- | --- |
| `Core/` | CubeMX 生成的外设初始化代码（main.c、adc/gpio/tim/dac/opamp/usart 等） |
| `Drivers/` | STM32G4xx HAL 驱动与 CMSIS |
| `BSP/` | 板级驱动层（bsp、bsp_tim、bsp_io、bsp_pwm、bsp_adc、bsp_encoder、bsp_led、bsp_protect） |
| `FOC/` | FOC 算法层（foc_math、foc_filter、foc_svpwm、foc_loop、foc_state） |
| `APP/` | 应用层（app_motor、app_vofa、app_cmd） |
| `docs/` | 分层设计说明文档 |
| `MDK-ARM/` | Keil 工程（`.uvprojx`）与 VS Code EIDE 工程（`.eide/`）、`.vscode/` 配置 |

## 编译方式

### Keil MDK

打开 `MDK-ARM/FOC_3.6.uvprojx`，按 `F7` 编译；输出在 `MDK-ARM/FOC_3.6/`。

### VS Code（EIDE，Keil 配置文件）

1. 用 VS Code 打开工作区 `MDK-ARM/FOC_3.6.code-workspace`（注意：EIDE 只在打开 `.code-workspace` 时自动加载工程）。
2. 左侧 EIDE 视图会加载 `FOC_3.6` 工程，点击状态栏 🔨 或按 `F7` 编译；输出在 `MDK-ARM/build/FOC_3.6/`。

EIDE 工具链路径在 `MDK-ARM/.vscode/settings.json` 中配置（ARMCC5 / ARMCLANG / UV4，指向本机 `E:\Keil5`）；如本机 Keil 安装路径不同，请在 VS Code 设置中修改 `EIDE.ARM.ARMCC5.InstallDirectory`、`EIDE.ARM.ARMCC6.InstallDirectory`、`EIDE.ARM.INI.Path`。

## 编译状态

- Keil MDK（ARM Compiler 5.06u5）：`0 Error(s), 0 Warning(s)`，Code=23860 RO=568 RW=84 ZI=2772
- VS Code EIDE（AC5）：`build successfully`，0 错误 0 警告，程序尺寸与 Keil 一致

## 说明

- `.cmsis/` 为 EIDE 随工程附带的 CMSIS 头文件；`build/`、`MDK-ARM/FOC_3.6/` 为编译产物，已在 `.gitignore` 中忽略。
