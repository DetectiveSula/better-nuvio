#!/bin/bash
# Profile picker: the opaque full-screen clear of desenhaFundo() must not be
# issued when psestilos (Filmes/Luz/Projetor) repaints the whole screen opaque
# right after it. Counts full-screen draws of the picker in the Mac harness.
#
#   bash tests/perfil_fundo_cobre.sh [/other/tree]   # FAIL on a tree before the fix
set -euo pipefail
aqui="$(cd "$(dirname "$0")/.." && pwd)"
raiz="${1:-$aqui}"
# NV_PERF_SEM_ADDON: the count below was calibrated on the harness's empty Home
# (see povoar in tests/fluidez_perf.c).
out="$(NV_PERF_SEM_ADDON=1 NV_CENARIO=imersiva bash "$aqui/tests/fluidez_perf.sh" "$raiz" 2>&1)"
linha="$(printf '%s\n' "$out" | grep -E '^\[imersiva\] perfil +upd=' | head -1)"
modo2="$(printf '%s\n' "$out" | grep -E '^\[imersiva\] perfil +fill por modo' | head -1 | sed -E 's/.* 2=([0-9.]+).*/\1/')"
cheios="$(printf '%s\n' "$linha" | sed -E 's/.*cheios=([0-9.]+).*/\1/')"
echo "perfil: cheios=$cheios fill-cor=$modo2"
# cheios <= 3: the wall layer is drawn offscreen at 1280x720 and composited in
# one opaque full-screen pass (psestilos.c, PAR_BAIXA_*), which the layout-area
# count adds to the wall's own two. fill-cor still catches a covered clear.
if awk -v c="$cheios" -v m="$modo2" 'BEGIN { exit !(c != "" && m != "" && c <= 3.0 && m <= 1.01) }'; then
  echo "PASS: perfil picker draws one opaque full-screen clear"
else
  echo "FAIL: perfil picker draws a covered full-screen clear"; exit 1
fi
