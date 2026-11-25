# STM32F407ZE 串口升级 Bootloader

硬件基线为 STM32F407ZET6、8 MHz HSE、168 MHz SYSCLK、3.3 V 供电。Bootloader 使用 ST 标准外设库，应用沿用同级 `../otatest` 的 HAL 源码。板上只需 MCU 片上 Flash。

## 从哪里读代码

| 想看什么 | 工程入口 |
| --- | --- |
| 上电、时钟、异常 | `System/Startup/startup_stm32f40_41xxx.s`、`Core/Src/main.c` |
| 按键、启动与升级的先后顺序 | `Tasks/Boot_Manager/boot_manager.c` |
| 串口一包如何变成文件数据 | `Middlewares/Ymodem/ymodem.c` |
| 长度、入口地址、CRC32、提交记录 | `Tasks/Boot_Manager/boot_image.c` |
| 实际擦哪个扇区、如何写一个字 | `BSP/Src/bsp_flash.c` |
| USART1、PE3 和 TIM3 | `BSP/Src/bsp_board.c` |
| 外设清理、向量表和栈切换 | `Core/Src/boot_jump.c`、`System/Startup/boot_jump.s` |
| 日志与底层库 | `Tasks/Debug`、`Middlewares/easylogger`、`Middlewares/Rtt`、`Drivers` |

## 分区

| 用途 | 首地址 | 末地址（含） | 容量 |
| --- | --- | --- | --- |
| Bootloader，扇区 0、1 | `0x08000000` | `0x08007FFF` | 32 KiB |
| App 镜像，扇区 2 至 7 | `0x08008000` | `0x0807FFEF` | 491504 字节 |
| 镜像提交记录，扇区 7 末尾 | `0x0807FFF0` | `0x0807FFFF` | 16 字节 |

`Core/Inc/boot_config.h`、`System/Linker/bootloader.ld` 和 `System/Linker/otatest_app.ld` 描述相同的边界。每次安装先擦记录所在的扇区 7，然后只擦新镜像覆盖的其他 App 扇区。旧应用在安装过程中被覆盖。断电后只要记录或 CRC 无效，Bootloader 就等待重传；恢复方法是重新发送完整文件。

## 用 VS Code 构建、下载与发送

1. 用 VS Code 打开 **bootloader 文件夹**。在 CMake Tools 中用 `CMake: Select Configure Preset` 选择 `Debug`，再执行 `CMake: Build`。产物为 `build/Debug/bootloader.elf`。用现有 F5 配置 **CMake Debug (J-Link)** 写入 Bootloader，停在 main 后按 F5 继续。
2. 仍在这个文件夹中选择 **App-Debug** 预设，执行构建（默认目标）。它读入同级 `otatest/CMakeLists.txt` 的源文件配置，用 App 分区脚本链接，将 Cortex-M4 系统控制块的向量表基址寄存器 `SCB->VTOR` 配置为 `0x08008000`。串口发送文件是 **`build/App-Debug/otatest.bin`**。同目录的 ELF 带调试符号。直接从原 `otatest` 的默认 Debug 预设生成的 ELF 仍使用整片 Flash 布局。
3. USB 转 3.3 V TTL 串口交叉接线：适配器 RX 接 PA9（USART1_TX），TX 接 PA10（USART1_RX），GND 接 GND。串口设置为 **115200、8N1、无流控**。升级键默认 PE3，内部上拉，按下接地；有应用时按住 PE3 再复位。首次没有有效记录时直接复位即可。
4. 终端周期性收到字符 `C` 后，选择 **YMODEM 发送**，单次发送一个 `.bin` 文件（例如 Tera Term 的文件传送菜单）。使用支持 CRC 和结束空包的普通 YMODEM 模式；单 Bank 擦除期间 MCU 可能暂停响应，发送端对数据包的等待时间建议至少 10 秒。
5. 接收时用 TIM3 更新标志驱动 PF9/PF10 心跳，SysTick 提供串口超时计时。日志输出到 J-Link RTT 通道 0。安装成功后自动跳转；下次复位松开 PE3 会验证并直接启动 App。单独通过 J-Link 写入 App ELF 不会生成有效的提交记录，首次联调请用第 4 步安装。

同样的构建操作也可从 bootloader 根目录执行：

```powershell
cmake --preset Debug
cmake --build --preset Debug
cmake --preset App-Debug
cmake --build --preset App-Debug
```

## 接收和跳转

- Ymodem 第 0 包给出文件名和真实字节数。首个数据包通过 CRC16、初始 MSP 和复位向量范围检查后才开始擦除。接收器按声明长度写入，尾包的协议填充不会进入镜像。收到重复序号只重发 ACK。
- 写 Flash 采用 32 位对齐编程和逐字回读。最后一个字不足 4 字节时用 `0xFF` 补齐。文件收完后重新读取整段 Flash 算 CRC32，与接收期间计算的 CRC32 比较。
- 记录依次写入长度、CRC32、长度按位取反值和完成标记。启动时复查记录、向量和整段 CRC32。CRC 用于检出传输或存储损坏；发送者身份认证需要另行设计。
- 跳转前停 SysTick、复位板级外设、切回 HSI、清除 NVIC 待处理中断，并设置 `SCB->VTOR`。汇编最后设置 Cortex-M4 特殊寄存器 `CONTROL`（线程特权和栈选择）与 `MSP`（主栈指针），然后进入 App 的 Reset_Handler。

## 构建入口

根 CMake 配置定义 Cortex-M4 编译选项、固件源文件、链接脚本和输出。`Drivers/CMakeLists.txt` 选入用到的 SPL 模块；`Middlewares/CMakeLists.txt` 构建协议与日志库。`cmake/otatest-app/CMakeLists.txt` 将 App 的源文件配置接到 OTA 构建预设。EIDE 的 Bootloader 资源配置在 `.eide/eide.yml`，输出到 `out/eide`。

如果已打开的 EIDE 窗口将资源树覆盖回旧目录，先关闭这个工程的所有 VS Code 窗口。在 bootloader 根目录运行 `Copy-Item -LiteralPath .eide/step3.yml -Destination .eide/eide.yml -Force`，再重新打开工程。
