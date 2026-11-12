#!/usr/bin/env bash
set -eo pipefail
cd "$(dirname "$0")/.."
work=$(mktemp -d "${TMPDIR:-/tmp}/nuvio-better-updater.XXXXXX")
trap 'rm -rf "$work"' EXIT
for variant in full lite-arm64 lite-armv7; do
 flags=()
 case "$variant" in lite-arm64) flags=(-DNV_APK_LITE=1);; lite-armv7) flags=(-DNV_APK_LITE=1 -DTEST_ARMV7);; esac
 cc "${flags[@]}" tests/atualizacao_better_android.c src/js.c -Isrc -o "$work/test" \
  -O1 -g -ffunction-sections -fdata-sections -Wl,-dead_strip \
  -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 -L/opt/homebrew/lib -lSDL2 -lm
 "$work/test"
done
