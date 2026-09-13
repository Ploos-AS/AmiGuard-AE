# AmiGuard AE Roadmap

AmiGuard AE (ARexx Edition) is the automation-oriented member of the AmiGuard family.

## Design constraints

- Target AmigaOS 2.04+.
- Target 68000 as the baseline CPU.
- Keep the antivirus/signature semantics compatible with AmiGuard wherever practical.
- Expose automation through an ARexx port named `AMIGUARD`.
- Keep ARexx optional for the scanner core: failure or absence of RexxMast must not corrupt scanner state.
- Prefer deterministic, script-friendly return codes and result variables.

## M0 - Foundation

Status: **complete**.

- Project scope and compatibility contract.
- MIT license, Copyright Ploos AS.
- Source/include/docs/examples layout.
- Initial `AMIGUARD` ARexx command contract.
- Scanner/backend boundary suitable for sharing or adapting AmiGuard functionality.
- Host-side structural qualification (`make check`).

## M1 - Native skeleton

Status: **implementation complete; runtime qualification pending**.

Implemented:

- Bebbo `m68k-amigaos-gcc` build target with `-m68000`.
- Native `AMIGUARD` public message port lifecycle.
- ARexx message dispatch and result-string replies.
- `PING`, `VERSION`, `STATUS`, `HELP`.
- AmigaDOS/ARexx return codes: 0 OK, 5 WARN, 10 ERROR, 20 FAIL.
- Host unit qualification for command dispatch.
- GitHub Actions host qualification.
- `examples/m1_qualification.rexx` runtime probe.

Runtime gate before M1 may be marked complete:

1. Cross-build `AmiGuardAE.amiga` for 68000.
2. Boot an AmigaOS 2.04+ / 68000-class FS-UAE guest with RexxMast available.
3. Start AmiGuardAE and verify the `AMIGUARD` port exists.
4. Run `examples/m1_qualification.rexx` through ARexx.
5. Record PASS for PING, VERSION, STATUS, HELP and deterministic unknown-command RC=10.
6. Verify clean shutdown/removal of the public port.

## M2 - Scanner bridge

- Connect AE to AmiGuard-compatible scanning/signature functionality.
- Implement `SCAN`, `SCANFILE`, `CHECKSUM`, `IDENTIFY`.
- Structured result retrieval (`RESULT.*`).
- Preserve scanner operation when ARexx is unavailable.

## M3 - Signature automation

- `SIGNATURE.INFO`, `SIGNATURE.COUNT`, update/status operations.
- Stable machine-readable result variables.
- Regression tests against AmiGuard signature semantics.

## M4 - Quarantine and policy

- `QUARANTINE`, `RESTORE` and policy/configuration API.
- Safe-path and destructive-action guards.
- Audit/logging interface.

## M5 - Events and ecosystem integration

- Event hooks for scan, detection, clean result, error, quarantine and signature updates.
- Example integrations for AmiForensics, AmiSandbox/ASW, AmiGet and BBS workflows.
- Qualification matrix across supported AmigaOS targets.

## Release gate

The first public release should not be cut merely because the ARexx port responds. It should include useful scanner automation, stable return semantics, documentation/examples, and runtime qualification.
