# STM32F407ZE 串口升级 Bootloader

硬件基线为 STM32F407ZET6、8 MHz HSE、168 MHz SYSCLK、3.3 V 供电。Bootloader 使用 ST 标准外设库。板上使用 MCU 片上 Flash。升级流程沿用 Step2：先暂存 AES-256-CBC 密文，完整收包后再解密并安装到运行区。

## 从哪里读代码

| 想看什么 | 工程入口 |
| --- | --- |
| 上电、时钟、异常 | `System/Startup/startup_stm32f40_41xxx.s`、`Core/Src/main.c` |
| 按键、启动与升级的先后顺序 | `Tasks/Boot_Manager/boot_manager.c` |
| 串口一包如何变成文件数据 | `Middlewares/Ymodem/Current/boot_ymodem.c` |
| 密文暂存、解密安装、CRC32 与提交记录 | `Tasks/Boot_Manager/boot_image.c` |
| Step2 AES 实现 | `Middlewares/AES/AES.c` |
| 实际擦哪个扇区、如何写一个字 | `BSP/Src/bsp_flash.c` |
| USART1、PE3 和 TIM3 | `BSP/Src/bsp_board.c` |
| 外设清理、向量表和栈切换 | `Core/Src/boot_jump.c`、`System/Startup/boot_jump.s` |
| 日志与底层库 | `Tasks/Debug`、`Middlewares/easylogger`、`Middlewares/Rtt`、`Drivers` |

## 分区

| 用途 | 首地址 | 末地址（含） | 容量 |
| --- | --- | --- | --- |
| Bootloader，扇区 0、1 | `0x08000000` | `0x08007FFF` | 32 KiB |
| App 镜像，扇区 2 至 4 | `0x08008000` | `0x0801FFEF` | 98288 字节 |
| 镜像提交记录，扇区 4 末尾 | `0x0801FFF0` | `0x0801FFFF` | 16 字节 |
| 密文暂存区，扇区 5 至 7 | `0x08020000` | `0x0807FFFF` | 384 KiB，当前整包上限 98304 字节 |

`Core/Inc/boot_config.h` 和 `System/Linker/bootloader.ld` 对应以上边界。接收密文期间，已提交的运行区可以继续在下次复位后启动。安装阶段先擦除扇区 4 的旧提交记录，再擦除新镜像覆盖的其他运行扇区。安装中断后若 App 校验失败，复位会进入串口接收，重新发送完整升级包即可。原先使用扇区 7 提交记录的 App 需要按新布局重新安装。

## 用 VS Code 构建、下载与发送

1. 用 VS Code 打开 **bootloader 文件夹**。在 CMake Tools 中用 `CMake: Select Configure Preset` 选择 `Debug`，再执行 `CMake: Build`。产物为 `build/Debug/bootloader.elf`。用现有 F5 配置 **CMake Debug (J-Link)** 写入 Bootloader，停在 main 后按 F5 继续。
2. USB 转 3.3 V TTL 串口交叉接线：适配器 RX 接 PA9（USART1_TX），TX 接 PA10（USART1_RX），GND 接 GND。串口设置为 **115200、8N1、无流控**。升级键默认 PE3，内部上拉，按下接地；有应用时按住 PE3 再复位。首次没有有效记录时直接复位即可。
3. 终端周期性收到字符 `C` 后，选择 **YMODEM 发送**，单次发送符合下方格式的 Step2 加密升级包（例如 Tera Term 的文件传送菜单）。使用支持 CRC 和结束空包的普通 YMODEM 模式；单 Bank 擦除期间 MCU 可能暂停响应，发送端对数据包的等待时间建议至少 10 秒。
4. 接收时用 TIM3 更新标志驱动 PF9/PF10 心跳，SysTick 提供串口超时计时。日志输出到 J-Link RTT 通道 0。安装成功后自动跳转；下次复位松开 PE3 会验证并直接启动 App。单独通过 J-Link 写入 App ELF 不会生成有效的提交记录，首次联调请用第 3 步安装。

同样的构建操作也可从 bootloader 根目录执行：

```powershell
cmake --preset Debug
cmake --build --preset Debug
```

## 接收和跳转

- Ymodem 第 0 包声明的是密文总长度。数据包通过 CRC16 后写进暂存扇区，重复包只重发 ACK。会话结束时回读密文 CRC32，之后启动安装。
- Step2 明文结构是 16 字节头加补齐到整块的 App 正文。头的字节 12 到 15 以小端存放 App 原始长度。上位机将头和正文接成一条 AES-256-CBC 链。每次安装重新初始化 IV，解密头和第一块正文，检查总长度和 App 向量后才擦运行区。
- 安装逐块解密，只写 App 的实际长度。运行区回读 CRC32 通过后，在扇区 4 末尾写入长度、CRC32、长度反码和最后的完成标记。启动时复查这些字段。示例沿用 Step2 的固定教学密钥和 IV，CRC 校验用于发现传输或写入损坏，固件签名认证需要单独实现。
- 跳转前停 SysTick、复位板级外设、切回 HSI、清除 NVIC 待处理中断，并设置 `SCB->VTOR`。汇编最后设置 Cortex-M4 特殊寄存器 `CONTROL`（线程特权和栈选择）与 `MSP`（主栈指针），然后进入 App 的 Reset_Handler。

## 构建入口

根 CMake 配置定义 Cortex-M4 编译选项、固件源文件、链接脚本和输出。`Drivers/CMakeLists.txt` 选入用到的 SPL 模块；`Middlewares/CMakeLists.txt` 构建协议、AES 与日志库。
