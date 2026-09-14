# AmiGuard AE ARexx API

## Port

The public ARexx port is `AMIGUARD`. Scripts should use `ADDRESS AMIGUARD` after verifying that AmiGuard AE is running.

## Return codes

The frozen base convention is:

- `0` — OK
- `5` — warning / detection / suspicious result
- `10` — command or runtime error
- `20` — fatal failure

Unknown and empty commands return RC `10` deterministically.

## Contract principles

1. Commands are case-insensitive.
2. Queries must not mutate scanner state unless explicitly documented.
3. Commands return an AmigaDOS/ARexx-compatible numeric return code.
4. Structured data is available through stable result commands rather than requiring scripts to parse UI text.
5. Unknown commands and malformed arguments fail deterministically.
6. Scanner/backend code is independent of RexxMast; ARexx is an interface, not the antivirus engine.

## Core commands

`PING` returns RC 0 and `PONG`. `VERSION` returns the AmiGuard AE version string. `STATUS` returns the current milestone/core and scanner state. `HELP` lists the available command surface.

## Scanner commands

### `SCAN <path>` / `SCANFILE <path>`

Scan one target through the configured AmiGuard-compatible scanner provider and store structured `RESULT.*` state. Outcomes are RC 0 CLEAN, RC 5 INFECTED/SUSPICIOUS, or RC 10 ERROR. A CLEAN result only means the current engine/signature set did not detect a threat.

### `CHECKSUM <path>`

Returns `CRC32 XXXXXXXX` on success. CRC32 is for identification/integrity convenience and is not a cryptographic authenticity primitive. This command does not alter `RESULT.*`.

### `IDENTIFY <path>`

Read-only identification. Stable categories include `AMIGA-HUNK HUNK_HEADER`, `IFF FORM container`, and `DATA unrecognized binary/data`. It does not alter `RESULT.*`.

## Structured scan result interface

`RESULT.STATUS`, `RESULT.PATH`, `RESULT.DETAIL`, and `RESULT.CLEAR` expose the most recent `SCAN`/`SCANFILE` result. Stable statuses are CLEAN, INFECTED, SUSPICIOUS and ERROR.

## M3 signature automation

### `SIGNATURE.COUNT`

Returns the number of active signatures as a decimal value. RC 10 is returned when the signature backend is unavailable.

### `SIGNATURE.INFO <index>`

Returns deterministic metadata for the zero-based signature index. Boot-block signatures precede file signatures in the current flattened view. Invalid/missing indices return RC 10.

### `SIGNATURE.STATUS`

Returns machine-readable capability state including active signature count, update availability, `VERIFY=CRC32`, and `AUTH=AVAILABLE|UNAVAILABLE`.

### `SIGNATURE.UPDATE <database> <CRC32> <manifest>`

Attempts an authenticated runtime signature database update. The update pipeline is deliberately fail-closed:

1. The candidate database CRC32 must match the supplied eight-hex-digit CRC32.
2. The signed manifest must parse using the strict `AMIGUARD-SIGMANIFEST 1` contract.
3. Manifest algorithm must be Ed25519 and its database name/CRC32 must bind to the candidate.
4. KEYID must resolve to the trusted public key provisioned into the build.
5. The Ed25519 signature over the canonical manifest payload must verify.
6. The manifest sequence must be newer than the accepted sequence in the running process.
7. Only after those gates pass may the runtime signature database be activated.

A production build with no trusted public key provisioned rejects authenticated updates rather than silently trusting a candidate. Private signing keys are never required by or embedded in AmiGuard AE runtime builds.

CRC32 provides accidental-corruption/integrity checking only; authenticity comes from Ed25519.

M3 sequence anti-rollback state is process-local. It prevents replay during a running instance but is not yet durable across reboot/process restart. Documentation and callers must not describe M3 as reboot-resistant rollback protection.

See `docs/M3_8A_SIGNED_MANIFEST.md`, `docs/M3_9_SIGNING_TRUST.md`, and `docs/M3_10_PRODUCTION_TRUST_BUILD.md` for the manifest, signing and production provisioning contracts.

## M4 planned quarantine/policy commands

- `QUARANTINE <path>`
- `RESTORE <id>`
- policy/configuration operations

M4 must define safe-path rules, deterministic quarantine IDs/metadata, non-overwrite restore behavior and audit semantics before destructive file operations are enabled.

## Planned configuration interface

- `CONFIG.GET <key>`
- `CONFIG.SET <key> <value>`

## Planned events

Candidate events are scan started/completed, infected/detection, clean, error, quarantine and signature update. The implementation must avoid re-entrancy hazards and define whether events invoke scripts, signal an observer, or enqueue messages before this API is declared stable.

## Qualification model

GitHub Actions qualifies strict host-side tests, Bebbo native 68000 builds, and native 68k scanner/core execution in an FS-UAE/AROS guest. AROS nightly does not supply the classic `rexxsyslib.library` required by the production ARexx transport. Therefore the public `AMIGUARD` port, RexxMast interaction and transport shutdown behavior remain a deliberate local qualification gate on classic AmigaOS 2.04+.

`examples/m1_qualification.rexx` remains the base transport probe and should be extended as later command surfaces are qualified through the live ARexx port.

## Compatibility

Baseline target: AmigaOS 2.04+ on 68000-class systems. Later CPUs and OS releases are expected to remain supported, but must not become requirements for the baseline build.
