#!/usr/bin/env bash
set -euo pipefail

IMAGE="${AMIGUARD_AE_BEBBO_IMAGE:-amigadev/m68k-amigaos-gcc@sha256:b18080e6ffca8f793e0f539536a9138e9d2a548ca1a301c7483f43ee15fedfed}"
OUT_DIR="${1:-build/fs-uae/native}"
mkdir -p "$OUT_DIR"

docker pull "$IMAGE"
docker image inspect "$IMAGE" --format '{{join .RepoDigests "\n"}}' | tee "$OUT_DIR/toolchain-image.txt"

# Production binary: includes the real classic ARexx port implementation.
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
    src/scanner_bridge.c \
    -mcrt=nix20

# CI-only AROS smoke binary. AROS nightly does not ship the classic
# rexxsyslib.library expected by AmigaOS ARexx applications, so this binary
# exercises the exact native 68k dispatcher/core inside FS-UAE without
# pretending to qualify the production ARexx transport.
docker run --rm \
  -v "$PWD:/work" \
  -w /work \
  "$IMAGE" \
  m68k-amigaos-gcc \
    -DAMIGUARD_AE_AROS_SMOKE=1 \
    -Iinclude -Isrc \
    -Os -Wall -Wextra -Werror -m68000 \
    -o AmiGuardAE-aros-smoke.amiga \
    src/main.c \
    src/core.c \
    src/arexx_dispatch.c \
    src/scanner_bridge.c \
    -mcrt=nix20

cp AmiGuardAE.amiga "$OUT_DIR/AmiGuardAE"
cp AmiGuardAE-aros-smoke.amiga "$OUT_DIR/AmiGuardAE-aros-smoke"
file "$OUT_DIR/AmiGuardAE" | tee "$OUT_DIR/file.txt"
file "$OUT_DIR/AmiGuardAE-aros-smoke" | tee "$OUT_DIR/aros-smoke-file.txt"
sha256sum "$OUT_DIR/AmiGuardAE" | tee "$OUT_DIR/AmiGuardAE.sha256"
sha256sum "$OUT_DIR/AmiGuardAE-aros-smoke" | tee "$OUT_DIR/AmiGuardAE-aros-smoke.sha256"

if ! grep -Eiq 'AmigaOS|Amiga.*executable|loadseg' "$OUT_DIR/file.txt"; then
  echo "ERROR: production native output is not recognized as an Amiga executable" >&2
  exit 1
fi
if ! grep -Eiq 'AmigaOS|Amiga.*executable|loadseg' "$OUT_DIR/aros-smoke-file.txt"; then
  echo "ERROR: AROS smoke output is not recognized as an Amiga executable" >&2
  exit 1
fi

printf 'STATUS=PASS\nGATE=M2_1_NATIVE_BEBBO_BUILD\nIMAGE=%s\nPRODUCTION_BINARY=%s\nAROS_SMOKE_BINARY=%s\nAREXX_RUNTIME_QUALIFICATION=DEFERRED_LOCAL_CLASSIC_AMIGAOS\n' \
  "$IMAGE" "$OUT_DIR/AmiGuardAE" "$OUT_DIR/AmiGuardAE-aros-smoke" | tee "$OUT_DIR/result.txt"
