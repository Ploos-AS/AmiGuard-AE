# AmiGuard AE ARexx API

## Port

The public ARexx port is:

```text
AMIGUARD
```

Scripts should use `ADDRESS AMIGUARD` after verifying that AmiGuard AE is running.

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

### `PING`

RC `0`, result:

```text
PONG
```

### `VERSION`

RC `0`, result contains the AmiGuard AE version string.

### `STATUS`

RC `0`. At M2.6 the native core reports one of:

```text
READY M2.6 scanner=connected
READY M2.6 scanner=not-connected
```

### `HELP`

RC `0`, result lists the available command surface.

## M2 scanner commands

### `SCAN <path>`

Scans one path through the configured AmiGuard-compatible scanner provider and stores the structured result.

Typical outcomes:

```text
RC 0  CLEAN <detail>
RC 5  INFECTED <detail>
RC 5  SUSPICIOUS <detail>
RC 10 ERROR <detail>
```

Missing path returns:

```text
RC 10 ERROR SCAN requires path
```

M2 does not define recursive directory or volume semantics for `SCAN`.

### `SCANFILE <path>`

Scans one file through the scanner provider and updates the same `RESULT.*` state as `SCAN`.

Typical outcomes use the same CLEAN / INFECTED / SUSPICIOUS / ERROR convention as `SCAN`.

Missing path returns:

```text
RC 10 ERROR SCANFILE requires path
```

A CLEAN result means that the current engine/signature set did not detect a threat; it is not a proof that the file is safe.

### `CHECKSUM <path>`

Calculates a read-only CRC32 checksum.

Success:

```text
RC 0 CRC32 XXXXXXXX
```

Missing path or read/open errors return RC `10` with deterministic `ERROR ...` text.

CRC32 is provided for identification and integrity convenience. It is not a cryptographic security primitive.

`CHECKSUM` does not modify `RESULT.*` scan state.

### `IDENTIFY <path>`

Performs read-only type identification and returns a stable type/detail pair.

Current stable categories include:

```text
AMIGA-HUNK HUNK_HEADER
IFF FORM container
DATA unrecognized binary/data
```

Missing path or read/open errors return RC `10`.

`IDENTIFY` does not modify `RESULT.*` scan state.

## Structured result interface

`RESULT.*` represents the most recent `SCAN` or `SCANFILE` operation.

### `RESULT.STATUS`

RC `0` when a result exists. Stable values:

```text
CLEAN
INFECTED
SUSPICIOUS
ERROR
```

Before any stored scan result exists:

```text
RC 10 ERROR no scan result
```

### `RESULT.PATH`

RC `0`, returns the path associated with the stored scan result.

### `RESULT.DETAIL`

RC `0`, returns scanner detail associated with the stored scan result.

### `RESULT.CLEAR`

Clears structured scan state.

```text
RC 0 OK
```

## M3 planned signature commands

- `SIGNATURE.INFO <name-or-id>`
- `SIGNATURE.COUNT`
- signature update/status operations

The M3 command contract will be frozen before that milestone is marked complete.

## Planned quarantine commands

- `QUARANTINE <path>`
- `RESTORE <id>`

Destructive or state-changing operations require additional safety design before implementation.

## Planned configuration interface

- `CONFIG.GET <key>`
- `CONFIG.SET <key> <value>`

## Planned events

Candidate events are scan started/completed, infected/detection, clean, error, quarantine and signature update. The implementation must avoid re-entrancy hazards and define whether events invoke scripts, signal an observer, or enqueue messages before this API is declared stable.

## Qualification model

GitHub Actions qualifies:

- strict host-side dispatch/tests,
- Bebbo native 68000 build,
- native 68k scanner/core execution in an FS-UAE/AROS guest.

AROS nightly does not supply the classic `rexxsyslib.library` required by the production ARexx transport. Therefore the public `AMIGUARD` port, RexxMast interaction and transport shutdown behavior remain a deliberate local qualification gate on classic AmigaOS 2.04+.

`examples/m1_qualification.rexx` remains the base transport probe for that local qualification and should be extended as later command surfaces are qualified through the live ARexx port.

## Compatibility

Baseline target: AmigaOS 2.04+ on 68000-class systems. Later CPUs and OS releases are expected to remain supported, but must not become requirements for the baseline build.
