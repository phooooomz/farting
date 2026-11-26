#ifndef ECLIPSE_SAVE_H
#define ECLIPSE_SAVE_H
#include <stddef.h>
#include <stdint.h>
#include "gamestate.h"

#define SAVE_MAGIC   0x45434C50u   /* 'ECLP' */
#define SAVE_VERSION 1u

/* Writes header + state + checksum into buf. Returns bytes written or 0 if buf is too small. */
size_t save_pack(const GameState *g, uint8_t *buf, size_t cap);
/* Validates magic, version and checksum, then copies into g. Returns 1 on success. */
int    save_unpack(GameState *g, const uint8_t *buf, size_t len);
size_t save_size(void);
#endif
