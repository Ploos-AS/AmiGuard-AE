#!/usr/bin/env bash
set -euo pipefail

IMAGE="${AMIGUARD_AE_BEBBO_IMAGE:-amigadev/m68k-amigaos-gcc@sha256:b18080e6ffca8f793e0f539536a9138e9d2a548ca1a301c7483f43ee15fedfed}"
OUT_DIR="${1:-build/fs-uae/native}"
mkdir -p "$OUT_DIR"

docker pull "$IMAGE"
docker image inspect "$IMAGE" --format '{{join .RepoDigests "\n"}}' | tee "$OUT_DIR/toolchain-image.txt"

docker run --rm \
  -v "$PWD:/work" \
  -w /work \
  "$IMAGE" \
  m68k-amigaos-gcc \
    -Iinclude -Isrc \
    -Os -Wall -Wextra -Werror -m68000 \
    -o AmiGuardAE.amiga \
    src/main.c \
    src/core.c \
    src/arexx_dispatch.c \
    src/arexx_amiga.c \
    -mcrt=nix20

cp AmiGuardAE.amiga "$OUT_DIR/AmiGuardAE"
file "$OUT_DIR/AmiGuardAE" | tee "$OUT_DIR/file.txt"
sha256sum "$OUT_DIR/AmiGuardAE" | tee "$OUT_DIR/AmiGuardAE.sha256"

if ! grep -Eiq 'AmigaOS|Amiga.*executable|loadseg' "$OUT_DIR/file.txt"; then
  echo "ERROR: native output is not recognized as an Amiga executable" >&2
  exit 1
fi

printf 'STATUS=PASS\nGATE=M1_NATIVE_BEBBO_BUILD\nIMAGE=%s\nBINARY=%s\n' \
  "$IMAGE" "$OUT_DIR/AmiGuardAE" | tee "$OUT_DIR/result.txt"
