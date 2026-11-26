#include <pspiofilemgr.h>
#include "savefile.h"
#include "save.h"

#define SAVE_DIR "ms0:/PSP/SAVEDATA/ECLIPSE"

static void path_for(int slot, char *out, int cap)
{
    /* slot 0..2 -> SAVE0.DAT .. SAVE2.DAT */
    int n = 0;
    const char *base = SAVE_DIR "/SAVE";
    while (base[n] && n < cap - 8) { out[n] = base[n]; n++; }
    out[n++] = (char)('0' + slot);
    out[n++] = '.'; out[n++] = 'D'; out[n++] = 'A'; out[n++] = 'T';
    out[n] = 0;
}

int savefile_exists(int slot)
{
    char path[64]; path_for(slot, path, sizeof(path));
    SceUID fd = sceIoOpen(path, PSP_O_RDONLY, 0);
    if (fd < 0) return 0;
    sceIoClose(fd);
    return 1;
}

int savefile_write(int slot, const GameState *g)
{
    static uint8_t buf[sizeof(GameState) + 64];
    size_t n = save_pack(g, buf, sizeof(buf));
    if (n == 0) return 0;
    sceIoMkdir("ms0:/PSP", 0777);
    sceIoMkdir("ms0:/PSP/SAVEDATA", 0777);
    sceIoMkdir(SAVE_DIR, 0777);
    char path[64]; path_for(slot, path, sizeof(path));
    SceUID fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    if (fd < 0) return 0;
    int written = sceIoWrite(fd, buf, n);
    sceIoClose(fd);
    return written == (int)n;
}

int savefile_read(int slot, GameState *g)
{
    static uint8_t buf[sizeof(GameState) + 64];
    char path[64]; path_for(slot, path, sizeof(path));
    SceUID fd = sceIoOpen(path, PSP_O_RDONLY, 0);
    if (fd < 0) return 0;
    int got = sceIoRead(fd, buf, sizeof(buf));
    sceIoClose(fd);
    if (got <= 0) return 0;
    return save_unpack(g, buf, (size_t)got);
}
