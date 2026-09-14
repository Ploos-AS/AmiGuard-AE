#!/usr/bin/env python3
import argparse
import binascii
import os
import subprocess
import tempfile
from pathlib import Path


def crc32_file(path: Path) -> str:
    crc = 0
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(65536), b""):
            crc = binascii.crc32(chunk, crc)
    return f"{crc & 0xffffffff:08X}"


def build_payload(key_id: str, sequence: int, database_name: str, crc32: str) -> bytes:
    text = (
        "AMIGUARD-SIGMANIFEST 1\n"
        "ALGORITHM=ED25519\n"
        f"KEYID={key_id}\n"
        f"SEQUENCE={sequence}\n"
        f"DATABASE={database_name}\n"
        f"CRC32={crc32}\n"
    )
    return text.encode("ascii")


def sign_payload(private_key: Path, payload: bytes) -> str:
    with tempfile.TemporaryDirectory() as tmp:
        payload_path = Path(tmp) / "payload.bin"
        signature_path = Path(tmp) / "signature.bin"
        payload_path.write_bytes(payload)
        subprocess.run(
            [
                "openssl", "pkeyutl", "-sign", "-rawin",
                "-inkey", str(private_key),
                "-in", str(payload_path),
                "-out", str(signature_path),
            ],
            check=True,
        )
        signature = signature_path.read_bytes()
    if len(signature) != 64:
        raise SystemExit(f"unexpected Ed25519 signature length: {len(signature)}")
    return signature.hex()


def main() -> int:
    parser = argparse.ArgumentParser(description="Create an AmiGuard AE signed signature manifest")
    parser.add_argument("--key", required=True, type=Path, help="Ed25519 private key PEM; never committed to the repo")
    parser.add_argument("--key-id", required=True, help="Trusted key identifier provisioned in AmiGuard AE")
    parser.add_argument("--sequence", required=True, type=int, help="Monotonically increasing positive sequence")
    parser.add_argument("--database", required=True, type=Path, help="Signature database to bind and sign")
    parser.add_argument("--output", required=True, type=Path, help="Manifest output path")
    args = parser.parse_args()

    if args.sequence <= 0:
        raise SystemExit("sequence must be greater than zero")
    if not args.key_id or any(ch.isspace() for ch in args.key_id):
        raise SystemExit("key id must be non-empty and contain no whitespace")
    if not args.key.is_file():
        raise SystemExit("private key not found")
    if not args.database.is_file():
        raise SystemExit("signature database not found")

    crc32 = crc32_file(args.database)
    database_name = args.database.name
    payload = build_payload(args.key_id, args.sequence, database_name, crc32)
    signature_hex = sign_payload(args.key, payload)
    manifest = payload.decode("ascii") + f"SIGNATURE={signature_hex}\n"

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(manifest, encoding="ascii", newline="\n")
    print(f"MANIFEST={args.output}")
    print(f"KEYID={args.key_id}")
    print(f"SEQUENCE={args.sequence}")
    print(f"DATABASE={database_name}")
    print(f"CRC32={crc32}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
