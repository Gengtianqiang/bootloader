#include "ymodem.h"
#include <string.h>

enum { SOH = 1, STX = 2, EOT = 4, ACK = 6, NAK = 21, CAN = 24, CRC_REQUEST = 'C' };
enum { WAIT_MS = 1000, BYTE_MS = 200, RETRIES = 10 };
typedef enum { HEADER, DATA, SECOND_EOT, END_HEADER } Phase;
typedef struct { uint8_t seq, data[1024]; unsigned length; } Packet;

static uint16_t crc16(const uint8_t *p, unsigned length)
{
    uint16_t crc = 0;
    while (length--) {
        crc ^= (uint16_t)*p++ << 8;
        for (unsigned bit = 0; bit < 8; ++bit) {
            crc = (crc & 0x8000U) ? (crc << 1) ^ 0x1021U : (unsigned)crc << 1;
        }
    }
    return crc;
}

static bool packet_read(const YmodemPort *io, uint8_t lead, Packet *packet)
{
    uint8_t inv, hi, lo;
    packet->length = lead == STX ? 1024U : 128U;
    if (!io->read(&packet->seq, BYTE_MS) || !io->read(&inv, BYTE_MS)) return false;
    for (unsigned i = 0; i < packet->length; ++i) {
        if (!io->read(&packet->data[i], BYTE_MS)) return false;
    }
    return io->read(&hi, BYTE_MS) && io->read(&lo, BYTE_MS) &&
           (packet->seq ^ inv) == 255 && crc16(packet->data, packet->length) ==
           (uint16_t)(((unsigned)hi << 8) | lo);
}

static bool file_size(const Packet *p, uint32_t *size)
{
    const uint8_t *end = p->data + p->length;
    const uint8_t *s = memchr(p->data, 0, p->length);
    if (!s || s == p->data || ++s == end || *s < '0' || *s > '9') return false;
    uint32_t n = 0;
    while (s < end && *s >= '0' && *s <= '9') {
        unsigned digit = *s++ - '0';
        if (n > (UINT32_MAX - digit) / 10U) return false;
        n = 10U * n + digit;
    }
    if (s == end || (*s != 0 && *s != ' ') || n == 0) return false;
    *size = n;
    return true;
}

static YmodemResult cancel(const YmodemPort *io, YmodemResult reason)
{
    io->send(CAN);
    io->send(CAN);
    return reason;
}

YmodemResult Ymodem_Receive(const YmodemPort *io)
{
    static Packet p;
    Phase phase = HEADER;
    uint8_t expected = 1;
    uint32_t remaining = 0;
    bool any_data = false;
    unsigned failures = 0;

    io->send(CRC_REQUEST);
    while (failures < RETRIES) {
        uint8_t lead;
        bool available = io->read(&lead, WAIT_MS);
        if (!available && phase == HEADER && failures == 0) return YM_IDLE;
        if (available && lead == CAN) {
            uint8_t second;
            if (io->read(&second, BYTE_MS) && second == CAN) return YM_CANCELLED;
        } else if (available && lead == EOT && !remaining && any_data) {
            if (phase == DATA) {
                phase = SECOND_EOT;
                io->send(NAK);
            } else {
                phase = END_HEADER;
                io->send(ACK);
                io->send(CRC_REQUEST);
            }
            if (++failures >= RETRIES) break;
            continue;
        } else if (available && (lead == SOH || lead == STX) && packet_read(io, lead, &p)) {
            if (phase == HEADER && p.seq == 0) {
                if (!p.data[0]) { io->send(ACK); return YM_IDLE; }
                if (!file_size(&p, &remaining) || !io->begin(remaining))
                    return cancel(io, YM_IMAGE_ERROR);
                phase = DATA;
                io->send(ACK);
                io->send(CRC_REQUEST);
                failures = 0;
                continue;
            }
            if (phase == DATA && p.seq == expected && remaining) {
                uint32_t count = remaining < p.length ? remaining : p.length;
                if (!io->append(p.data, count)) return cancel(io, YM_IMAGE_ERROR);
                remaining -= count;
                any_data = true;
                ++expected;  /* Ymodem sequence numbers wrap modulo 256. */
                io->send(ACK);
                failures = 0;
                continue;
            }
            if (phase == DATA && p.seq == (uint8_t)(expected - 1U)) {
                /* ACK lost: leave Flash and the receive offset untouched. */
                io->send(ACK);
                if (!any_data) io->send(CRC_REQUEST);
                ++failures;
                continue;
            }
            if (phase == END_HEADER && p.seq == 0) {
                if (p.data[0]) return cancel(io, YM_PROTOCOL);
                if (!io->finish()) return cancel(io, YM_IMAGE_ERROR);
                io->send(ACK);
                /* Reply again if the sender retransmits the final empty block. */
                for (unsigned i = 0; i < RETRIES && io->read(&lead, WAIT_MS * 2U); ++i) {
                    if ((lead == SOH || lead == STX) && packet_read(io, lead, &p) &&
                        p.seq == 0 && !p.data[0]) io->send(ACK);
                }
                return YM_OK;
            }
        }
        ++failures;
        if (phase == END_HEADER) io->send(CRC_REQUEST);
        else if (phase == HEADER || (phase == DATA && !any_data && !available)) io->send(CRC_REQUEST);
        else io->send(NAK);
    }
    return cancel(io, YM_PROTOCOL);
}
