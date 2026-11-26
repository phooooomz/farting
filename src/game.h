#ifndef FANTASY_RPG_GAME_H
#define FANTASY_RPG_GAME_H

#define MAP_W 20
#define MAP_H 12

typedef struct {
    int x, y;
    int hp, max_hp;
    int attack;
    int level;
    int exp;
    int next_exp;
    int gel;
} Player;

typedef struct {
    int x, y;
    int hp, max_hp;
    int attack;
    int alive;
} Slime;

typedef struct {
    Player player;
    Slime slime;
    int message_id;
    char message[64];
} Game;

void game_init(Game *g);
void game_move(Game *g, int dx, int dy);
void game_attack(Game *g);
void game_save(Game *g);
void game_load(Game *g);
void game_update(Game *g);
void game_draw(Game *g);

#endif
