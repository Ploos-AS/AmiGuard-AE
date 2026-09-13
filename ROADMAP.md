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

Status: complete when the repository contains the architecture/API baseline and a host-checkable source skeleton.

- Project scope and compatibility contract.
- MIT license, Copyright Ploos AS.
- Source/include/docs/examples layout.
- Initial `AMIGUARD` ARexx command contract.
- Scanner/backend boundary suitable for sharing or adapting AmiGuard functionality.
- Host-side structural qualification (`make check`).

## M1 - Native skeleton

- Build with the Bebbo `m68k-amigaos-gcc` toolchain for `-m68000`.
- Start/stop `AMIGUARD` ARexx port safely.
- Implement `VERSION`, `STATUS`, `PING`, `HELP`.
- Establish documented AmigaDOS/ARexx return-code conventions.
- Initial FS-UAE runtime qualification.

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
