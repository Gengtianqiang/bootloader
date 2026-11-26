#ifndef YMODEM_H
#define YMODEM_H
#include <stdbool.h>
#include <stdint.h>

/* One binary file per session. Data callbacks see only declared file bytes. */
typedef struct {
    bool (*read)(uint8_t *byte, uint32_t timeout_ms);
    void (*send)(uint8_t byte);
    bool (*begin)(uint32_t size);
    bool (*append)(const uint8_t *data, uint32_t count);
    bool (*finish)(void);
} YmodemPort;

typedef enum { YM_OK, YM_IDLE, YM_CANCELLED, YM_PROTOCOL, YM_IMAGE_ERROR } YmodemResult;
YmodemResult Ymodem_Receive(const YmodemPort *port);
#endif
