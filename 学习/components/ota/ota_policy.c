#include "ota_policy.h"
Ota_Action Ota_SelectAction(Ota_State state, int app_vectors_valid)
{
    switch (state) {
    case OTA_NORMAL:
    case OTA_DOWNLOADING:
        return app_vectors_valid ? OTA_ACT_RUN_APP : OTA_ACT_RECEIVE;
    case OTA_DOWNLOADED: return OTA_ACT_INSTALL;
    case OTA_INSTALLING: return OTA_ACT_RESUME_INSTALL;
    case OTA_TRIAL_READY: return OTA_ACT_START_TRIAL;
    case OTA_TRIAL_RUNNING:
    case OTA_ROLLING_BACK: return OTA_ACT_ROLLBACK;
    default: return OTA_ACT_RECEIVE;
    }
}
