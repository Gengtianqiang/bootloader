# 来源、改动和许可

原教案：用户提供的 `C:/Users/33481/Desktop/workspace/OTA/08_代码合集`，用于分析调用关系和课程设计。没有批量复制其中的 Ymodem、AES、EEPROM、W25Q、SFUD 或业务源码。

ST 库：从同一工作区现有 bootloader/Libraries 复制 CMSIS 与 SPL，来源为用户提供的 STM32F4xx_DSP_StdPeriph_Lib_V1.9.0。原版权、组件许可、Package_license.md、LICENSE.txt 均保留。它们属于 ST，不是本课程作者编写。

platform/stm32f407/system_stm32f4xx.c 与 startup/startup_stm32f40_41xxx.s 来自现有 Bootloader 的 ST 模板适配版：已有的 HSE=8 MHz PLL 适配、启动时复制/清零和 __libc_init_array 调用保留；本课程将 VECT_TAB_OFFSET 改成每个固件目标的 COURSE_VECTOR_OFFSET，保持 Boot/App 各自的向量表位置。

本课程新增：CMake 模块、分区生成、板级演示封装、App/Boot main、向量检查、交接汇编、纯状态决策示例、Python 工具和测试、课程文档与 VS Code 配置。它们是教学实现，不代表 ST 的官方 OTA 方案。

只通过目标列表编译 RCC/GPIO/misc，其他随库保留的外设驱动供后续接入。代码教学重构不改变第三方许可要求。
