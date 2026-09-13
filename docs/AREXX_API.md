# AmiGuard AE ARexx API

## Port

The public ARexx port is:

```
AMIGUARD
```

Scripts should use `ADDRESS AMIGUARD` after verifying that the application is running.

## Return codes

M1 freezes the base return-code convention:

- `0` — OK
- `5` — warning / partial success
- `10` — command or runtime error
- `20` — fatal failure

Unknown and empty commands return RC `10` deterministically.

## Contract principles

1. Commands are case-insensitive.
2. Queries must not mutate scanner state unless explicitly documented.
3. Commands return an AmigaDOS/ARexx-compatible numeric return code.
4. Detailed machine-readable data is exposed through result variables rather than requiring scripts to parse human UI text.
5. Unknown commands and malformed arguments fail deterministically.
6. Scanner/backend code is independent of RexxMast; ARexx is an interface, not the antivirus engine.

## M1 core commands

### `PING`

Returns RC `0` and result `PONG`.

### `VERSION`

Returns RC `0` and the AmiGuard AE version string.

### `STATUS`

Returns RC `0` and current service state. During M1 the expected result is:

```
READY M1 scanner=not-connected
```

### `HELP`

Returns RC `0` and the discovery command list:

```
PING VERSION STATUS HELP
```

## Planned scanner commands

- `SCAN <path>`
- `SCANFILE <file>`
- `CHECKSUM <file>`
- `IDENTIFY <file>`

## Planned signature commands

- `SIGNATURE.INFO <name-or-id>`
- `SIGNATURE.COUNT`
- `SIGNATURE.UPDATE`

## Planned result interface

- `RESULT`
- `RESULT.COUNT`
- `RESULT.NAME <index>`
- `RESULT.VIRUS <index>`

The exact stem/result-variable representation will be frozen before M2 runtime qualification.

## Planned quarantine commands

- `QUARANTINE <path>`
- `RESTORE <id>`

Destructive or state-changing operations require additional safety design before implementation.

## Planned configuration interface

- `CONFIG.GET <key>`
- `CONFIG.SET <key> <value>`

## Planned events

Candidate events are scan started/completed, infected/detection, clean, error, quarantine and signature update. The implementation must avoid re-entrancy hazards and must define whether events invoke scripts, signal an observer, or enqueue messages before this API is declared stable.

## Qualification

`examples/m1_qualification.rexx` is the canonical M1 guest-side runtime probe. M1 is not complete until it passes against the native 68000 build in FS-UAE and the public port is verified to disappear cleanly on shutdown.

## Compatibility

Baseline target: AmigaOS 2.04+ on 68000-class systems. Later CPUs and OS releases are expected to remain supported, but must not become requirements for the baseline build.
