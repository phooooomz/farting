#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pspiofilemgr.h>
#include <pspiofilemgr.h>
#include "game.h"

static const char *save_path = "ms0:/PSP/SAVEDATA/FANTASYRPG/PLAYER.DAT";

static int wall_at(int x, int y) {
    if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) return 1;
    if (x == 0 || y == 0 || x == MAP_W - 1 || y == MAP_H - 1) return 1;
    if ((x == 7 && y >= 3 && y <= 7) || (y == 5 && x >= 11 && x <= 15)) return 1;
    return 0;
}

void game_init(Game *g) {
    memset(g, 0, sizeof(*g));
    g->player.x = 2; g->player.y = 2;
    g->player.hp = g->player.max_hp = 40;
    g->player.attack = 8;
    g->player.level = 1;
    g->player.next_exp = 30;
    g->slime.x = 5; g->slime.y = 3;
    g->slime.hp = g->slime.max_hp = 20;
    g->slime.attack = 3;
    g->slime.alive = 1;
    snprintf(g->message, sizeof(g->message), "Welcome! Explore and defeat the Slime.");
}

void game_move(Game *g, int dx, int dy) {
    int nx = g->player.x + dx, ny = g->player.y + dy;
    if (wall_at(nx, ny)) return;
    if (g->slime.alive && nx == g->slime.x && ny == g->slime.y) {
        snprintf(g->message, sizeof(g->message), "Slime blocks the way. Press X to attack!");
        return;
    }
    g->player.x = nx; g->player.y = ny;
}

void game_attack(Game *g) {
    Slime *m = &g->slime;
    Player *p = &g->player;
    if (!m->alive) {
        snprintf(g->message, sizeof(g->message), "No monster here. Slime defeated!");
        return;
    }
    int distance = abs(p->x - m->x) + abs(p->y - m->y);
    if (distance != 1) {
        snprintf(g->message, sizeof(g->message), "Move next to the Slime to attack.");
        return;
    }
    m->hp -= p->attack;
    snprintf(g->message, sizeof(g->message), "You hit Slime for %d damage!", p->attack);
    if (m->hp <= 0) {
        m->alive = 0;
        p->exp += 12;
        if (rand() % 2) { p->gel++; snprintf(g->message, sizeof(g->message), "Slime defeated! +12 EXP, Gel +1"); }
        else snprintf(g->message, sizeof(g->message), "Slime defeated! +12 EXP");
        while (p->exp >= p->next_exp) {
            p->exp -= p->next_exp;
            p->level++;
            p->next_exp = (p->next_exp * 3) / 2;
            p->max_hp += 10;
            p->hp = p->max_hp;
            p->attack += 2;
            snprintf(g->message, sizeof(g->message), "LEVEL UP! Level %d - HP restored!", p->level);
        }
    } else {
        p->hp -= m->attack;
        if (p->hp < 0) p->hp = 0;
        snprintf(g->message, sizeof(g->message), "Hit %d! Slime counterattacks: -%d HP", p->attack, m->attack);
    }
}

void game_save(Game *g) {
    sceIoMkdir("ms0:/PSP/SAVEDATA", 0777);
    sceIoMkdir("ms0:/PSP/SAVEDATA/FANTASYRPG", 0777);
    FILE *f = fopen(save_path, "wb");
    if (!f) {
        snprintf(g->message, sizeof(g->message), "Save failed (folder unavailable). Try Memory Stick.");
        return;
    }
    fwrite(&g->player, sizeof(Player), 1, f);
    fwrite(&g->slime, sizeof(Slime), 1, f);
    fclose(f);
    snprintf(g->message, sizeof(g->message), "Saved! (Memory Stick)");
}

void game_load(Game *g) {
    FILE *f = fopen(save_path, "rb");
    if (!f) return;
    Player p; Slime m;
    if (fread(&p, sizeof(Player), 1, f) == 1 && fread(&m, sizeof(Slime), 1, f) == 1) {
        g->player = p; g->slime = m;
        snprintf(g->message, sizeof(g->message), "Save loaded.");
    }
    fclose(f);
}

void game_update(Game *g) {
    if (g->player.hp <= 0) {
        g->player.hp = g->player.max_hp;
        g->player.x = 2; g->player.y = 2;
        snprintf(g->message, sizeof(g->message), "You fainted! Returned to camp; HP restored.");
    }
}

void game_draw(Game *g) {
    int x, y;
    printf("\n\n\n  FANTASY RPG - PROTOTYPE\n");
    printf("  Lv.%d  HP %d/%d  ATK %d  EXP %d/%d  Gel:%d\n\n", g->player.level, g->player.hp, g->player.max_hp, g->player.attack, g->player.exp, g->player.next_exp, g->player.gel);
    for (y = 0; y < MAP_H; y++) {
        printf("  ");
        for (x = 0; x < MAP_W; x++) {
            if (g->player.x == x && g->player.y == y) putchar('@');
            else if (g->slime.alive && g->slime.x == x && g->slime.y == y) putchar('S');
            else if (wall_at(x, y)) putchar('#');
            else putchar('.');
        }
        putchar('\n');
    }
    printf("\n  @ Player  S Slime  # Wall  . Grass\n");
    printf("  %s\n", g->message);
    printf("\n  D-pad Move | X Attack | Triangle Save | Start Quit\n");
}
