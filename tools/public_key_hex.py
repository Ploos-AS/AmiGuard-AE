#!/usr/bin/env python3
import argparse
import subprocess
from pathlib import Path

ED25519_SPKI_PREFIX = bytes.fromhex("302a300506032b6570032100")


def main() -> int:
    parser = argparse.ArgumentParser(description="Extract raw Ed25519 public-key hex for AmiGuard AE provisioning")
    parser.add_argument("key", type=Path, help="Ed25519 private or public PEM key")
    args = parser.parse_args()
    if not args.key.is_file():
        raise SystemExit("key not found")

    der = subprocess.check_output(
        ["openssl", "pkey", "-in", str(args.key), "-pubout", "-outform", "DER"],
        stderr=subprocess.DEVNULL,
    )
    if len(der) != len(ED25519_SPKI_PREFIX) + 32 or not der.startswith(ED25519_SPKI_PREFIX):
        raise SystemExit("key is not a canonical Ed25519 key")
    print(der[len(ED25519_SPKI_PREFIX):].hex())
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
