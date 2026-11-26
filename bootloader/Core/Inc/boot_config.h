#ifndef BOOT_CONFIG_H
#define BOOT_CONFIG_H

/* 片上 Flash 分区，地址和容量须与链接脚本及 BSP 扇区表一致。 */
/* Bootloader 起始地址，位于扇区 0。 */
#define BOOT_FLASH_BASE            0x08000000UL
/* Bootloader 分区容量，单位字节，覆盖扇区 0、1。 */
#define BOOT_FLASH_SIZE            (32UL * 1024UL)
/* App 运行区起始地址，也是 App 的中断向量表地址。 */
#define BOOT_APP_BASE              0x08008000UL
/* 密文暂存区起始地址，位于扇区 5。 */
#define BOOT_STAGE_BASE            0x08020000UL
/* STM32F407ZE 的 512 KiB 片上 Flash 结束地址，不包含该地址。 */
#define BOOT_FLASH_END             0x08080000UL
/* App 运行区总容量，含末尾提交记录，单位字节。 */
#define BOOT_APP_SIZE              (BOOT_STAGE_BASE - BOOT_APP_BASE)
/* 密文暂存区物理容量，单位字节。 */
#define BOOT_STAGE_SIZE            (BOOT_FLASH_END - BOOT_STAGE_BASE)
/* App 提交记录大小，包含长度、CRC32、长度反码和完成标记。 */
#define BOOT_RECORD_SIZE           16UL
/* App 提交记录起始地址，位于运行区末尾。 */
#define BOOT_RECORD_BASE           (BOOT_STAGE_BASE - BOOT_RECORD_SIZE)
/* 明文 App 最大长度，单位字节，留出提交记录空间。 */
#define BOOT_IMAGE_MAX             (BOOT_APP_SIZE - BOOT_RECORD_SIZE)

/* Step2 AES-256-CBC 参数，密钥和初始 IV 须与上位机打包配置一致。 */
/* AES 块长，单位字节，CBC 的 IV 同样占一个块。 */
#define BOOT_AES_BLOCK_SIZE        16U
/* AES-256 密钥长度，单位字节。 */
#define BOOT_AES_KEY_SIZE          32U
/* 教学密钥，按 ASCII 字节使用，结尾的字符串终止符不参与运算。 */
#define BOOT_AES_KEY_ASCII         "12121212121212121212121212121212"
/* CBC 初始 IV，按 ASCII 字节使用。每次安装复制一份供解密函数更新。 */
#define BOOT_AES_IV_ASCII          "1212121212121212"
/* Step2 的单块解密函数。它原地解密，并把 IV 更新为当前块的原始密文。 */
#define BOOT_AES_DECRYPT_BLOCK     Aes_IV_key256bit_Decode

/* Step2 包格式。头和正文使用同一条 CBC 链，正文由打包端补零至整块。 */
/* 密文头占一个 AES 块，单位字节。 */
#define BOOT_PACKAGE_HEADER_SIZE   BOOT_AES_BLOCK_SIZE
/* 解密头中明文 App 长度的字节偏移，字段为 4 字节小端值。 */
#define BOOT_PACKAGE_LENGTH_OFFSET 12U
/* 密文包上限，含加密头和对齐后的正文，单位字节。 */
#define BOOT_PACKAGE_MAX           (BOOT_PACKAGE_HEADER_SIZE + BOOT_IMAGE_MAX)

/* Bootloader 初始化后的目标系统时钟，单位 Hz。 */
#define BOOT_SYSTEM_CLOCK_HZ       168000000UL
/* 升级模式下 LED 的翻转间隔，单位毫秒。 */
#define BOOT_HEARTBEAT_MS          500UL

_Static_assert(BOOT_FLASH_BASE + BOOT_FLASH_SIZE == BOOT_APP_BASE,
               "Bootloader and App must be adjacent");
_Static_assert(BOOT_AES_BLOCK_SIZE == 16U && BOOT_AES_KEY_SIZE == 32U,
               "Step2 AES implementation requires 16-byte blocks and a 32-byte key");
_Static_assert(sizeof(BOOT_AES_KEY_ASCII) - 1U == BOOT_AES_KEY_SIZE,
               "AES key length mismatch");
_Static_assert(sizeof(BOOT_AES_IV_ASCII) - 1U == BOOT_AES_BLOCK_SIZE,
               "AES IV length mismatch");
_Static_assert(BOOT_PACKAGE_LENGTH_OFFSET + 4U <= BOOT_PACKAGE_HEADER_SIZE,
               "App length field must fit in the encrypted header");
_Static_assert(BOOT_IMAGE_MAX % BOOT_AES_BLOCK_SIZE == 0UL && BOOT_PACKAGE_MAX <= BOOT_STAGE_SIZE,
               "Aligned encrypted image must fit internal Flash staging");
#endif
