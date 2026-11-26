#include "boot_debug.h"
#include "elog.h"

void Boot_DebugInit(void)
{
    if (elog_init() == ELOG_NO_ERR) {
        for (unsigned level = ELOG_LVL_ASSERT; level <= ELOG_LVL_VERBOSE; ++level) {
            elog_set_fmt(level, ELOG_FMT_LVL | ELOG_FMT_TAG);
        }
        elog_start();
    }
}
