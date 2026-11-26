# Cmake_OTA_StandardTemplate

STM32F407ZE bootloader and application learning workspace. Firmware projects are independent CMake projects with separate entry points and build directories.

| Directory | Purpose | Build entry |
| --- | --- | --- |
| `bootloader/` | SPL bootloader, internal-Flash staging, Ymodem, Step2 AES-CBC install, and J-Link Commander button | `cmake --preset Debug` |
| `otatest/` | Independent HAL App, OTA linker layout, Step2 package generation, and App download button | `cmake --preset App-Debug` |
| `学习/` | CMake, Cortex-M4 startup, linker layout, and OTA course materials | See `学习/README.md` |

Open this root directory in VS Code to navigate the full workspace. Configure and build each firmware project from its own directory.

For serial OTA, build the App with the `App-Debug` preset, generate `otatest/build/App-Debug/otatest.step2.bin` with the App's **生成 OTA 包** button, place the Bootloader in its Ymodem receive mode, and send the encrypted package.

The Step2 AES key and IV remain the demonstration values used by the course reference. They are not production firmware authentication credentials.
