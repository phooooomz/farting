# ECLIPSE: Little Wonders of Aetheria

PSP Homebrew (Open World Action RPG) — Phase 1: build pipeline and basic loop.

## Project structure

```
eclipse-aetheria/
├── Makefile                  # PSP SDK build (produces EBOOT.PBP)
├── README.md
├── src/
│   ├── main.c                # Entry point, main loop, 30 FPS cap
│   ├── system.c/.h           # Exit callback (HOME button), kernel thread
│   ├── video.c/.h            # 480x272 framebuffer, double buffering, fill/rect
│   └── input.c/.h            # Controller state (buttons, pressed, analog)
└── data/
    └── localization/
        ├── th/               # Thai strings (Phase 2+, empty for now)
        └── en/               # English strings (Phase 2+, empty for now)
```

## Toolchain

- **PSP SDK (pspdev)**: provides `psp-gcc`, `psp-config`, `mksfoex`, `pack-pbp`
- Easiest route: the `pspdev/pspdev` Docker image
- Emulator for testing: **PPSSPP** (Settings → System → Enable PSP Homebrew / Emulation)

## Build

With Docker:

```sh
docker run --rm -v "$PWD":/src -w /src pspdev/pspdev:latest make
```

With a local pspdev install (`PSPDEV` set, `psp-config` on PATH):

```sh
make
```

Output: `EBOOT.PBP`

## Run in PPSSPP

1. Create the folder `PSP/GAME/ECLIPSE/` in PPSSPP's memory stick (`ms0`).
2. Copy `EBOOT.PBP` into it.
3. Open the game from PPSSPP's homebrew list.

## Phase 1 controls

| Input | Action |
|---|---|
| D-pad / Analog stick | Move the test marker |
| START | Exit |

## Status

- [x] Project structure and Makefile
- [x] Display init (480x272, 8888, double buffered)
- [x] Controller input
- [x] Main loop with 30 FPS cap
- [x] Build verified with pspdev (GitHub Actions artifact)
- [x] Tested in PPSSPP (boots, shows test marker)
- [x] Thai font rendering: Sarabun (OFL 1.1), generated atlas via tools/gen_font.py
- [x] Main menu, settings (language switch), Localization IDs (Thai default, English)
- [ ] Phases 3-15 (not started)

Phase 2 status: menu and Thai text are in place. Continue, Load, and Start (game screen) are placeholders; the game world is not built yet.

Fonts: `assets/font/Sarabun-Regular.ttf` and `OFL.txt` (SIL Open Font License 1.1). Regenerate with `python3 tools/gen_font.py assets/font/Sarabun-Regular.ttf 16 src/font_data.c`.
