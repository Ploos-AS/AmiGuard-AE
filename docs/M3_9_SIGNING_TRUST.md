# M3.9 - Signing and production trust provisioning

M3.9 adds release-side tooling around the M3.8b Ed25519 verifier. It does not change the runtime ARexx protocol.

## Security model

- Private Ed25519 signing keys are never stored in this repository.
- A release/signature workstation signs manifests with `tools/sign_manifest.py`.
- AmiGuard AE runtime builds trust only the explicitly provisioned public key and KEYID.
- A runtime with no trusted public key remains fail-closed for authenticated signature updates.
- `SEQUENCE` remains monotonically increasing and is committed only after a database update succeeds.
- CRC32 binds the manifest to the exact database bytes, while Ed25519 authenticates the signed manifest payload.

## Generate an Ed25519 key

Example for an offline signing workstation:

```sh
openssl genpkey -algorithm ED25519 -out amiguard-signing.pem
openssl pkey -in amiguard-signing.pem -pubout -out amiguard-signing.pub.pem
```

Keep the private PEM outside the source tree and outside GitHub Actions secrets unless a deliberate release-signing design later requires otherwise.

## Extract the runtime public key

```sh
python3 tools/public_key_hex.py amiguard-signing.pem
```

The result is the 32-byte Ed25519 public key encoded as 64 lowercase hex characters. Provision that value together with a stable KEYID when producing the trusted production runtime build.

## Sign a signature database

```sh
python3 tools/sign_manifest.py \
  --key /secure/path/amiguard-signing.pem \
  --key-id ploos-amiguard-prod-1 \
  --sequence 42 \
  --database AmiGuard.sigdb \
  --output AmiGuard.sigdb.manifest
```

The tool computes the database CRC32 itself and signs the canonical manifest payload with OpenSSL Ed25519. The private key is read only by OpenSSL and is never copied into the manifest.

## Canonical signed payload

The Ed25519 signature covers exactly these six lines, including the final newline:

```text
AMIGUARD-SIGMANIFEST 1
ALGORITHM=ED25519
KEYID=<key-id>
SEQUENCE=<positive integer>
DATABASE=<basename>
CRC32=<8 uppercase hex digits>
```

`SIGNATURE=<128 hex characters>` is appended after signing and is not part of the signed payload itself.

## Qualification

`tests/test_sign_manifest.sh` generates a disposable Ed25519 key, creates a manifest, verifies the signature with OpenSSL, then proves a modified payload fails verification. No persistent or production private key is used by CI.
