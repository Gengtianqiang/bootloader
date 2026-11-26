# 第 01 课：用你的两个工程解释一次构建

本课直接使用 geng_ota/otatest 和 geng_ota/bootloader。先不构建学习目录里的独立实验工程，也不引入新硬件。

## 阅读顺序

| 步骤 | otatest | bootloader | 对比问题 |
|---|---|---|---|
| 1 | [CMakePresets.json](../../otatest/CMakePresets.json) | [CMakePresets.json](../../bootloader/CMakePresets.json) | default/base、Debug、binaryDir 如何对应？ |
| 2 | [gcc-arm-none-eabi.cmake](../../otatest/cmake/gcc-arm-none-eabi.cmake) | [arm-none-eabi.cmake](../../bootloader/cmake/arm-none-eabi.cmake) | 工具怎样找到？哪些设置属于平台？ |
| 3 | [根 CMakeLists.txt](../../otatest/CMakeLists.txt) | [根 CMakeLists.txt](../../bootloader/CMakeLists.txt) | project、可执行目标、子目录在哪里？ |
| 4 | [stm32cubemx/CMakeLists.txt](../../otatest/cmake/stm32cubemx/CMakeLists.txt) | [Libraries/CMakeLists.txt](../../bootloader/Libraries/CMakeLists.txt) | main 和驱动从哪里加入？ |
| 5 | [.vscode/tasks.json](../../otatest/.vscode/tasks.json) | [.vscode/tasks.json](../../bootloader/.vscode/tasks.json) | 哪个任务调用 CMake，哪个调用 EIDE？ |

## 先建立一条链

```text
CMake Tools/命令行选择原工程的 Debug 预设
  → CMake 读取工具链和 CMakeLists
  → 配置并生成 Ninja 依赖规则
  → Ninja 调用 GCC、汇编器与链接器
  → 对应原工程的 ELF
```

otatest 的主目标名是 otatest。根文件先 add_executable，再由 stm32cubemx 子目录 target_sources 加入 main 和启动汇编。bootloader 在根文件中显式列出 main、启动汇编和板级源文件。

两种写法都在描述构建图。VS Code 文件树里出现一个 .c，不等于它已经属于某个目标。

配置阶段决定工具、源文件、参数和依赖；生成阶段写出构建规则；构建阶段执行实际命令。通常 cmake --preset 一次完成前两步。

## 第一轮：只读观察

分别进入两个原工程运行：

```powershell
Set-Location 'C:\Users\33481\Desktop\workspace\OTA\geng_ota\otatest'
cmake --list-presets

Set-Location 'C:\Users\33481\Desktop\workspace\OTA\geng_ota\bootloader'
cmake --list-presets
```

先从 JSON 手算 Debug 的实际 binaryDir。两个目录都叫 build/Debug，但分别位于不同源码根目录。

如果已有 CMakeCache.txt 和 compile_commands.json，直接观察：Cache 中记录了哪个编译器？main.c 的编译命令是什么？已有文件不保证匹配最新源码，先记录配置来源。

## 第二轮：由你执行一次构建实验

在每个原工程的根目录分别执行：

```powershell
cmake --preset Debug
cmake --build --preset Debug --verbose
```

分别保存一条 C 编译、汇编和最终链接命令。bootloader 明确有 POST_BUILD 导出 BIN/HEX；当前 otatest 脚本没有同样的导出命令，不能默认每份工程都会生成同样产物。

compile_commands.json 记录逐源文件编译，不是完整链接日志。链接常由 GCC 驱动执行，命令里出现 gcc 不代表它一定在编译 .c。

原 otatest 的 tasks.json 同时保留 EIDE 和 CMake 任务。本课明确使用 CMake 命令或 CMake: Build Debug，避免把两套构建结果混为一谈。

## 本课验收

1. 写出两份 Debug 的源码目录、binaryDir、目标名和 ELF 路径。
2. 指出两份 main.c 分别在哪一行构建描述中加入目标。
3. 找到两份链接命令中的 -T，指出对应 ld。
4. 说明 toolchainFile 为什么要在 project 检测编译器前生效。
5. 解释配置成功、编译成功、链接成功分别说明什么。

把实际输出写进 [实验记录](../records/template.md)。本课不要求先理解学习目录里的 Firmware.cmake 封装。
