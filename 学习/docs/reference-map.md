# 原教案到新实验的对应关系

原始资料目录：`C:/Users/33481/Desktop/workspace/OTA/08_代码合集`。原始工程不做覆盖。压缩包路径后的源码路径是包内相对路径。

课程第一教材是用户现有的 [otatest](../../otatest/CMakeLists.txt) 和 [bootloader](../../bootloader/CMakeLists.txt)。先完成两份工程的构建、目标、启动和交接实验，再用下面的代码合集补齐 OTA 流程。具体差异见 [原工程对照](project-comparison.md)。

| 实验 | 原始资料 | 优先阅读 |
|---|---|---|
| 01～03 | 05 基础工程；现有 bootloader/otatest | main、启动汇编、scatter/ld；Keil 选项对应到目标属性 |
| 04 | 06 跳转；09 已解压工程 | Tasks/Boot_Manager/Boot_Manager.c，JumpToApp |
| 05、08 | 07 外设、08 Ymodem、09 已解压工程 | Core/Src/Usart.c；Middlewares/Ymodem/ymodem.c |
| 06、09 | 09_Basic_Bootloader_App_Receive_Jump_Step3 | Core/Src/Flash.c，Ymodem_Receive，main |
| 07 | 14 解密搬运；29_Integrity_Check_Step2.zip | ExA_To_ExB_AES；Receive_Packet 的 crc_xmodem |
| 10 | 17 W25Q、18_External_Flash_ReplacementStep3 | Bsp/W25Qxx，Flash Handler |
| 11、12 | 21～24 App 集成；27_OTA_Application_Logic_Step10_App.zip | SWC_OTA.c，AT24Cxx_Driver.c，写入任务 |
| 11、13 | 27_OTA_Application_Logic_Step10_Boot.zip | Bootloader/Tasks/Boot_Manager/Boot_Manager.c |
| 13 | 30_Watchdog_Detection_Step6_App.zip / Boot.zip | App main 的 0x44 确认；Boot 状态机和 Iwdg.c |
| 13、14 | Course_31...、32...、33...、34... | 跳转策略、SFUD 适配、驱动边界 |

## 保留的教学思路

Boot 与 App 分工；串口升级的最小闭环；下载与安装分离；W25Q 暂存；EEPROM 跨复位协作；新 App 首次启动确认；借助 IWDG 在失败时重新获得控制。

## 重新设计的地方

1. **F411 → F407ZE。** 后期参考 Keil 工程目标是 STM32F411CEUx。移植时使用当前板子的 8 MHz HSE、168 MHz 系统时钟、引脚与 Flash 扇区。
2. **Keil 项目选项 → CMake 目标属性。** 不复制绝对地址变量 `__attribute__((at(...)))`；共享状态若放 RAM，应由 ld 的 NOLOAD 区和启动约定共同支持。
3. **向量掩码 → 显式边界检查。** 128 KiB 普通 SRAM 的合法初始栈顶包含 0x20020000，同时检查 Thumb 位和复位入口分区。
4. **C 函数内换栈 → 独立交接汇编。** 换 MSP 后不能继续依赖旧 C 栈帧。
5. **单个升级包/备份复用 A 区 → 独立下载、暂存、备份槽。** 后续实验以明确状态避免覆盖恢复所需数据。
6. **完成标志最后提交。** 参考 App 先写 0x22、后写长度，断电窗口不一致；本课程要求长度/校验信息先落盘，再提交 DOWNLOADED。
7. **回退必须实际执行与验证。** 后期参考代码的 ExA_To_App 调用被注释；函数内部还存在把 RAM 指针当写入数值的问题，不能直接启用。
8. **状态中明确 INSTALLING。** 不用“有备份”代替安装中断后的恢复流程。
9. **区分校验层次。** 第 29 阶段 Boot 接收函数检查 CRC16；检查过的第 30 阶段 App 接收路径没有相同 CRC 比较。存在 CRC 函数不代表调用了它。CRC 也不证明固件来自可信发布者。
10. **检查适配语义。** 第 33、34 阶段 SFUD 与旧 Handler 混用两套长度/偏移管理，调用成功不等于上层状态一致。

原教案中的状态 0x00/0x11/0x22/0x33/0x44 用于理解历史流程；新 Ota_State 不直接兼容其 EEPROM 二进制格式。不要把两种版本连接到同一份元数据而不做迁移。
