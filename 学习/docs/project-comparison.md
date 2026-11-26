# 先以你的 otatest 和 bootloader 为教材

以下结论来自本次整理时的实际源码。原工程会随实验演进，修改后以源码和最新产物为准。

## 两份工程分别教什么

| 项目 | otatest | bootloader |
|---|---|---|
| 定位 | CubeMX/HAL 应用基础工程 | SPL Bootloader 基础工程 |
| 根目标 | otatest | bootloader |
| main | Core/Src/main.c | User/Src/main.c |
| 驱动组织 | stm32cubemx INTERFACE + STM32_Drivers OBJECT | stm32_spl STATIC + mcu_options INTERFACE |
| 工具链 | cmake/gcc-arm-none-eabi.cmake | cmake/arm-none-eabi.cmake |
| CPU/FPU 参数 | 工具链中拼接全局 flags | mcu_options 按目标传播 |
| 链接脚本入口 | 工具链拼接 -T STM32F407xx_FLASH.ld | 固件目标 target_link_options 指定 bootloader.ld |
| 当前 Flash | 起点 0x08000000，512 KiB | 起点 0x08000000，32 KiB |
| App 基址约定 | 还未改造成偏移 App | boot_config.h 中为 0x08008000 |
| RAM | 普通 SRAM 128 KiB + CCM 64 KiB 分区 | 普通 SRAM 128 KiB |
| 时钟主要配置 | main 中 SystemClock_Config | 启动阶段 SPL SystemInit |
| 当前主流程 | HAL 初始化及应用循环 | 先尝试 App 交接，失败后心跳 |
| BIN/HEX 导出 | 当前 CMake 中未见相同 POST_BUILD | 根 CMake 中显式 objcopy |

“生成式”和“手写式”没有互斥关系：两者最终都描述源文件、目标和依赖。课程先认清现有设计，再选择重构哪些配置。

## 第一遍阅读只沿两条链

### 构建链

```text
otatest/CMakePresets.json
  → cmake/gcc-arm-none-eabi.cmake
  → 根 CMakeLists.txt
  → cmake/stm32cubemx/CMakeLists.txt
  → otatest.elf

bootloader/CMakePresets.json
  → cmake/arm-none-eabi.cmake
  → 根 CMakeLists.txt
  → Libraries/CMakeLists.txt
  → bootloader.elf → BIN/HEX
```

找到 main、启动汇编、system 文件、驱动分别由哪个目标编译。头文件路径与编译宏是否到达这些目标，要用实际命令验证。

### 运行链

```text
otatest：向量表 → Reset_Handler → SystemInit
  → data/bss 初始化 → __libc_init_array → main
  → HAL_Init → SystemClock_Config → GPIO/业务

bootloader：向量表 → Reset_Handler → data/bss 初始化
  → SystemInit → __libc_init_array → main
  → Boot_JumpToApp，或保留 Boot 心跳
```

这两份启动文件的 SystemInit 调用时机不同。学习时比较源码，不把示意顺序当作所有 STM32 工程的固定顺序。

## 目前还不能直接配成 Boot/App

两份 ld 都从 0x08000000 开始，所以分别运行正常不等于能同时安装。第四课明确改原 otatest 的 Flash 起点和向量偏移，保留原 Bootloader 的 32 KiB 区域。

特别注意：原 otatest 的 system_stm32f4xx.c 由 STM32_Drivers 编译。向量宏只加给 otatest 的 PRIVATE 设置不会反向影响该 OBJECT 目标。这正好连接第二课的依赖传播和第四课的中断实验。

## 哪些材料现在只作对照

学习目录的 CMakeLists.txt、config、firmware、platform、components 是模块化实验版本；它已经集中生成分区，并为两个镜像分别编译 system 文件。先完成原工程的前四课，再对照这些封装的用途。

原 bootloader/README.md 部分文字还保留旧的 64 KiB、未实现跳转等描述。本次整理不改原文件；分析以当前 ld、boot_config.h 和 main 为准，不能根据旧 README 推断运行行为。

05 起每课先在原工程指出当前缺少的模块，再决定新增到哪个工程、哪个目标。以原工程为主线；需要实验隔离或比较结构时再用学习目录中的独立示例。
