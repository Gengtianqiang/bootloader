# 第 07 课：文件、镜像和升级包

前置：第 03、06 课。Python 工具已给出；MCU 解析/CRC 本课实现。

## 先理解

ELF 有地址、符号和调试信息；BIN 是字节流，不携带加载地址；HEX 携带地址记录；本课程 OTA1 包是 32 字节头加 App BIN。

阅读 ../docs/package-format.md 和 ../tools/ota_tool.py。头部字段固定宽度且小端，不依赖 C 结构体布局。原教案 AES 密文文件另有格式，不能直接喂给 OTA1 解析器。

## PC 实验

```powershell
python tools/ota_tool.py inspect build/Debug/firmware/app/app.elf
python tools/ota_tool.py pack build/Debug/firmware/app/app.bin build/app-v1.ota --version 1
python tools/ota_tool.py verify build/app-v1.ota
python -B -m unittest discover -s tests -v
```

对副本改一个载荷字节，verify 应报 CRC 错误；截断或追加字节应报长度错误；改变 load address 应拒绝安装。先预测再观察，不破坏原始固件。

## MCU 实验

新增 image_header / crc32 组件，先解析已知字节数组，再解析实际包头。用 `123456789` 验证结果 0xCBF43926；将同一镜像分成 1、7、256 字节块，计算结果必须一致。

检查 payload 长度不超过 App 区，Reset_Handler 不只在分区中，还应落在实际收到的镜像字节里。协议尾包填充和 AES 填充都不自动属于有效镜像。

## 验收

PC 和 MCU 对同一 App 得到相同长度与 CRC；异常包在任何内部 Flash 擦除之前被拒绝。能解释 CRC16 包校验、CRC32 整镜像校验、签名认证分别回答什么问题。

思考：为什么 AES 解密返回成功不代表固件正确？版本号能否单独阻止降级？为什么 bin 文件名不能证明其链接地址？
