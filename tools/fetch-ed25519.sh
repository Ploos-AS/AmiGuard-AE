#!/usr/bin/env bash
set -euo pipefail
COMMIT="b1f19fab4aebe607805620d25a5e42566ce46a0e"
DIR="${1:-deps/ed25519}"
mkdir -p "$(dirname "$DIR")"
if [[ ! -d "$DIR/.git" ]]; then
  git clone https://github.com/orlp/ed25519.git "$DIR"
fi
git -C "$DIR" fetch --depth 1 origin "$COMMIT"
git -C "$DIR" checkout --detach "$COMMIT"
printf '%s\n' "$COMMIT"
