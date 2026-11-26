# ECLIPSE: Little Wonders of Aetheria

PSP Homebrew action RPG. Renders with GU (3D world), Thai text via a generated bitmap atlas.

## Layout

```
Makefile                 PSP build -> EBOOT.PBP (compiles src/*.c and src/core/*.c)
src/
  main.c                 title -> game flow, pause toggle (START)
  system.c               exit callback
  input.c                pad state (pressed edges)
  gfx.c                  GU setup, 3D boxes/ground, 2D rects, textured glyph text
  font.c                 UTF-8 decode, Thai mark placement, glyph lookup
  font_data.c            generated atlas (tools/gen_font.py, Sarabun OFL)
  locale.c               Localization IDs, Thai (default) and English
  menu.c                 title menu, settings (language)
  game.c                 world sim (movement, mobs, AI, quests, gather, pets), 3D draw, pause tabs
  savefile.c             3 save slots on ms0:/PSP/SAVEDATA/ECLIPSE
src/core/                portable logic (no PSP headers): stats, items, crafting, quests,
                         combat, world, pets, save format, RNG
tests/test_core.c        host unit tests for src/core
tests/run_tests.sh       build + run host tests (gcc only)
tools/gen_font.py        regenerate font_data.c
assets/font/             Sarabun-Regular.ttf + OFL.txt
```

## Controls

| Input | Action |
|---|---|
| Analog | move (camera-relative) |
| D-pad left/right | rotate camera |
| D-pad up/down | zoom |
| X | attack nearby monster, talk to NPC, buy potion, open crafting table, gather node |
| O | jump |
| Triangle | dodge (short invulnerability, uses stamina) |
| Square | magic skill (Bubble Blast, uses MP) |
| R (held) | run (uses stamina) |
| SELECT | mount (current pet with mount ability and friendship >= 300) |
| START | pause menu (L/R switch tab, X and D-pad act on current tab) |

## Build and test

- Host tests (no PSP SDK): `sh tests/run_tests.sh`
- PSP build: GitHub Actions builds `EBOOT.PBP` on every push (download the `EBOOT` artifact)

## Status

Done:
- Build pipeline, GU rendering, input, 30 FPS loop
- Thai text rendering (Sarabun atlas, combining marks centered on base), English fallback
- Title menu, settings (language switch), pause menu with 6 tabs
- Stats, levels (1-100), stat points, skill points
- Items, inventory (stack, split, sort, use, equip), 7 equipment slots
- Crafting table recipes, materials and gold cost
- Quests: story chain, gather, hunt, dungeon trigger; rewards; pets granted by quests
- Monsters with idle/wander/chase/attack/return/death, respawn timers, drops
- Boss (Pudding King) with 3 HP phases, appears while quest 6 is active
- Resource nodes with respawn; pets follow, feed (treats), mount speed-up
- 3 save slots (checksum-verified binary format)
- Region 0 (Bloomvale) and region 1 (Whispering Woods) with gated portal

Not done (placeholders or missing):
- Regions 2-5 (Snowberry Valley, Coral Coast, Crystal Caverns, Starcloud Kingdom): content and travel missing
- Dungeon floors (only the cave trigger), Forest Cave is a single box
- Models: all characters, monsters and props are colored boxes, not chibi/low-poly models
- Animations, sound, sky/weather (rain, snow, fog), fishing, farming, mining animation
- Skill tree, enchanting/upgrade/repair, shop selling, storage, NPC schedules, fast travel, minimap
- Status effects, weak points, dodge-based combat balance
- Localization: most UI strings are localized; item/monster/quest names are stored per language inside their tables
- Not run on PPSSPP yet (only host tests of core logic have been executed)
