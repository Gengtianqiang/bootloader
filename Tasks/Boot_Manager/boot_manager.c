#include "boot_manager.h"
#include "boot_image.h"
#include "boot_jump.h"
#include "bsp_board.h"
#include "bsp_tick.h"
#include "elog.h"
#include "ymodem.h"
#include "SEGGER_RTT.h"


_Noreturn void Boot_ManagerRun(void)
{
    static const YmodemPort port = {
        .read = BSP_UartRead, .send = BSP_UartWrite,
        .begin = Boot_ImageBegin, .append = Boot_ImageAppend, .finish = Boot_ImageFinish
    };
    BSP_KeyInit();
    BSP_DelayMs(20U);
    if (!BSP_UpdateKeyPressed()) {
        (void)Boot_JumpToApp();
    }

    BSP_StatusTimerInit();
    BSP_UartInit();
    elog_i("boot", "Waiting for one Ymodem binary on USART1");
    for (;;) {

        YmodemResult result = Ymodem_Receive(&port);
        if (result == YM_OK) {
            BSP_UartDrain();
            elog_i("boot", "Transfer complete");
            (void)Boot_JumpToApp();
            elog_e("boot", "Image failed launch validation");
        } else if (result != YM_IDLE) {
            elog_w("boot", "Transfer stopped (code %u), waiting again", (unsigned)result);
        }
    }
}
