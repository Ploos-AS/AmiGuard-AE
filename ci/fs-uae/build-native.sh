#!/usr/bin/env bash
set -euo pipefail

IMAGE="${AMIGUARD_AE_BEBBO_IMAGE:-amigadev/m68k-amigaos-gcc@sha256:b18080e6ffca8f793e0f539536a9138e9d2a548ca1a301c7483f43ee15fedfed}"
AMIGUARD_COMMIT="5f30e456cb7cfcef156d06c2ac68b52d6ad5723b"
OUT_DIR="${1:-build/fs-uae/native}"
AMIGUARD_DIR="deps/AmiGuard"
mkdir -p "$OUT_DIR" deps

if [[ ! -d "$AMIGUARD_DIR/.git" ]]; then
  git clone https://github.com/Ploos-AS/AmiGuard.git "$AMIGUARD_DIR"
fi
git -C "$AMIGUARD_DIR" fetch --depth 1 origin "$AMIGUARD_COMMIT"
git -C "$AMIGUARD_DIR" checkout --detach "$AMIGUARD_COMMIT"
printf '%s\n' "$AMIGUARD_COMMIT" > "$OUT_DIR/amiguard-engine-commit.txt"

docker pull "$IMAGE"
docker image inspect "$IMAGE" --format '{{join .RepoDigests "\n"}}' | tee "$OUT_DIR/toolchain-image.txt"

COMMON_SOURCES=(
  src/core.c
  src/arexx_dispatch.c
  src/scanner_bridge.c
  src/signature_bridge.c
  src/result_store.c
  src/checksum.c
  src/identify.c
  "$AMIGUARD_DIR/src/file_intake.c"
  "$AMIGUARD_DIR/src/file_signatures.c"
  "$AMIGUARD_DIR/src/hunk.c"
  "$AMIGUARD_DIR/src/signatures.c"
)

docker run --rm -v "$PWD:/work" -w /work "$IMAGE" \
  m68k-amigaos-gcc \
    -DAMIGUARD_AE_WITH_AMIGUARD=1 \
    -Iinclude -Isrc -I"$AMIGUARD_DIR/src" \
    -Os -Wall -Wextra -Werror -m68000 \
    -o AmiGuardAE.amiga \
    src/main.c src/arexx_amiga.c "${COMMON_SOURCES[@]}" \
    -mcrt=nix20

docker run --rm -v "$PWD:/work" -w /work "$IMAGE" \
  m68k-amigaos-gcc \
    -DAMIGUARD_AE_WITH_AMIGUARD=1 \
    -Iinclude -Isrc -I"$AMIGUARD_DIR/src" \
    -Os -Wall -Wextra -Werror -m68000 \
    -o AmiGuardAE-aros-smoke.amiga \
    ci/fs-uae/aros_smoke_main.c "${COMMON_SOURCES[@]}" \
    -mcrt=nix20

cp AmiGuardAE.amiga "$OUT_DIR/AmiGuardAE"
cp AmiGuardAE-aros-smoke.amiga "$OUT_DIR/AmiGuardAE-aros-smoke"
file "$OUT_DIR/AmiGuardAE" | tee "$OUT_DIR/file.txt"
file "$OUT_DIR/AmiGuardAE-aros-smoke" | tee "$OUT_DIR/aros-smoke-file.txt"
sha256sum "$OUT_DIR/AmiGuardAE" | tee "$OUT_DIR/AmiGuardAE.sha256"
sha256sum "$OUT_DIR/AmiGuardAE-aros-smoke" | tee "$OUT_DIR/AmiGuardAE-aros-smoke.sha256"

if ! grep -Eiq 'AmigaOS|Amiga.*executable|loadseg' "$OUT_DIR/file.txt"; then exit 1; fi
if ! grep -Eiq 'AmigaOS|Amiga.*executable|loadseg' "$OUT_DIR/aros-smoke-file.txt"; then exit 1; fi

printf 'STATUS=PASS\nGATE=M3_1_NATIVE_BEBBO_BUILD\nIMAGE=%s\nAMIGUARD_COMMIT=%s\nPRODUCTION_BINARY=%s\nAROS_SMOKE_BINARY=%s\nAREXX_RUNTIME_QUALIFICATION=DEFERRED_LOCAL_CLASSIC_AMIGAOS\n' \
  "$IMAGE" "$AMIGUARD_COMMIT" "$OUT_DIR/AmiGuardAE" "$OUT_DIR/AmiGuardAE-aros-smoke" | tee "$OUT_DIR/result.txt"
