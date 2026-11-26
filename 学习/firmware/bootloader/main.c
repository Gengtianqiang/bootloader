#include "board.h"
#include "boot_jump.h"
#include "ota_policy.h"
volatile uint32_t g_boot_heartbeat;
int main(void)
{
    /* Later lessons: read persistent state and complete installation BEFORE
     * allowing this jump. Until then, no EEPROM or Flash writes are present. */
    if (COURSE_ENABLE_APP_JUMP &&
        Ota_SelectAction(OTA_NORMAL, Boot_AppVectorsValid()) == OTA_ACT_RUN_APP) {
        Boot_JumpToApp();
    }
    Board_Init();
    for (;;) {
        ++g_boot_heartbeat;
        Board_LedToggle(BOARD_LED_BOOT);
        Board_DelayMs(500U);
    }
}
