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

Status: **implementation complete; automated native-core qualification complete; classic AmigaOS ARexx transport qualification deferred to local runtime**.

Implemented:

- Bebbo `m68k-amigaos-gcc` build target with `-m68000`.
- Native `AMIGUARD` public message port lifecycle.
- ARexx message dispatch and result-string replies.
- `PING`, `VERSION`, `STATUS`, `HELP`.
- AmigaDOS/ARexx return codes: 0 OK, 5 WARN, 10 ERROR, 20 FAIL.
- Host unit qualification for command dispatch.
- GitHub Actions host qualification.
- FS-UAE/AROS native 68k qualification of the dispatcher/core.
- `examples/m1_qualification.rexx` runtime probe for classic AmigaOS.

Deferred local transport gate:

1. Cross-build `AmiGuardAE.amiga` for 68000.
2. Boot an AmigaOS 2.04+ / 68000-class FS-UAE guest with RexxMast available.
3. Start AmiGuardAE and verify the `AMIGUARD` port exists.
4. Run `examples/m1_qualification.rexx` through ARexx.
5. Verify deterministic command return codes and result strings.
6. Verify clean shutdown/removal of the public port.

AROS nightly does not provide the classic `rexxsyslib.library` required to qualify the production ARexx transport, so CI intentionally qualifies the same native 68k dispatcher/core without pretending that transport gate has passed.

## M2 - Scanner bridge

Status: **complete for implementation and automated native-core qualification; classic AmigaOS ARexx transport qualification remains deferred with M1**.

Implemented and automated-qualified:

- M2.1 scanner bridge/provider boundary and `SCANFILE`.
- M2.2 pinned AmiGuard scanner integration for native 68000 builds.
- M2.3 structured `RESULT.STATUS`, `RESULT.PATH`, `RESULT.DETAIL`, `RESULT.CLEAR`.
- M2.4 `CHECKSUM <path>` using CRC32 for identification/integrity convenience.
- M2.5 `IDENTIFY <path>` with stable file-type output.
- M2.6 `SCAN <path>` using the scanner bridge and structured result state.
- Scanner/core operation remains independent of RexxMast.
- Host qualification plus Bebbo 68000 build and FS-UAE/AROS guest smoke gates.

Current M2 command surface:

- `SCAN <path>`
- `SCANFILE <path>`
- `CHECKSUM <path>`
- `IDENTIFY <path>`
- `RESULT.STATUS`
- `RESULT.PATH`
- `RESULT.DETAIL`
- `RESULT.CLEAR`

`SCAN` and `SCANFILE` currently operate on one path per command. Recursive directory or volume scanning is not implied by the M2 contract and should get its own explicit semantics before implementation.

## M3 - Signature automation

Status: **complete for implementation and automated native-core qualification; classic AmigaOS ARexx transport qualification remains deferred with M1**.

Implemented and automated-qualified:

- `SIGNATURE.COUNT` and indexed `SIGNATURE.INFO` over AmiGuard-compatible signatures.
- Runtime signature database loading through `SIGNATURE.UPDATE`.
- `SIGNATURE.STATUS` with update, integrity and authentication capability reporting.
- CRC32 integrity verification before activation.
- Strict signed-manifest contract and canonical payload.
- Ed25519 manifest authentication using a pinned established verifier.
- Fail-closed behavior when no trusted public key is provisioned.
- Sequence anti-replay/rollback protection within the running process.
- Host-side manifest signing/provisioning tools; private keys are never part of the runtime build.
- Production-shaped trusted-key 68000 build gate with explicit KEYID/public-key provisioning.
- Host, Bebbo 68000 and FS-UAE/AROS automated qualification.

Security boundary at M3 closeout:

- CRC32 is an integrity check, not authentication.
- Ed25519 is the authenticity gate for trusted signature updates.
- Production builds without an explicitly provisioned trusted public key reject authenticated updates.
- Sequence rollback state is currently process-local; durable reboot-resistant anti-rollback remains future hardening and must not be claimed by M3.

## M4 - Quarantine and policy

Status: **in progress**.

### M4.1 - Quarantine safety model

Status: **complete and automated-qualified**.

- Safe-path guards.
- Deterministic quarantine IDs.
- Source CRC32 and size capture.
- Explicitly non-destructive planning boundary.

### M4.2 - Transactional quarantine store

Status: **implementation complete; automated qualification pending**.

- Stage source into a per-object temporary file.
- Verify staged CRC32 against the plan.
- Commit the verified quarantine object.
- Commit per-object metadata before source removal.
- Remove the source only after quarantine commit succeeds.
- Refuse an existing quarantine destination.
- Preserve the source on staging, verification or metadata failure.
- Report source-removal-pending separately when commit succeeds but source deletion fails.

M4.2 deliberately does not expose `QUARANTINE` through ARexx yet and does not implement restore.

### M4.3 - ARexx quarantine command

Next after M4.2 qualification:

- `QUARANTINE <path>`
- deterministic result/return semantics
- policy checks before invoking the mutating backend

### M4.4 - Restore and policy

- `RESTORE <id>`
- refuse silent overwrite of an existing destination
- safe destination/path rules
- explicit policy/configuration interface

### M4.5 - Audit/logging

- durable audit records
- scan/quarantine/restore/signature-update events
- deterministic machine-readable status

## M5 - Events and ecosystem integration

- Event hooks for scan, detection, clean result, error, quarantine and signature updates.
- Example integrations for AmiForensics, AmiSandbox/ASW, AmiGet and BBS workflows.
- Qualification matrix across supported AmigaOS targets.

## Release gate

The first public release should not be cut merely because the ARexx port responds. It should include useful scanner automation, stable return semantics, documentation/examples, and classic AmigaOS runtime qualification of the production ARexx transport.
