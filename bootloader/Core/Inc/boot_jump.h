#ifndef BOOT_JUMP_H
#define BOOT_JUMP_H
#include <stdint.h>
/* Call only from privileged Thread mode. Returns 0 if validation fails;
 * a successful transfer never returns. Checks the commit record, vectors and image CRC32. */
uint32_t Boot_JumpToApp(void);
#endif
