# 三条知识线合在同一组实验中

前四课的主要教材是相邻的原 otatest 和 bootloader。先读 [现有工程对照](project-comparison.md)。学习目录中的模块化代码用于后续对照，不能替代原工程的起步实验。

| 课次 | CMake 学什么 | OTA 学什么 | 底层学什么 | 完成的可观察结果 |
|---|---|---|---|---|
| [01](../lessons/01-build-chain.md) | 对比两份原工程的 Preset、工具链和源文件入口 | Boot/App 是独立产物 | 编译、汇编、链接、ELF | 说清两个原工程各自如何构建 |
| [02](../lessons/02-targets.md) | 原工程的 OBJECT/STATIC/INTERFACE 和参数传播 | 独立镜像与共享实现 | 编译单元、符号与链接 | 在原工程正确添加模块 |
| [03](../lessons/03-startup-memory.md) | 原工程的 -T、源文件归属；再看集中配置 | 当前地址与未来分区 | 两份真实复位流程、VMA/LMA、data/bss | 用原 ELF 和源码解释初始化 |
| [04](../lessons/04-handoff.md) | 将宏送到 STM32_Drivers，重新链接原 App | 原 Boot→原 otatest | MSP、VTOR、Thumb、中断交接 | 原工程完成交接，SysTick 正常 |
| [05](../lessons/05-uart.md) | SPL 源文件与头文件接入 | 升级命令入口 | GPIO AF、USART、波特率、newlib | 串口可观察，接收有超时 |
| [06](../lessons/06-flash.md) | 可移植逻辑与硬件后端 | 受保护的 App 写入 | 扇区、擦除、编程、对齐 | 边界错误被拒绝，写后可比对 |
| [07](../lessons/07-image-format.md) | 工具与固件的职责 | 包头、长度、CRC、版本 | 字节序、padding、字节流 | PC 与 MCU 对同一文件算出同一 CRC |
| [08](../lessons/08-ymodem.md) | 独立协议组件 | 分包、重传、取消 | CRC16、序号回绕、超时 | 错包不会进入写入接口 |
| [09](../lessons/09-basic-update.md) | 接线组合成固件 | 接收→校验→安装→启动 | 单 Bank 写入与中断约束 | 两个 App 版本切换 |
| [10](../lessons/10-external-flash.md) | 存储适配接口 | 下载槽、暂存槽、备份槽 | SPI NOR 页/扇区 | 跨页数据读回一致，槽不互相覆盖 |
| [11](../lessons/11-metadata.md) | 公共格式与组件边界 | 状态跨复位保存 | EEPROM 页写、掉电、序号、CRC | 损坏一份记录仍可恢复 |
| [12](../lessons/12-app-download.md) | App 复用协议/存储库 | 下载和安装分离 | DMA、缓冲区所有权、可选 RTOS | App 下载后复位，由 Boot 安装 |
| [13](../lessons/13-trial-rollback.md) | 可测的状态决策 | 安装恢复、试运行、回退 | IWDG、复位来源、noinit | 新 App 启动失败可恢复旧版 |
| [14](../lessons/14-release.md) | Debug/Release、干净环境、依赖 | 发布与故障验收 | map、栈水位、调试证据 | 可复现产物和故障结果 |

每课约一次至数次学习会话，以验收结果推进，不预先承诺天数。前四课用现有板卡与 J-Link；05 起需要串口连接，10 起需要确认外部 Flash/EEPROM 型号与接线。

前四课已有原工程和模块化对照代码；课程整理未自动执行第四课的原 App 改造。后续协议、驱动与安装器按实验实现。读懂参考答案、自己完成练习、上板通过，是三种不同进度。
