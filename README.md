# STM32F407 Bootloader 基础工程

这是面向 **STM32F407ZET6** 的 **标准外设库 SPL + Arm GNU Toolchain + CMake + Ninja** 工程。

当前完成第一阶段的工程骨架：启动、系统时钟、GPIO、SysTick、链接布局与构建配置。主循环每隔 500 ms 翻转 PF9/PF10。
**尚未实现 App 跳转、串口接收固件、Flash 升级、固件校验和断电恢复。**

## 1. 硬件与时钟

| 项目 | 当前设置 |
| --- | --- |
| MCU | STM32F407ZET6，512 KiB Flash |
| HSE | 8 MHz 外部晶振 |
| PLL | M=8，N=336，P=2，Q=7 |
| SYSCLK / AHB | 168 MHz |
| APB1 / APB2 | 42 MHz / 84 MHz |
| PLLQ 输出 | 48 MHz，仅配置时钟，未启用 USB/SDIO/RNG |
| 演示 GPIO | PF9、PF10，推挽输出，初始低电平 |
| SysTick | 1 ms |
| 主循环 | 每 500 ms 翻转两路 GPIO |

PF9/PF10 沿用原工程的 GPIO 分配。它们是否连接 LED、LED 的有效电平，请以实际板卡原理图为准。

SPL 的 SystemInit() 在启动汇编中调用，负责配置时钟。main() 使用 SystemCoreClockUpdate() 更新软件时钟值，并检查是否为 168 MHz。
若 HSE 未成功启动、时钟不符合预期，或出现异常，程序会停在 Error_Handler()；可用调试器查看 g_boot_error。

## 2. Flash 与 SRAM

| 区域 | 起始地址 | 大小 | 说明 |
| --- | --- | --- | --- |
| Bootloader | 0x08000000 | 64 KiB | 扇区 0～3，本工程链接范围 |
| App 预留区 | 0x08010000 | 448 KiB | 尚未创建 App 工程 |
| 普通 SRAM | 0x20000000 | 128 KiB | 本工程数据与栈 |
| CCM SRAM | 0x10000000 | 64 KiB | 本工程暂未映射 |

链接脚本只允许本固件占用前 64 KiB Flash，超出时由链接器报错。栈预留 2 KiB，堆最低预留 512 B。
此布局仅用于第一阶段的 Bootloader/App 实验，还没有预留下载暂存区或独立升级信息区。

修改分区时，需要同步修改：

- linker/bootloader.ld
- User/Inc/boot_config.h

## 3. 目录

```text
bootloader/
├─ CMakeLists.txt                    固件目标、编译参数、链接脚本、产物导出
├─ CMakePresets.json                 Debug / Release 预设
├─ cmake/arm-none-eabi.cmake          ARM GCC 工具链定位
├─ linker/bootloader.ld              Bootloader 的 Flash / RAM 布局
├─ startup/startup_stm32f40_41xxx.s   配套的 GCC 启动汇编，已作项目适配
├─ Libraries/
│  ├─ CMakeLists.txt                 当前实际编译的 SPL 驱动列表
│  ├─ CMSIS/                        配套 CMSIS 头文件、芯片头文件与许可
│  └─ STM32F4xx_StdPeriph_Driver/    SPL 驱动源码与头文件
├─ User/
│  ├─ Inc/                          项目配置、标准库配置、中断声明
│  └─ Src/                          main、系统时钟、中断处理
├─ BSP/
│  ├─ Inc/                          板级驱动接口
│  └─ Src/                          GPIO 与毫秒节拍
└─ .vscode/                         CMake 构建任务与编辑器配置
```

Libraries 内只复制了需要的 CMSIS 头文件/设备定义和完整 SPL 外设驱动目录，没有复制 DSP 算法源码和示例工程。
实际参与编译的 SPL 驱动暂时只有 misc.c、stm32f4xx_rcc.c、stm32f4xx_gpio.c，其余驱动保留供后续接入。
工程已包含自身所需库文件，不依赖原始库目录继续存在。

## 4. 工具路径

本机已准备的工具：

- GCC 工具链根目录：C:/Tools/ArmGNU/15.2.Rel1
- GCC 可执行目录：C:/Tools/ArmGNU/15.2.Rel1/bin
- Ninja：C:/Tools/Ninja/ninja.exe
- CMake：C:/Program Files/CMake/bin/cmake.exe

构建默认通过 PATH 查找 ARM GCC、CMake、Ninja。工具链文件会从所选 GCC 的同一 bin 目录中查找 ar、ranlib、objcopy、size，避免混用其他版本。
如果终端尚未识别更新后的 PATH，请重启 VS Code。

也可以在首次配置时显式选择 GCC 根目录：

```powershell
cmake --preset Debug -DARM_GCC_ROOT=C:/Tools/ArmGNU/15.2.Rel1
```

SPL 宏由 Libraries/CMakeLists.txt 提供：STM32F40_41xxx、USE_STDPERIPH_DRIVER、HSE_VALUE=8000000U。
Debug 额外启用 USE_FULL_ASSERT。

## 5. 之后由你执行的构建命令

创建工程时未执行 CMake 配置、编译、链接、烧录或上板测试。以下命令仅供之后手动使用：

```powershell
cmake --preset Debug
cmake --build --preset Debug

cmake --preset Release
cmake --build --preset Release
```

构建成功后对应目录将产生：

- build/Debug/bootloader.elf
- build/Debug/bootloader.bin
- build/Debug/bootloader.hex
- build/Debug/bootloader.map
- build/Debug/compile_commands.json

Release 产物位于 build/Release。BIN 不包含地址信息，后续手动烧录本 Bootloader 的 BIN 时使用 0x08000000。

VS Code 中打开 bootloader.code-workspace 或整个工程目录。Ctrl+Shift+B 对应 CMake: Build Debug，会先配置再构建。
.vscode 中没有自动烧录任务；未创建 EIDE 的独立工程配置，也未设置调试探针类型。

## 6. 添加自己的文件

新增业务 .c 文件，加入根 CMakeLists.txt 的 add_executable(bootloader ...) 或 target_sources(bootloader PRIVATE ...)。
新增头文件目录，加入 target_include_directories(bootloader PRIVATE ...)。

接入新的 SPL 外设时：

1. 在 Libraries/CMakeLists.txt 的 stm32_spl 目标中添加对应的驱动 .c 文件。
2. 在 User/Inc/stm32f4xx_conf.h 中添加对应头文件。
3. 再编写自己的板级初始化与业务逻辑。

## 7. 下一阶段

先验证时钟与 GPIO，再添加串口日志。之后另建从 0x08010000 链接的测试 App，完成向量表设置和可靠的 App 交接。
在此之后再设计固件接收、Flash 擦写、固件完整性与真实性验证、升级状态和断电恢复。

当前主循环始终停留在 Bootloader，不会跳转 App，也不会擦除或改写 Flash。

## 8. 标准库来源与许可

源文件来自用户提供的 STM32F4xx_DSP_StdPeriph_Lib_V1.9.0。该包中部分组件头注释显示 V1.8.1，是包内组件版本，保留原样。
第三方许可与适配说明见 THIRD_PARTY.md、Package_license.md 和各组件的许可证文件。
