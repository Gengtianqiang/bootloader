# 第 02 课：在你的工程中添加模块，理解目标与传播

前置：第 01 课；教材仍是原 otatest 和 bootloader。

## 先画出真实目标

otatest：

```text
otatest 可执行目标
  ├─ stm32cubemx INTERFACE：HAL 头文件与宏
  └─ STM32_Drivers OBJECT：HAL 驱动和 system 对象
       └─ PUBLIC stm32cubemx
```

bootloader：

```text
bootloader 可执行目标
  └─ stm32_spl STATIC：RCC/GPIO/misc
       └─ PUBLIC mcu_options INTERFACE：CPU/FPU 与编译链接参数
```

在两个根文件和子目录 CMakeLists 中逐项找到证据。以 add_library 的实际类型为准；不要因为旧注释写着 static 就把 OBJECT 当 STATIC。

一个 .c 是编译单元；一个目标描述源文件、要求和产物。INTERFACE 不编译机器码；OBJECT 编译对象；STATIC 归档对象。PUBLIC 同时作用于自身和使用者，PRIVATE 只作用于自身，INTERFACE 只作用于使用者。

## 实验 A：先给 otatest 添加一个普通模块

在原 otatest/APP 下新建 lesson_counter.h/.c（先确认不覆盖已有同名文件）：

```c
/* lesson_counter.h：自行加头文件保护 */
#include <stdint.h>
uint32_t LessonCounter_Next(void);

/* lesson_counter.c */
#include "lesson_counter.h"
uint32_t LessonCounter_Next(void)
{
    static uint32_t count;
    return ++count;
}
```

在原 otatest 根 CMakeLists 的用户区加入：

```cmake
target_sources(${CMAKE_PROJECT_NAME} PRIVATE APP/lesson_counter.c)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE APP)
```

在原 Core/Src/main.c 的 USER CODE 区声明 volatile 观察变量并调用函数。先不修改 CubeMX 生成的 HAL 驱动列表。

## 实验 B：在 bootloader 中做同样的事

在原 bootloader/User/Inc 和 User/Src 添加另一个同名练习模块，明确列入 bootloader 的 add_executable 或 target_sources。头路径 User/Inc 已在目标上，无须重复配置。

这是两份独立源码和 ELF，不会因此共享 RAM。以后再研究如何抽出共同库。

## 实验 C：观察错误发生在哪一阶段

保留函数调用，临时从源列表移除对应 .c。预期链接报 undefined reference，而不是头文件错误。恢复后，将头文件路径暂时移错，比较编译报错。每次只改变一件事并恢复。

## 实验 D：PRIVATE 不能反向传播

在原 bootloader 的 PRIVATE 定义中添加一个练习宏，观察它是否出现在 stm32_spl 的编译命令里。使用者的 PRIVATE 宏不会自动反向影响依赖库。

这点在第四课尤其重要：原 otatest 的 SystemInit 属于 STM32_Drivers，而不是 otatest 主目标自身。向量偏移宏必须送到真正编译 system 文件的目标。

## 验收

能独立完成两个原工程的文件添加；能区分缺头、缺实现、重复符号；能解释为什么同名 main 分属两个 ELF 没问题，而放进同一个 ELF 会冲突。

完成后再对照学习目录的 components 和 platform，看它们如何把刚才的重复组织抽成库；不要求先照抄所有抽象。
