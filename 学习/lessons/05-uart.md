# 第 05 课：让升级过程可观察

前置：第 04 课通过。待实现 UART 驱动；先确认串口引脚和电平。

## 本课目标

Boot/App 能打印身份、版本、系统时钟和 App 地址；读取命令有超时。参考 07 外设与 09 工程的 Usart.c，不沿用 F411 的时钟和引脚假设。

## CMake 实验

在 third_party/CMakeLists.txt 的 stm32_spl 中增加 stm32f4xx_usart.c；在 stm32f4xx_conf.h 中启用对应外设头。新增 board_uart.c，明确加入 board_support。

先写有限长度的 Board_UartWrite / Board_UartReadByte 接口，再考虑 printf。需要 printf 时实现 _write，将缓冲区发往 UART；声明和返回值符合 newlib 的接口约定。不依赖 nosys 占位函数输出字符。

## 底层实验

1. 根据板子确认 USART、TX/RX、GPIO 复用号、APB 时钟。PA9/PA10 USART1 只是可选例子。
2. 先轮询输出固定字符串，示波器或串口端确认波特率。
3. 接收一个命令并回显；加入超时，拔掉发送端不应永久卡在接收字节函数。
4. 再接入 printf，观察标准库引用及固件体积变化。

USART1 在 APB2、USART2/3 通常在 APB1；不要把 SYSCLK 直接代入所有波特率计算。多字节字符串可能遇到缓冲或重入问题，不在每个中断中随意 printf。

## 与跳转的连接

UART 中断/DMA 一旦启用，交接前必须停接收、停止 DMA 并清理状态。仅关闭 NVIC 不能停止 DMA 写 RAM。先保持轮询实现，后续再研究 DMA。

## 验收

记录真实波特率、引脚、接收超时结果和一段身份输出。解释为什么加了头文件仍可能 undefined reference；为什么 _write 警告与链接失败是两回事。

习题：把 UART .c 从目标列表暂时移除，预测报错；恢复后检查 compile_commands 中 CPU/FPU 参数是否一致。
