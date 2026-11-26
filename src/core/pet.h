#ifndef ECLIPSE_PET_H
#define ECLIPSE_PET_H
#include <stdint.h>

#define PET_COUNT 6
typedef struct {
    const char *name_th, *name_en;
    uint8_t can_mount;
    uint8_t finds_treasure;
} PetDef;

typedef struct {
    uint8_t owned;
    uint8_t level;        /* 1..20 */
    uint16_t exp;
    uint16_t friendship;  /* 0..1000 */
} PetState;

extern const PetDef pet_defs[PET_COUNT];
int  pet_feed(PetState *p, int food_value);     /* returns 1 if level up */
int  pet_can_mount(const PetState *p, int pet_id);
#endif
