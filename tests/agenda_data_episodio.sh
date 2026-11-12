#!/bin/bash
export TMPDIR="${TMPDIR:-/tmp}"
TMPDIR="${TMPDIR%/}"
# Schedule, detail tag and episode list agree on the next episode's day
# (the episode list's own `released` wins over TMDB air_date).
#
#   bash tests/agenda_data_episodio.sh
#
# NUVIO_DADOS APONTA PARA UMA PASTA TEMPORARIA, e o teste ABORTA se dados_dir()
# nao for exatamente ela. Um teste deste repositorio ja escreveu por cima dos
# dados reais do dono; a pasta some no fim, deu certo ou nao.
set -eu
cd "$(dirname "$0")/.."

NUVIO_DADOS=$(mktemp -d ${TMPDIR:-/tmp}/nuvio-agdataep-dados.XXXXXX)
export NUVIO_DADOS
trap 'rm -rf "$NUVIO_DADOS"' EXIT

sources=()
for source in src/*.c src/dts/*.c; do
  case "$source" in src/main.c|src/agenda.c) continue;; esac
  sources+=("$source")
done
# src/agenda.c e INCLUIDO pelo teste: `lerCorpoTv` e estatico e e o parse de
# verdade, o mesmo que extras.c executa com o corpo vindo da rede.
cc "${sources[@]}" tests/agenda_data_episodio.c -Isrc -o ${TMPDIR:-/tmp}/nuvio-agdataep \
  -O1 -g -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 \
  -L/opt/homebrew/lib -lSDL2 -lSDL2_image -lSDL2_ttf -lz -framework OpenGL \
  -Wall -Wextra -Wno-deprecated-declarations -Wno-macro-redefined
${TMPDIR:-/tmp}/nuvio-agdataep "$@"
