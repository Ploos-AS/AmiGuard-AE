#!/usr/bin/env bash
set -euo pipefail
IMAGE="${AMIGUARD_AE_BEBBO_IMAGE:-amigadev/m68k-amigaos-gcc@sha256:b18080e6ffca8f793e0f539536a9138e9d2a548ca1a301c7483f43ee15fedfed}"
AMIGUARD_COMMIT="da4455200e6e29ced29edd4ea1cfff722af4c0a2"
ED25519_COMMIT="b1f19fab4aebe607805620d25a5e42566ce46a0e"
OUT_DIR="${1:-build/fs-uae/native}"
AMIGUARD_DIR="deps/AmiGuard"
ED25519_DIR="deps/ed25519"
CI_KEY_ID="ci-test-1"
CI_KEY_HEX="43046bfe4092b3e94994eada15dcc20d8aaa07b658fd3954eb8e0efb8bdca5de"
PROD_KEY_ID="${AMIGUARD_AE_TRUSTED_KEY_ID:-}"
PROD_KEY_HEX="${AMIGUARD_AE_TRUSTED_PUBLIC_KEY_HEX:-}"

if [[ -n "$PROD_KEY_ID" && -z "$PROD_KEY_HEX" ]] || [[ -z "$PROD_KEY_ID" && -n "$PROD_KEY_HEX" ]]; then
    echo "ERROR: set both AMIGUARD_AE_TRUSTED_KEY_ID and AMIGUARD_AE_TRUSTED_PUBLIC_KEY_HEX" >&2
    exit 2
fi
if [[ -n "$PROD_KEY_HEX" && ! "$PROD_KEY_HEX" =~ ^[0-9A-Fa-f]{64}$ ]]; then
    echo "ERROR: AMIGUARD_AE_TRUSTED_PUBLIC_KEY_HEX must be exactly 64 hex characters" >&2
    exit 2
fi

