# M3.10 - Production trusted-key build

M3.10 qualifies how the full native AmiGuard AE binary is built with an explicitly provisioned Ed25519 public trust root.

## Security boundary

- The private signing key is never compiled into AmiGuard AE.
- The private signing key is never needed by the runtime build.
- The runtime contains only a public Ed25519 key and a stable KEYID.
- The default production build has no trusted key and therefore fails closed for authenticated signature updates.
- Both trust variables must be supplied together. Partial provisioning is rejected.
- The public key must be exactly 32 bytes encoded as 64 hexadecimal characters.

## Default fail-closed build

```sh
ci/fs-uae/build-native.sh
```

The resulting `build/fs-uae/native/AmiGuardAE` contains the scanner and Ed25519 verifier but no trusted update key.

## Trusted production build

Provision the public trust root explicitly:

```sh
export AMIGUARD_AE_TRUSTED_KEY_ID=ploos-amiguard-prod-1
export AMIGUARD_AE_TRUSTED_PUBLIC_KEY_HEX=<64-hex-character-public-key>
ci/fs-uae/build-native.sh
```

The output binary is again `build/fs-uae/native/AmiGuardAE`, now compiled with that public trust root.

The public-key hex can be obtained from the release signing key with:

```sh
python3 tools/public_key_hex.py /secure/path/amiguard-signing.pem
```

This reads key material only on the signing/release workstation. Do not copy the private key into the source tree.

## CI qualification

CI builds three native 68000 binaries:

1. `AmiGuardAE` - production default, no trust key, fail-closed.
2. `AmiGuardAE-trusted-ci` - production-shaped binary with a public CI-only test key.
3. `AmiGuardAE-aros-smoke` - guest qualification binary using the same public CI-only test key.

The CI key is not a production trust root and has no release authority. Its purpose is solely to prove that explicit trusted-key provisioning compiles through the full production binary path and that authenticated update semantics execute in the guest.

## Release procedure

For a future release build:

1. Keep the private Ed25519 signing key offline or on the designated signing workstation.
2. Extract only its public-key hex.
3. Select a stable KEYID.
4. Build AmiGuard AE with both public trust variables set.
5. Sign signature databases with `tools/sign_manifest.py` using the matching private key.
6. Preserve monotonically increasing manifest `SEQUENCE` values.

Classic AmigaOS public-ARexx transport qualification remains a separate local release gate.
