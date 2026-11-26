# CMake Tools 与 Cortex-Debug 实验流程

本文件的 Debug/JumpDebug、g_app_heartbeat 和双镜像配置属于学习目录中的**模块化对照工程**。前四课以原 otatest/bootloader 为主，原工程操作先按 lessons/01～04；不要将本文件路径直接套到原工程。

打开 ota-course.code-workspace，插件推荐 CMake Tools、C/C++、Cortex-Debug。CMake Tools 选择 Configure Preset；明确使用任务或命令行构建同一预设。

本机调试配置使用：

- GCC/GDB：`C:/Tools/ArmGNU/15.2.Rel1/bin`
- J-Link GDB Server：`D:/jlink_software/JLinkGDBServerCL.exe`
- MCU：STM32F407ZE，SWD，4000 kHz

这是机器相关的 launch.json 设置。换电脑修改工具路径；不需要让 CMake 目标依赖这些绝对路径。未配置 SVD，因此可以看寄存器/内存，但外设专用视图需补充可用的 F407 SVD。

## 第 01 课：Bootloader 心跳

选择 `01 Bootloader heartbeat (Debug)`。preLaunchTask 配置并构建 Debug，再由 J-Link 下载 Boot ELF，在 Boot main 停住。继续后 PF9 翻转，观察 g_boot_heartbeat、g_course_fault、SystemCoreClock。

此配置只负责 Boot ELF，不安装 App。已有 App 是否存在不会影响 Debug 的心跳，因为 COURSE_ENABLE_APP_JUMP 为 OFF。

## 第 04 课：通过硬件复位验证交接

选择 `04 Handoff: load both images (JumpDebug)`。它构建 JumpDebug，用 loadFiles 下载两个 ELF，再从 Boot 停在 main。

1. 在 Boot_JumpToApp 和 Boot_EnterApp 设置断点。
2. 继续，观察 0x08008000 的两个向量字；初始 MSP 一般为 0x20020000。
3. 换栈前后检查 MSP；进入 App 后检查 VTOR=0x08008000、SysTick 正常。
4. App 使用 PF10，200 ms 翻转；Boot 心跳使用 PF9，500 ms 翻转。
5. 退出调试，按硬件复位键，确认仍从 Boot 进入 App，避免把调试器直接设置 PC 当作交接成功。

第二个配置的主符号文件是 Boot ELF。需要 App 源码和变量时，结束当前会话，使用 `04 Observe installed App (attach; no download)`，其主符号文件为 App ELF，不执行下载或构建。也可在 GDB 控制台使用对应插件的命令前缀执行：

```text
add-symbol-file "C:/Users/33481/Desktop/workspace/OTA/geng_ota/学习/build/JumpDebug/firmware/app/app.elf"
```

GNU GDB 从已链接 ELF 的节地址加载附加符号；不要手工把 App 的 .text 起点猜成向量表起点。重复的 main/SystemInit 符号可用文件限定断点或地址断点区分。

J-Link 下载/擦除行为仍需板上验证。若工具配置要求整片擦除，两次单独下载可能互相清除；优先一次会话加载两个 ELF，并在下载后读取两个向量表核实。后续 OTA 恢复实验不要用调试器整片重写来代替恢复路径。

## 故障定位顺序

| 现象 | 先检查什么 |
|---|---|
| 找不到 GCC | PATH、ARM_GCC_ROOT、CMakeCache 中的编译器路径 |
| 新文件没编译 | target_sources/库源列表；不是仅看 VS Code 文件树 |
| 找不到头文件 | 对应目标的 include 路径和依赖传播 |
| undefined reference | .c 是否加入目标、符号签名、静态库提取 |
| App 不跳转 | JumpDebug、两个向量、Thumb 位、入口范围 |
| App main 能进但延时卡住 | VTOR、PRIMASK、SysTick_Handler 是否实际链接 |
| 出现 Board_Fault | g_course_fault；HardFault 时保存堆栈和异常寄存器 |
| Flash overflow | 真实固件尺寸、分区配置，不删除边界检查掩盖问题 |

如果变更编译器，使用新的 binaryDir 或重新配置干净的构建目录，不能只改 JSON 后沿用旧 cache。
