# Third-party source provenance

Source package supplied by the user:

```text
C:/Users/33481/Desktop/workspace/OTA/STM32F4xx_DSP_StdPeriph_Lib_V1.9.0
```

Package release: V1.9.0 / 27-January-2022. Several contained STM32F4 components identify themselves as V1.8.1; those original notices are retained.

Copied into this project:

- Libraries/CMSIS/Include/*.h from the package's matching CMSIS Include directory.
- Libraries/CMSIS/Device/ST/STM32F4xx/Include/*.h from the package.
- Libraries/STM32F4xx_StdPeriph_Driver, including original inc, src, release notes, and license.
- User/Src/system_stm32f4xx.c from Libraries/CMSIS/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.c.
- startup/startup_stm32f40_41xxx.s from the same template tree's gcc_ride7 directory.

Project adaptations:

1. system_stm32f4xx.c: use HSE_VALUE / 1000000U for PLL_M, yielding M=8 for the configured 8 MHz HSE. The source copy's original Windows-1252 comments were converted to UTF-8. The original source package is untouched.
2. startup_stm32f40_41xxx.s: select fpv4-sp-d16, explicitly initialize SP, invoke __libc_init_array before main, and place the vector table size directive after the table.
3. The new project's stm32f4xx_conf.h enables only the initially selected peripheral headers. It is a project configuration file, not a replacement of the vendor's original template.
4. All CMSIS headers and SPL driver files in Libraries retain their original contents.

License files:

- Package_license.md: the supplied package's license terms.
- LICENSE.txt: copied SPL component license notice, referring to the package terms.
- Libraries/STM32F4xx_StdPeriph_Driver/LICENSE.txt.
- Libraries/CMSIS/Device/ST/STM32F4xx/LICENSE.txt.
- Libraries/CMSIS/CMSIS_END_USER_LICENCE_AGREEMENT.pdf and .rtf.
- Copyright/license notices embedded in each source file.
