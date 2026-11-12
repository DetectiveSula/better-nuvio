#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
for mode in ordinary readonly login; do
 flags=()
 [ "$mode" = ordinary ] || flags+=(-DNV_VALIDATION_READONLY)
 [ "$mode" != login ] || flags+=(-DNV_VALIDATION_LOGIN)
 cc -std=c11 -Wall -Wextra -Werror ${flags[@]+"${flags[@]}"} -DNV_SUPABASE_URL='"https://mock.invalid"' -DNV_SUPABASE_ANON_KEY='"mock"' -Isrc src/nuvem.c tests/validation_readonly.c -o "$work/$mode"
 "$work/$mode"
done
if cc -DNDEBUG -DNV_VALIDATION_READONLY -Isrc -x c -c -o "$work/release.o" - <<< '#include "validation_readonly.h"' 2>"$work/release.log"; then
 echo 'FAIL: release guard absent'; exit 1
fi
grep -q 'Read-only validation must not' "$work/release.log"
echo 'PASS: release builds reject read-only validation flag'

if cc -DNV_VALIDATION_LOGIN -Isrc -x c -c -o "$work/unsafe.o" - <<< '#include "validation_readonly.h"' 2>"$work/unsafe.log"; then
 echo 'FAIL: login allowed without read-only guard'; exit 1
fi
grep -q 'Validation login requires read-only' "$work/unsafe.log"
if cc -DNDEBUG -DNV_VALIDATION_READONLY -DNV_VALIDATION_LOGIN -Isrc -x c -c -o "$work/release-login.o" - <<< '#include "validation_readonly.h"' 2>"$work/release-login.log"; then
 echo 'FAIL: release login guard absent'; exit 1
fi
grep -q 'Read-only validation must not' "$work/release-login.log"
echo 'PASS: login opt-in requires debug read-only build; ordinary and strict modes unchanged'
