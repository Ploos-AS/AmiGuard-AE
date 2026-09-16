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

## M1 - Native skeleton
Status: **implementation complete; automated native-core qualification complete; classic AmigaOS ARexx transport qualification deferred to local runtime**.

## M2 - Scanner bridge
Status: **complete for implementation and automated native-core qualification; classic AmigaOS ARexx transport qualification remains deferred with M1**.

Current M2 command surface: `SCAN`, `SCANFILE`, `CHECKSUM`, `IDENTIFY`, and structured `RESULT.*` commands.

## M3 - Signature automation
Status: **complete for implementation and automated native-core qualification; classic AmigaOS ARexx transport qualification remains deferred with M1**.

Implemented: signature count/info/status/update, runtime databases, CRC32 integrity, signed manifests, Ed25519 authentication, fail-closed trusted-key provisioning and process-local sequence rollback protection.

Security boundary: CRC32 is integrity only; Ed25519 is authenticity. Durable reboot-resistant anti-rollback remains future hardening.

## M4 - Quarantine and policy
Status: **implementation complete through M4.5; automated host/native-core qualification complete; hardening and classic AmigaOS ARexx transport qualification remain**.

### M4.1 - Quarantine safety model
Status: **complete and automated-qualified**.

- Safe-path guards.
- Deterministic quarantine IDs.
- Source CRC32 and size capture.
- Non-destructive planning boundary.

### M4.2 - Transactional quarantine store
Status: **complete and covered by the later M4.3 automated qualification path**.

- stage and verify source;
- commit object and metadata before source removal;
- refuse existing quarantine destinations;
- preserve source on pre-commit failure;
- report source-removal-pending separately.

### M4.3 - ARexx quarantine command
Status: **complete and automated-qualified**.

- `QUARANTINE <path>` dispatcher command;
- fail-closed without an explicitly configured quarantine directory;
- RC 0 committed+removed, RC 5 committed/removal-pending, RC 10 rejected/failed;
- deterministic `QUARANTINED ID=<id> PATH=<stored-path>` result;
- host, Bebbo 68000 and FS-UAE/AROS qualification green.

### M4.4 - Restore and policy
Status: **complete and automated-qualified**.

- `RESTORE <id>` dispatcher command;
- resolve quarantine object and versioned metadata by deterministic ID;
- require metadata ID to match requested ID;
- verify object CRC32 and size before restore;
- validate the recorded original destination;
- refuse silent overwrite when destination already exists;
- stage and verify restored content before destination commit;
- retain quarantine object and metadata after successful restore;
- reject tampered quarantine content;
- deterministic RC/result strings;
- host, Bebbo 68000 and FS-UAE/AROS qualification green.

### M4.5 - Audit/logging
Status: **complete for the defined best-effort audit contract and automated-qualified**.

- explicit audit-path policy; disabled by default;
- append-only `AMIGUARD-AUDIT|1|...` machine-readable records;
- SCAN/SCANFILE, QUARANTINE, RESTORE and SIGNATURE.UPDATE dispatcher events;
- deterministic OK/WARN/ERROR status mapping;
- record-field injection guards;
- audit implementation linked into production, trusted-CI and AROS-smoke native 68000 builds;
- host and FS-UAE/AROS automated qualification green.

Security boundary: M4.5 audit writes occur after command dispatch and are best-effort. An audit append failure does not rewrite the result of an operation that has already committed. M4.5 is therefore not a transactional/fail-closed audit journal.

See `docs/M4_5_AUDIT_LOGGING.md`.

### M4 hardening / closeout
Status: **next**.

Before closing M4 completely:

- tighten restore ID parsing to the exact deterministic ID format;
- tighten metadata CRC32 and SIZE parsing to reject trailing/ambiguous input;
- fix audit append close/error handling so the log file is always closed after an attempted write;
- refresh native qualification labels/evidence from older M4.3/M4.4 names where needed;
- keep classic AmigaOS public `AMIGUARD`/RexxMast transport qualification in the deferred local runtime gate.

## M5 - Events and ecosystem integration

- Event hooks for scan, detection, clean result, error, quarantine and signature updates.
- Example integrations for AmiForensics, AmiSandbox/ASW, AmiGet and BBS workflows.
- Qualification matrix across supported AmigaOS targets.

## Release gate

The first public release should include useful scanner automation, stable return semantics, documentation/examples, and classic AmigaOS runtime qualification of the production ARexx transport.
