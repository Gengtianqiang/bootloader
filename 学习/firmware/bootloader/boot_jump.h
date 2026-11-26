#ifndef COURSE_BOOT_JUMP_H
#define COURSE_BOOT_JUMP_H
int Boot_AppVectorsValid(void);
/* Invalid App/context: returns. Valid transfer: never returns. */
void Boot_JumpToApp(void);
#endif
