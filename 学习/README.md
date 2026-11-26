# 从你的两个工程开始学习 CMake、OTA 和嵌入式底层

本课程放在 `C:/Users/33481/Desktop/workspace/OTA/geng_ota/学习`。课程开头直接使用你的两份现有代码：

- [otatest](../otatest/CMakeLists.txt)：CubeMX + HAL + CMake 工程，用于理解生成式工程，并逐步改成 OTA App。
- [bootloader](../bootloader/CMakeLists.txt)：SPL + CMake 工程，用于理解显式的构建目标、32 KiB 分区和 App 交接。

**先读懂和改进自己的两份工程，再研究学习目录中的模块化实验代码。** `08_代码合集` 是后续 OTA 流程的参考教案。学习目录中的代码是对照和后续实验材料，不能替代前四课对原工程的分析。

## 从这里开始

1. 打开 [学习.code-workspace](学习.code-workspace)，同时查看学习资料、otatest 和 bootloader。
2. 阅读 [两份现有工程对照](docs/project-comparison.md) 和 [课程路线](docs/roadmap.md)。
3. 从 [第 01 课：对比两份工程的构建链](lessons/01-build-chain.md) 开始。
4. 按 [实验记录模板](records/template.md) 保存证据，并更新 [学习进度](progress.md)。

## 先学自己的代码

| 课次 | 主要教材 | 结果 |
|---|---|---|
| 01 构建链 | 两份 CMakePresets.json、工具链和根 CMakeLists.txt | 能追出源码如何生成 ELF |
| 02 目标与依赖 | otatest 的 stm32cubemx/STM32_Drivers；bootloader 的 mcu_options/stm32_spl | 能在现有工程添加模块，解释参数传播 |
| 03 启动与内存 | 两份启动汇编、system 文件和 ld | 能解释实际初始化顺序、Flash 分区和向量表 |
| 04 Boot→App | 原 bootloader 跳转模块 + 原 otatest 的 App 改造 | 让两个原工程完成交接并验证中断 |
| 05～14 OTA 实验 | 上述两个工程 + 08_代码合集 + 本目录模块化示例 | 按课实现串口升级、存储、状态与回退 |

需要修改原工程时，手册会明确指出具体文件和目标。本次整理只调整学习资料的位置和内容，没有替你修改原工程的固件配置。

## 三个工程不要混淆

```text
geng_ota/
├─ otatest/           课程起点之一：你的 HAL 工程，后续作为 App
├─ bootloader/        课程起点之二：你的 SPL Bootloader
└─ 学习/              全部课程文档、工具和模块化对照代码
   ├─ 学习.code-workspace         前四课入口，同时查看三处目录
   ├─ ota-course.code-workspace   后续独立实验工程入口
   ├─ lessons/                   14 课实验手册
   ├─ docs/                      对照、架构、调试和包格式
   ├─ records/                   你的实验记录
   ├─ firmware/bootloader/       模块化对照 Bootloader
   ├─ firmware/app/              模块化对照 App
   ├─ components/                镜像检查、状态决策示例
   ├─ platform/                  F407 板级示例
   ├─ config/                    实验工程的单一分区配置
   ├─ cmake/                     实验工程的 CMake 模块
   ├─ linker/、startup/、third_party/
   └─ tools/、tests/
```

注意：config/flash_layout.json 目前只管理学习目录中的实验工程，**不会自动修改原 otatest 和 bootloader**。第一阶段要先看清原工程各自的配置入口，之后再学习如何集中管理。

## 命令在哪执行

第一课的命令在原工程分别执行：

```powershell
Set-Location 'C:\Users\33481\Desktop\workspace\OTA\geng_ota\otatest'
cmake --list-presets

Set-Location 'C:\Users\33481\Desktop\workspace\OTA\geng_ota\bootloader'
cmake --list-presets
```

以下命令属于学习目录中的工具与独立实验，不是原工程的命令：

```powershell
Set-Location 'C:\Users\33481\Desktop\workspace\OTA\geng_ota\学习'
python tools/ota_tool.py doctor
python -B -m unittest discover -s tests -v

# 后续研究模块化对照工程时由你执行
cmake --preset Debug
cmake --build --preset Debug
```

在多目录工作区选择构建/调试前先确认目标文件夹。三个目录都存在 Debug 预设，但源码和产物不同。打开学习.code-workspace 时不会自动执行配置。

## 已有材料与待做实验

已经提供独立 Boot/App 对照目标、分区生成、向量校验、交接汇编、心跳、状态决策示例、Python 打包/验包工具，以及 14 课手册。

UART、Flash 写入、Ymodem、外部存储、持久化安装与回退需要按第 05～14 课完成。原 otatest 尚未因本次整理改成偏移 App；原 bootloader 的自动跳转也不能直接替代后续 OTA 状态管理。

本机工具路径：Arm GNU GCC `C:/Tools/ArmGNU/15.2.Rel1/bin`；Ninja `C:/Tools/Ninja`；J-Link `D:/jlink_software`。独立实验工程要求 CMake >=3.22、Arm GNU GCC >=11。

本次没有执行固件配置、编译、链接、下载或板上测试。Python 离线工具测试与设备验收分开记录。

教学方式见 [逐课教学约定](docs/teaching-method.md)，原教案映射见 [参考对应关系](docs/reference-map.md)，来源和许可见 [THIRD_PARTY.md](THIRD_PARTY.md)。
