#ifndef COURSE_OTA_POLICY_H
#define COURSE_OTA_POLICY_H
/* Teaching policy only. No EEPROM, Flash writes, transfer or rollback driver. */
typedef enum {
    OTA_NORMAL, OTA_DOWNLOADING, OTA_DOWNLOADED,
    OTA_INSTALLING, OTA_TRIAL_READY, OTA_TRIAL_RUNNING, OTA_ROLLING_BACK
} Ota_State;
typedef enum {
    OTA_ACT_RUN_APP, OTA_ACT_RECEIVE, OTA_ACT_INSTALL,
    OTA_ACT_RESUME_INSTALL, OTA_ACT_START_TRIAL, OTA_ACT_ROLLBACK
} Ota_Action;
Ota_Action Ota_SelectAction(Ota_State state, int app_vectors_valid);
#endif