mkdir -p "$OUT_DIR" deps
if [[ ! -d "$AMIGUARD_DIR/.git" ]]; then git clone https://github.com/Ploos-AS/AmiGuard.git "$AMIGUARD_DIR"; fi
git -C "$AMIGUARD_DIR" fetch --depth 1 origin "$AMIGUARD_COMMIT"
git -C "$AMIGUARD_DIR" checkout --detach "$AMIGUARD_COMMIT"
if [[ ! -d "$ED25519_DIR/.git" ]]; then git clone https://github.com/orlp/ed25519.git "$ED25519_DIR"; fi
git -C "$ED25519_DIR" fetch --depth 1 origin "$ED25519_COMMIT"
git -C "$ED25519_DIR" checkout --detach "$ED25519_COMMIT"
printf '%s\n' "$AMIGUARD_COMMIT" > "$OUT_DIR/amiguard-engine-commit.txt"
printf '%s\n' "$ED25519_COMMIT" > "$OUT_DIR/ed25519-engine-commit.txt"
docker pull "$IMAGE"
docker image inspect "$IMAGE" --format '{{join .RepoDigests "\n"}}' | tee "$OUT_DIR/toolchain-image.txt"
ED25519_SOURCES=("$ED25519_DIR/src/verify.c" "$ED25519_DIR/src/sha512.c" "$ED25519_DIR/src/ge.c" "$ED25519_DIR/src/fe.c" "$ED25519_DIR/src/sc.c")
COMMON_SOURCES=(src/core.c src/arexx_dispatch.c src/scanner_bridge.c src/signature_bridge.c src/signature_manifest.c src/signature_auth_ed25519.c src/result_store.c src/checksum.c src/identify.c "$AMIGUARD_DIR/src/file_intake.c" "$AMIGUARD_DIR/src/file_signatures.c" "$AMIGUARD_DIR/src/hunk.c" "$AMIGUARD_DIR/src/signatures.c" "${ED25519_SOURCES[@]}")
COMMON_FLAGS=(-DAMIGUARD_AE_WITH_AMIGUARD=1 -DAMIGUARD_AE_WITH_ED25519=1 -Iinclude -Isrc -I"$AMIGUARD_DIR/src" -I"$ED25519_DIR/src" -Os -Wall -Wextra -Werror -m68000)
CI_TRUST_FLAGS=(-DAMIGUARD_AE_TRUSTED_KEY_ID=\"$CI_KEY_ID\" -DAMIGUARD_AE_TRUSTED_PUBLIC_KEY_HEX=\"$CI_KEY_HEX\")
PROD_TRUST_FLAGS=()
PROD_TRUST_STATUS="NOT_PROVISIONED_FAIL_CLOSED"
if [[ -n "$PROD_KEY_ID" ]]; then
    PROD_TRUST_FLAGS=(-DAMIGUARD_AE_TRUSTED_KEY_ID=\"$PROD_KEY_ID\" -DAMIGUARD_AE_TRUSTED_PUBLIC_KEY_HEX=\"$PROD_KEY_HEX\")
    PROD_TRUST_STATUS="PROVISIONED:$PROD_KEY_ID"
fi

# Production binary. Without explicit public trust-root environment variables it
# intentionally remains fail-closed for authenticated signature updates.
docker run --rm -v "$PWD:/work" -w /work "$IMAGE" m68k-amigaos-gcc "${COMMON_FLAGS[@]}" "${PROD_TRUST_FLAGS[@]}" -o AmiGuardAE.amiga src/main.c src/arexx_amiga.c "${COMMON_SOURCES[@]}" -mcrt=nix20

# Production-shaped trust-provisioning qualification binary using a public CI key.
docker run --rm -v "$PWD:/work" -w /work "$IMAGE" m68k-amigaos-gcc "${COMMON_FLAGS[@]}" "${CI_TRUST_FLAGS[@]}" -o AmiGuardAE-trusted-ci.amiga src/main.c src/arexx_amiga.c "${COMMON_SOURCES[@]}" -mcrt=nix20

# Guest smoke uses the same CI trust root and exercises authenticated update semantics.
docker run --rm -v "$PWD:/work" -w /work "$IMAGE" m68k-amigaos-gcc "${COMMON_FLAGS[@]}" "${CI_TRUST_FLAGS[@]}" -o AmiGuardAE-aros-smoke.amiga ci/fs-uae/aros_smoke_main.c "${COMMON_SOURCES[@]}" -mcrt=nix20

cp AmiGuardAE.amiga "$OUT_DIR/AmiGuardAE"
cp AmiGuardAE-trusted-ci.amiga "$OUT_DIR/AmiGuardAE-trusted-ci"
cp AmiGuardAE-aros-smoke.amiga "$OUT_DIR/AmiGuardAE-aros-smoke"
file "$OUT_DIR/AmiGuardAE" | tee "$OUT_DIR/file.txt"
file "$OUT_DIR/AmiGuardAE-trusted-ci" | tee "$OUT_DIR/trusted-ci-file.txt"
file "$OUT_DIR/AmiGuardAE-aros-smoke" | tee "$OUT_DIR/aros-smoke-file.txt"
sha256sum "$OUT_DIR/AmiGuardAE" | tee "$OUT_DIR/AmiGuardAE.sha256"
sha256sum "$OUT_DIR/AmiGuardAE-trusted-ci" | tee "$OUT_DIR/AmiGuardAE-trusted-ci.sha256"
sha256sum "$OUT_DIR/AmiGuardAE-aros-smoke" | tee "$OUT_DIR/AmiGuardAE-aros-smoke.sha256"
if ! grep -Eiq 'AmigaOS|Amiga.*executable|loadseg' "$OUT_DIR/file.txt"; then exit 1; fi
if ! grep -Eiq 'AmigaOS|Amiga.*executable|loadseg' "$OUT_DIR/trusted-ci-file.txt"; then exit 1; fi
if ! grep -Eiq 'AmigaOS|Amiga.*executable|loadseg' "$OUT_DIR/aros-smoke-file.txt"; then exit 1; fi
printf 'STATUS=PASS\nGATE=M3_10_PRODUCTION_TRUST_PROVISIONING_BUILD\nIMAGE=%s\nAMIGUARD_COMMIT=%s\nED25519_COMMIT=%s\nPRODUCTION_BINARY=%s\nTRUSTED_CI_BINARY=%s\nAROS_SMOKE_BINARY=%s\nPRODUCTION_TRUST_KEY=%s\nTRUST_PROVISIONING_PATH=QUALIFIED_WITH_PUBLIC_CI_KEY\nCI_TRUST_KEY_ID=%s\nPRIVATE_SIGNING_KEY=NEVER_EMBEDDED\nAREXX_RUNTIME_QUALIFICATION=DEFERRED_LOCAL_CLASSIC_AMIGAOS\n' "$IMAGE" "$AMIGUARD_COMMIT" "$ED25519_COMMIT" "$OUT_DIR/AmiGuardAE" "$OUT_DIR/AmiGuardAE-trusted-ci" "$OUT_DIR/AmiGuardAE-aros-smoke" "$PROD_TRUST_STATUS" "$CI_KEY_ID" | tee "$OUT_DIR/result.txt"
