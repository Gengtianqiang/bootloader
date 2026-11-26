# 第 10 课：独立的下载、暂存和备份槽

前置：第 09 课。硬件阶段先确认 W25Q 型号、容量、供电、SPI 引脚和片选。

## 为什么重做分区

参考工程后期 A/B 区以 64 KiB 为间隔，而你的 App 容量为 480 KiB。把 App 容量直接套入旧 Handler 会覆盖相邻区。本课程为支持恢复，保留独立槽，而不在安装中复用升级包所在的 A 区。

以确认有 8 MiB 的 W25Q64 为例，未来配置可采用：

| 槽 | SPI 偏移 | 容量 | 内容 |
|---|---|---|---|
| download | 0x000000 | 512 KiB | 原始升级包，含 32 字节头 |
| staging | 0x080000 | 512 KiB | 经验证/可选解密的新 App |
| backup | 0x100000 | 512 KiB | 旧 App 和独立完整性信息 |

这是本课待实现的外部布局，不是当前 flash_layout.json 已定义的内部地址。外部 SPI 偏移不能当 CPU 可直接执行地址。

## 实现接口

```c
int Storage_Read(Slot slot, uint32_t offset, uint8_t *data, uint32_t length);
int Storage_Write(Slot slot, uint32_t offset, const uint8_t *data, uint32_t length);
int Storage_Erase(Slot slot);
```

将 SPI NOR 驱动作为后端；槽边界检查在统一接口完成。选择原生驱动或 SFUD 中的一套作为实际偏移/长度权威，不让两套 Handler 同时更新不同状态。

## 底层实验

读 JEDEC ID，确认容量；写入跨 256 字节页和 4 KiB 擦除扇区的数据；断电再读回比较；分别填充三个槽检查互不影响。写前擦除，轮询 WIP，超时和底层错误必须向上传播。

RAM 缓冲写满和文件结束 flush 都要验证，最后不足一页的数据不能丢失。重启后有效长度从持久化记录恢复，不能依赖 RAM 里归零的 write_index。

## 验收

480 KiB 载荷加包头可放入下载槽；跨界输入拒绝；断电后仍可读回完整文件。解释页编程与扇区擦除的区别，为什么每收 16 字节就擦写同一 4 KiB 扇区可能破坏已写内容。
