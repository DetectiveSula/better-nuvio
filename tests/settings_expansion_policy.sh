#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
tmp=$(mktemp -d "${TMPDIR:-/tmp}/nuvio-expansion.XXXXXX")
trap 'rm -rf "$tmp"' EXIT
cc -std=c11 -O1 -g -Wall -Wextra -Werror -Isrc tests/settings_expansion_policy.c src/fontefiltro.c src/linguas.c -o "$tmp/test"
"$tmp/test"
