#!/bin/sh
# Smoke test: run EBOOT.PBP in PPSSPP under a virtual display, screenshot it, check it does not crash.
# Usage: tests/ppsspp_smoke.sh path/to/EBOOT.PBP out_dir
set -u
EBOOT="$1"
OUT="$2"
mkdir -p "$OUT"

PPSSPP_BIN=""
for c in PPSSPPSDL ppsspp PPSSPP; do
  if command -v "$c" >/dev/null 2>&1; then PPSSPP_BIN="$c"; break; fi
done
if [ -z "$PPSSPP_BIN" ]; then
  PPSSPP_BIN="$(dpkg -L ppsspp 2>/dev/null | grep -E '/(PPSSPPSDL|ppsspp)$' | head -1)"
fi
if [ -z "$PPSSPP_BIN" ]; then
  echo "PPSSPP not found" | tee "$OUT/result.txt"; exit 2
fi

export LIBGL_ALWAYS_SOFTWARE=1
# Run for 25s. timeout exit 124 = still running after 25s (no crash).
xvfb-run -a -s "-screen 0 1280x800x24" sh -c "
  '$PPSSPP_BIN' '$EBOOT' > '$OUT/ppsspp.log' 2>&1 &
  PID=\$!
  sleep 20
  import -window root '$OUT/screen.png' 2>/dev/null || true
  sleep 5
  if kill -0 \$PID 2>/dev/null; then echo ALIVE > '$OUT/status.txt'; kill \$PID; exit 0; else wait \$PID; echo \"EXITED \$?\" > '$OUT/status.txt'; exit 1; fi
"
RC=$?
cat "$OUT/status.txt" 2>/dev/null | tee "$OUT/result.txt"
exit $RC
