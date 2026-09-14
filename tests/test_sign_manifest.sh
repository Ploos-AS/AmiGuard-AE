#!/usr/bin/env bash
set -euo pipefail
mkdir -p build/m3_9
key=build/m3_9/test-ed25519.pem
pub=build/m3_9/test-ed25519.pub.pem
db=build/m3_9/fixture.sigdb
manifest=build/m3_9/fixture.manifest
payload=build/m3_9/payload.txt
sig=build/m3_9/signature.bin

openssl genpkey -algorithm ED25519 -out "$key" >/dev/null 2>&1
openssl pkey -in "$key" -pubout -out "$pub" >/dev/null 2>&1
printf 'AMIGUARD-FILE-SIGDB 1\nFILE|M3.9.Test|0|1|313233343536373839|ffffffffffffffffff\n' > "$db"

pubhex="$(python3 tools/public_key_hex.py "$key")"
[[ "$pubhex" =~ ^[0-9a-f]{64}$ ]]

python3 tools/sign_manifest.py \
  --key "$key" \
  --key-id m3-9-test \
  --sequence 7 \
  --database "$db" \
  --output "$manifest" >/dev/null

grep -q '^AMIGUARD-SIGMANIFEST 1$' "$manifest"
grep -q '^ALGORITHM=ED25519$' "$manifest"
grep -q '^KEYID=m3-9-test$' "$manifest"
grep -q '^SEQUENCE=7$' "$manifest"
grep -q '^DATABASE=fixture.sigdb$' "$manifest"
grep -Eq '^CRC32=[0-9A-F]{8}$' "$manifest"
grep -Eq '^SIGNATURE=[0-9a-f]{128}$' "$manifest"

head -n 6 "$manifest" > "$payload"
sed -n 's/^SIGNATURE=//p' "$manifest" | xxd -r -p > "$sig"
openssl pkeyutl -verify -rawin -pubin -inkey "$pub" -in "$payload" -sigfile "$sig" >/dev/null

printf 'tamper\n' >> "$payload"
if openssl pkeyutl -verify -rawin -pubin -inkey "$pub" -in "$payload" -sigfile "$sig" >/dev/null 2>&1; then
  echo 'FAIL: tampered payload verified' >&2
  exit 1
fi

echo 'M3.9 signing/provisioning qualification: PASS'
