# 第 04 课：让你的 bootloader 启动你的 otatest

前置：第 03 课。此次课程整理没有自动修改原工程；本课是接下来由你逐步完成的实验。

## 先看已有交接代码

阅读原 bootloader 的 [boot_jump.c](../../bootloader/User/Src/boot_jump.c)、[boot_jump.s](../../bootloader/startup/boot_jump.s)、[boot_config.h](../../bootloader/User/Inc/boot_config.h) 和 [main.c](../../bootloader/User/Src/main.c)。

Boot_JumpToApp 读取 0x08008000 的初始 MSP 和第二个字的 Thumb 复位入口。当前 Boot main 已在心跳初始化前尝试交接；App 不合法时才留在 Boot 闪灯。

检查普通 SRAM 边界和 8 字节对齐、Thumb 位、复位入口范围。交接前停止 SysTick、关闭/清除外部 IRQ、处理时钟，最后由独立汇编切换 MSP。它并不等同于硬件复位，也不证明镜像 CRC 正确。

## 实验 A：把原 otatest 链接成 App

修改原 otatest/STM32F407xx_FLASH.ld 的 FLASH 区：

```ld
FLASH (rx) : ORIGIN = 0x08008000, LENGTH = 480K
```

RAM/CCM 的声明不因这次 Flash 分区调整自动改变。修改后需要重新构建 App，不能只把旧 BIN 换地址下载。

## 实验 B：把偏移送到真正编译 SystemInit 的目标

当前原 otatest 的 USER_VECT_TAB_ADDRESS 没有启用。system 文件里的 VECT_TAB_OFFSET 有 #if !defined 保护，因此可由编译定义设置。

在原 otatest 根 CMakeLists 的 add_subdirectory(cmake/stm32cubemx) 之后，为真正包含 system_stm32f4xx.c 的目标增加：

```cmake
target_compile_definitions(STM32_Drivers PRIVATE
    USER_VECT_TAB_ADDRESS
    VECT_TAB_OFFSET=0x00008000U
)
```

只给 otatest 可执行目标加 PRIVATE 宏，不会反向传到已经独立编译的 STM32_Drivers。用 compile_commands.json 中 system 文件的实际命令验证，而不是只看编辑器宏颜色。

原 Bootloader 的向量偏移保持 0。下一阶段再讨论如何把这处常量与链接区域统一生成。

## 实验 C：让原 App 有可观察行为

在 otatest 的 USER CODE 区添加自己的身份计数或区别于 Boot 的 GPIO 心跳；HAL 初始化和 SystemClock_Config 保留原流程。GPIO 以板卡原理图为准。

分别在两个原目录由你配置、构建 Debug。读两个 ELF，确认向量表分别是 0x08000000 和 0x08008000，初始 MSP 与入口合法。

原工程没有 JumpDebug 预设；JumpDebug 只属于学习目录中的模块化对照代码。

## 实验 D：下载并验证两个原镜像

使用现有 J-Link/Cortex-Debug 配置，明确选择原 bootloader 或原 otatest 的 ELF。双镜像加载时指定两份正确 ELF，并核对工具没有整片擦除另一份镜像。

进入 Boot_JumpToApp 前观察 0x08008000 和 0x08008004。进入原 App main 后观察 MSP、VTOR=0x08008000、PRIMASK 和 SysTick。用 App 计数/心跳证明其中断正常。

退出调试后硬件复位，确认仍从 Boot 进入 App。调试器直接设置 PC 进入 App 不能代替这一项。

## 对照扩展

本课原工程完成后，再打开学习目录的 ota-course.code-workspace，研究模块化示例如何用两个目标、同一链接模板和单一 JSON 配置减少重复。其双镜像调试配置见 ../docs/debugging.md，不要混用三个目录的产物。

## 验收

能解释两个原工程分别改了什么、为什么只改 App ld 还不足够；能证明 App 中断和硬件复位启动正常；能说明后续加入 OTA 状态后为什么要把自动跳转放到状态判断之后。
