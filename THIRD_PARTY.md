# Third-party source provenance

Source package supplied by the user:

```text
C:/Users/33481/Desktop/workspace/OTA/STM32F4xx_DSP_StdPeriph_Lib_V1.9.0
```

Package release: V1.9.0 / 27-January-2022. Several contained STM32F4 components identify themselves as V1.8.1; those original notices are retained.

Copied into this project:

- Drivers/CMSIS/Include/*.h from the package's matching CMSIS Include directory.
- Drivers/CMSIS/Device/ST/STM32F4xx/Include/*.h from the package.
- Drivers/STM32F4xx_StdPeriph_Driver, including original inc, src, release notes, and license.
- Core/Src/system_stm32f4xx.c from Libraries/CMSIS/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.c.
- System/Startup/startup_stm32f40_41xxx.s from the same template tree's gcc_ride7 directory.

Project adaptations:

1. system_stm32f4xx.c: use HSE_VALUE / 1000000U for PLL_M, yielding M=8 for the configured 8 MHz HSE. The source copy's original Windows-1252 comments were converted to UTF-8. The original source package is untouched.
2. startup_stm32f40_41xxx.s: select fpv4-sp-d16, explicitly initialize SP, invoke __libc_init_array before main, and place the vector table size directive after the table.
3. The new project's stm32f4xx_conf.h enables only the initially selected peripheral headers. It is a project configuration file, not a replacement of the vendor's original template.
4. All CMSIS headers and SPL driver files in Drivers retain their original contents.

License files:

- Package_license.md: the supplied package's license terms.
- LICENSE.txt: copied SPL component license notice, referring to the package terms.
- Drivers/STM32F4xx_StdPeriph_Driver/LICENSE.txt.
- Drivers/CMSIS/Device/ST/STM32F4xx/LICENSE.txt.
- Drivers/CMSIS/CMSIS_END_USER_LICENCE_AGREEMENT.pdf and .rtf.
- Copyright/license notices embedded in each source file.

## Step3 ports

- Middlewares/Rtt contains SEGGER RTT from the reference Bootloader_Test. Original notices are retained.
- Middlewares/easylogger contains Armink's MIT-licensed synchronous logger. elog_cfg.h and the RTT port are adapted for this board.
- System/Linker/otatest_app.ld is derived from the sibling otatest/STM32F407xx_FLASH.ld. Only the Flash region is relocated and shortened.
- The board transport, bounded Ymodem receiver, Flash image record and boot manager are project implementations for STM32F407ZE.
