# 第 03 课：逐份追踪你的启动汇编和链接脚本

前置：第 02 课。本课区分两份工程的当前事实与下一课的修改目标。

## 阅读你的文件

| 内容 | otatest | bootloader |
|---|---|---|
| 启动汇编 | [startup_stm32f407xx.s](../../otatest/startup_stm32f407xx.s) | [startup_stm32f40_41xxx.s](../../bootloader/startup/startup_stm32f40_41xxx.s) |
| 链接脚本 | [STM32F407xx_FLASH.ld](../../otatest/STM32F407xx_FLASH.ld) | [bootloader.ld](../../bootloader/linker/bootloader.ld) |
| 系统初始化 | [system_stm32f4xx.c](../../otatest/Core/Src/system_stm32f4xx.c) | [system_stm32f4xx.c](../../bootloader/User/Src/system_stm32f4xx.c) |
| main | [Core/Src/main.c](../../otatest/Core/Src/main.c) | [User/Src/main.c](../../bootloader/User/Src/main.c) |

## 首先找出实际顺序

当前 otatest 启动汇编先调用 SystemInit，再复制 .data、清零 .bss，之后调用 __libc_init_array 和 main。HAL 的主系统时钟配置在 main 中的 SystemClock_Config 执行。

当前 bootloader 启动汇编先复制 .data、清零 .bss，再调用 SPL SystemInit 配置时钟，随后运行库初始化和 main。

不要把“通常启动顺序”背成一份固定答案；分别从两份汇编的 bl 指令和循环找到依据。SystemCoreClockUpdate 是更新软件记录，不是重新配置硬件时钟。

## 当前地址，并非已经配成一对

otatest 仍从 0x08000000 链接，占用整片 512 KiB；bootloader 从同一基址链接，但只允许 32 KiB。它们当前不能直接一起作为 Boot/App 下载。

下一课才把原 otatest 改成从 0x08008000 起的 480 KiB App。bootloader 的 boot_config.h 和 bootloader.ld 已按这处分界配置。

普通 SRAM 为 0x20000000～0x2001FFFF，初始 MSP 可以是尾后地址 0x20020000。otatest 额外映射 CCM，bootloader 暂未映射。CCM 初始化是否完成要看启动代码，不只看 ld 中是否有 .ccmram。

## 实验 A：读现有 ELF

在两个原工程目录分别运行（已有 ELF 才读取，不把旧产物当最新代码）：

```powershell
arm-none-eabi-readelf -S -l build/Debug/otatest.elf
arm-none-eabi-nm -n build/Debug/otatest.elf
```

```powershell
arm-none-eabi-readelf -S -l build/Debug/bootloader.elf
arm-none-eabi-nm -n build/Debug/bootloader.elf
```

定位 .isr_vector、.text、.data、.bss、_sidata、_sdata、_edata、_sbss、_ebss、_estack。解释 .data 的运行地址和加载地址为什么不同。

## 实验 B：观察数据初始化

在其中一个原 main 文件的用户位置添加 volatile 全局变量：

```c
volatile uint32_t g_initialized = 0x12345678U;
volatile uint32_t g_zeroed;
```

让循环使用它们，避免优化删除。在 Reset_Handler 和 main 处观察值，并结合当前这份启动文件的真实顺序解释。

## 实验 C：对比链接描述

找出 MEMORY、KEEP、ALIGN、LOADADDR、NOLOAD、ASSERT 分别承担什么职责。bootloader 的边界断言为什么能发现 32K/64K 不一致？otatest 的 TLS/CCM 段分别需要什么初始化支持？

只有读懂原来的手动分区后，再对照学习目录中的 config/flash_layout.json 与 FlashLayout.cmake，理解如何集中生成配置。此 JSON 不会自动修改原工程。

## 验收

能画出两个不同的启动流程；能指出当前两个向量表会占用同一 Flash 地址；能解释链接器决定地址、启动代码执行初始化、CMake选择二者三种职责。
