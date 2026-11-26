#include "boot_manager.h"
#include "boot_image.h"
#include "boot_jump.h"
#include "bsp_board.h"
#include "bsp_tick.h"
#include "elog.h"
#include "boot_ymodem.h"


_Noreturn void Boot_ManagerRun(void)
{
    static const YmodemPort port = {
        .read   =     BSP_UartRead, 
        .send   =    BSP_UartWrite,
        .begin  =  Boot_ImageBegin, 
        .append = Boot_ImageAppend, 
        .finish = Boot_ImageFinish
    };

    BSP_KeyInit();
    BSP_DelayMs(20U);
    if (!BSP_UpdateKeyPressed()) {
        (void)Boot_JumpToApp();
    }

    BSP_StatusTimerInit();
    BSP_UartInit();
    elog_i("boot", "Waiting for Ymodem package");
    for (;;) {
        YmodemResult result = Ymodem_Receive(&port);
        if (result == YM_OK) {
            BSP_UartDrain();
            elog_i("boot", "Package staged, decrypting and installing");
            if (Boot_ImageInstall()) {
                elog_i("boot", "Image installed,AES success, jumping to App");
                (void)Boot_JumpToApp();
                elog_e("boot", "Image failed launch validation");
            } else {
                elog_e("boot", "Package install failed, waiting again");
            }
        } else if (result != YM_IDLE) {
            elog_w("boot", "Transfer stopped (code %u), waiting again", (unsigned)result);
        }
    }
}
