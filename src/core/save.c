#include <string.h>
#include "save.h"

#define HEADER_BYTES 12   /* magic, version, payload size */

static uint32_t checksum(const uint8_t *p, size_t n)
{
    uint32_t h = 2166136261u;           /* FNV-1a */
    for (size_t i = 0; i < n; i++) { h ^= p[i]; h *= 16777619u; }
    return h;
}

static void put32(uint8_t *p, uint32_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24); }
static uint32_t get32(const uint8_t *p) { return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24); }

size_t save_size(void) { return HEADER_BYTES + sizeof(GameState) + 4; }

size_t save_pack(const GameState *g, uint8_t *buf, size_t cap)
{
    size_t total = save_size();
    if (cap < total) return 0;
    put32(buf, SAVE_MAGIC);
    put32(buf + 4, SAVE_VERSION);
    put32(buf + 8, (uint32_t)sizeof(GameState));
    memcpy(buf + HEADER_BYTES, g, sizeof(GameState));
    put32(buf + HEADER_BYTES + sizeof(GameState), checksum(buf, HEADER_BYTES + sizeof(GameState)));
    return total;
}

int save_unpack(GameState *g, const uint8_t *buf, size_t len)
{
    if (len < save_size()) return 0;
    if (get32(buf) != SAVE_MAGIC || get32(buf + 4) != SAVE_VERSION) return 0;
    if (get32(buf + 8) != sizeof(GameState)) return 0;
    uint32_t stored = get32(buf + HEADER_BYTES + sizeof(GameState));
    if (checksum(buf, HEADER_BYTES + sizeof(GameState)) != stored) return 0;
    memcpy(g, buf + HEADER_BYTES, sizeof(GameState));
    return 1;
}
