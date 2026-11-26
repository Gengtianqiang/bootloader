# 第 14 课：让工程和升级结果可复现

前置：前面各课，尤其是失败恢复实验。

## CMake 收尾

使用明确的 Debug / Release 预设和独立 binaryDir；检查每个库的 CPU/FPU ABI 一致；去掉只因本机环境偶然存在的 include/lib 依赖。换工具链时用新构建目录。

```powershell
cmake --preset Release
cmake --build --preset Release --verbose
python tools/ota_tool.py inspect build/Release/firmware/app/app.elf
python tools/ota_tool.py pack build/Release/firmware/app/app.bin build/app-release-v2.ota --version 2
python tools/ota_tool.py verify build/app-release-v2.ota
```

保存版本、工具链版本、源代码版本、配置、BIN 哈希、长度和 CRC；保留 ELF/map 供诊断。生成物不代替源代码，发布产物可以放版本发布目录。

## 体积和运行资源

对照 size、map、readelf 解释 Flash/RAM 使用；栈的 .su 信息与中断嵌套、调用路径一起分析，必要时上板测栈水位。链接的最小堆/栈预留不是运行时越界检测。

Release 优化可能改变单步、变量可见性和时序。能在 Debug 运行还不能跳过 Release 的升级与恢复验收。

## 最终验收矩阵

| 场景 | 预期 |
|---|---|
| 新文件未列入目标 | 能定位构建描述问题 |
| 镜像超过 32 KiB Boot 或 App 边界 | 链接或镜像检查拒绝 |
| 固件截断、CRC 错、错误地址 | 安装前拒绝 |
| 下载中断 | 旧 App 可用或进入恢复接收 |
| 元数据写入中断 | 选出最后有效记录 |
| 安装中断 | staging/backup 完整，可继续 |
| 新 App 首次启动失败 | 看门狗恢复控制，恢复旧版 |
| 回退中断 | 可重做，不丢唯一备份 |
| 正常升级后硬件复位 | 运行确认版本，中断和外设正常 |

扩展：真实性验证、防降级、加密升级包。先定义威胁和密钥保存方案，再选择签名/认证机制；不是简单加入 AES 函数就完成可信升级。

完成标准：你能用自己的源码、构建规则、产物和故障记录解释每一步，并在新的干净构建目录复现结果。
