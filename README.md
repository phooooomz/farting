# Fantasy RPG PSP - Starter Scaffold

A free/open-source starter project for a small PSP Homebrew RPG prototype, intended to be built on GitHub Actions from an Android phone and played in PPSSPP.

## Current prototype features
- PSP controls (D-pad movement, X attack, Triangle save, Start exit)
- Small tile-map with grass, walls, player, and a Slime
- Player HP, attack, level and EXP
- Slime combat, EXP gain, random item drops (Gel)
- Basic save file (`ms0:/PSP/SAVEDATA/FANTASYRPG/PLAYER.DAT`)
- PSPDEV/PSPSDK Makefile and GitHub Actions workflow

This is a **single-player prototype scaffold**, not an online MMORPG yet. Networking, server authority, accounts, trading, party/guild and larger world systems are planned future milestones.

## Build from Android (no local compiler needed)
1. Create a GitHub repository and upload these project files (or upload the ZIP contents).
2. Open the repository's **Actions** tab.
3. Run the `Build PSP Homebrew` workflow, or push a commit to `main`.
4. Open the successful workflow run and download artifact `FantasyRPG-EBOOT`.
5. Extract it and copy `EBOOT.PBP` into `PSP/GAME/FANTASYRPG/` on your device, or open the PBP from PPSSPP.

The workflow uses the PSPDEV Docker toolchain image. If upstream image availability changes, the workflow may need adjustment.

## Controls
- D-pad: move
- X: attack an adjacent Slime
- Triangle: save
- Start: exit

## Build locally (Linux with PSPDEV installed)
```sh
export PSPDEV=/usr/local/pspdev
export PATH="$PATH:$PSPDEV/bin"
make
```
The output is `EBOOT.PBP`.

## Project layout
```text
src/main.c              main loop, input and rendering
src/game.h              shared game data / declarations
src/game.c              movement, combat, leveling, drops, saving
Makefile                PSPSDK build recipe
.github/workflows/build.yml  cloud build workflow
assets/                 reserved for art/audio
```

## Roadmap
1. Prototype controls, Slime, EXP, drop, save (this scaffold)
2. Inventory screen, item definitions, equipment and crafting
3. More monsters, NPCs, quests, map transitions and dungeon
4. Save/load expansion and Thai text/font support
5. Multiplayer research/prototype: PSP networking and a server; not assumed to be MMO-ready

## License
MIT. See `LICENSE`.
