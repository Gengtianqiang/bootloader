# otatest

`otatest` 是独立的 STM32F407ZE HAL 应用工程。在 VS Code 中打开此目录，在 CMake Tools 中选择预设。

| 预设 | 用途 | Flash 起点 |
| --- | --- | --- |
| `App-Debug` | 生成用于 Step2 OTA 的 App | `0x08008000` |
| `Debug`、`Release` | 单独烧录 App | `0x08000000` |

`App-Debug` 使用 `STM32F407xx_OTA.ld`，App 最大为 98288 字节。运行区末尾 16 字节由 Bootloader 保存提交记录。此预设也为 App 的 `SystemInit()` 设置向量表偏移 `0x8000`，使 Cortex-M4 系统控制块的 `SCB->VTOR` 指向 `0x08008000`。默认 F5 配置使用独立烧录的 `Debug` 产物，在装有 Bootloader 的板上执行会覆盖 Bootloader 区域。

## 生成 Step2 升级包

在本目录执行以下命令。CMake 构建生成调试用 ELF 和明文 BIN，打包步骤使用 Step2 教学密钥和包格式生成密文。

```powershell
cmake --preset App-Debug
cmake --build --preset App-Debug
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\tools\Pack-Step2.ps1 -InputBin .\build\App-Debug\otatest.bin -OutputPackage .\build\App-Debug\otatest.step2.bin
```

板上进入升级模式后，用 Ymodem 发送 `build/App-Debug/otatest.step2.bin`。Bootloader 通过串口接收并安装这个文件。

也可以在 VS Code 中打开本工程，点击状态栏的 **生成 OTA 包**。该按钮会先构建 `App-Debug`，再从最新的 `otatest.elf` 导出明文 BIN 并加密，输出仍为 `build/App-Debug/otatest.step2.bin`。构建或 ELF 地址检查失败时，不会生成新的升级包。首次安装状态栏按钮时，在 PowerShell 执行以下命令，然后重新加载 VS Code 窗口。

```powershell
$vsix = Join-Path $env:TEMP 'jlink-firmware-download-1.2.0.vsix'
python tools/package-jlink-button-extension.py $vsix
code --install-extension $vsix
```

## 用 VS Code 一键下载 App

打开本工程后，点击状态栏的 **下载 App**。这个按钮依次配置并构建 `App-Debug`、生成烧录 HEX、运行 J-Link Commander。通过 SWD 烧录成功后 MCU 复位并运行。进度显示在 VS Code 任务终端。

烧录文件包含从 `0x08008000` 开始的 App 和从 `0x0801FFF0` 开始的 16 字节提交记录，供板上 Step2 Bootloader 在复位后校验并启动 App。这个方式使用 J-Link 直接写入运行区。串口升级继续发送上面的 Step2 密文包。
