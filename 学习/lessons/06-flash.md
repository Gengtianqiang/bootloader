# 第 06 课：受边界约束的内部 Flash 写入

前置：第 05 课；代码待按实验实现。参考 09 工程的 Flash.c，但重新设计边界和返回状态。

## 接口目标

```c
int AppFlash_Erase(uint32_t offset, uint32_t length);
int AppFlash_Write(uint32_t offset, const uint8_t *data, uint32_t length);
int AppFlash_Read(uint32_t offset, uint8_t *data, uint32_t length);
```

上层使用 App 内偏移，不允许传任意绝对地址。先检查 offset<=APP_SIZE，再用 length<=APP_SIZE-offset 检查范围，避免 offset+length 溢出。

## 分两次实验

先用 RAM 数组模拟擦写，检查长度为 0、首/尾字节、越界和整数溢出。再接入 F407 Flash 驱动，只操作明确用于本实验的 App 区内容。内部 Flash 是运行镜像，擦写实验之后需要重新安装测试 App。

SPL 新增 stm32f4xx_flash.c 和对应头。把 Flash 后端与镜像逻辑分目标组织，不让串口解析代码直接到处调用 FLASH_ProgramWord。

## 必须解释的硬件行为

F407ZE 的 Sector 0～3 各 16 KiB，Sector 4 为 64 KiB，Sector 5～7 各 128 KiB。32 KiB Boot 占 Sector 0～1。不能按“每扇区都一样大”计算擦除范围。

NOR 写入只能把位从 1 改到 0，恢复成 1 需要擦除。按字编程需满足地址对齐及供电对应的编程宽度要求；尾部不足 4 字节要明确补 0xFF，不能越界读取接收缓冲区。

单 Bank Flash 擦写时从同一 Bank 取指可能停顿；串口和中断时序会受影响。Boot 代码仍在 Flash 中，“关闭中断就能无限接收高速数据”并不成立。

## 验收案例

| 输入 | 结果 |
|---|---|
| Boot 区地址经偏移伪装/下溢 | 拒绝 |
| App 最后一字节 | 有明确定义的写入处理 |
| offset=APP_SIZE，length=1 | 拒绝 |
| length 极大导致加法溢出 | 拒绝 |
| 跨 Sector 3/4 的有效区间 | 擦除正确的两个扇区 |
| 写入成功 | 读回比较、检查错误返回 |

保存原 Boot 向量表的读数，实验后核对未变化。解释 Flash 返回码如何传到安装器，避免参考代码 void Flash_Write 丢失失败信息。
