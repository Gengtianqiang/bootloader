# 本课程的 OTA1 包格式

第 07 课开始使用。当前 Python 端已经实现打包/验证；MCU 解析与安装在后续实验实现。不要把整个 .ota 当 App BIN 直接烧到 App 地址。

头部固定 32 字节，小端序，随后是原始 App BIN：

| 偏移 | 字节数 | 内容 |
|---|---:|---|
| 0 | 4 | ASCII `OTA1` |
| 4 | 2 | 格式版本，当前 1 |
| 6 | 2 | 头长度，当前 32 |
| 8 | 4 | App 加载地址 |
| 12 | 4 | 原始 BIN 精确长度，不含头和协议填充 |
| 16 | 4 | 原始 BIN 的 CRC32 |
| 20 | 4 | App 版本号，uint32 |
| 24 | 8 | 保留，必须全零 |
| 32 | N | App BIN 原始内容 |

CRC 使用 CRC-32/ISO-HDLC，与 Python zlib.crc32 对应：反射多项式 0xEDB88320，初始值 0xFFFFFFFF，最终异或 0xFFFFFFFF；字符串 `123456789` 的结果为 0xCBF43926。

在 MCU 上按字节读取并组合字段，不直接把串口缓冲区强转成编译器结构体。检查魔数、版本、长度、地址范围、精确文件长度、向量、入口实际落在镜像字节内，然后检查 CRC。分块 CRC 的初始/最终处理必须与整块一致。

```powershell
python tools/ota_tool.py inspect build/Debug/firmware/app/app.elf
python tools/ota_tool.py pack build/Debug/firmware/app/app.bin build/app-v1.ota --version 1
python tools/ota_tool.py verify build/app-v1.ota
```

pack 默认不覆盖已有输出。BIN 没有地址标签；本工具按当前 App 分区解释它，并检查入口范围。它不能证明所有代码都按正确地址链接，因此发布时同时检查 ELF。

ELF inspect 读取节表和向量，不替代完整 ELF 装载器/链接器检查。CRC 覆盖载荷，当前头部只有格式和范围检查，没有头部认证；版本字段本身也不是防降级机制。签名、密钥与加密作为后续扩展，AES 解密成功不等于来源认证。
