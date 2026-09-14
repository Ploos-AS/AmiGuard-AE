# M3.8a Signed Manifest Contract

M3.8a defines the strict, portable manifest syntax that the M3.8b cryptographic verifier will authenticate.

The parser is deliberately separate from cryptographic verification. Successful parsing MUST NOT be treated as authentication. Until an M3.8b verifier accepts the detached signature through the existing authentication provider, `SIGNATURE.UPDATE` remains fail-closed.

## Format

```
AMIGUARD-SIGMANIFEST 1
ALGORITHM=ED25519
KEYID=ploos-release-1
SEQUENCE=42
DATABASE=amiguard-file.db
CRC32=4D0F09D8
SIGNATURE=<128 hexadecimal characters>
```

Fields are ordered and mandatory. Unknown or extra fields are rejected.

- `ALGORITHM` is fixed to `ED25519` for the v1 contract.
- `KEYID` identifies the trusted public key; it is not itself proof of trust.
- `SEQUENCE` is a positive monotonically increasing release number intended for rollback protection in M3.8b.
- `DATABASE` names the signature database covered by the manifest.
- `CRC32` binds the existing M3.6 integrity gate to the manifest.
- `SIGNATURE` is a 64-byte Ed25519 signature represented as 128 hexadecimal characters.

## Security boundary

M3.8a validates syntax and canonical metadata only. It does not claim that an Ed25519 signature is valid. M3.8b must add public-key verification and rollback enforcement before authenticated remote updates are considered complete.
