# M4.5 Audit/logging

M4.5 adds an explicit, append-only audit-log foundation and dispatcher integration for security-relevant operations.

## Configuration boundary

Audit logging is disabled by default. The core exposes `amiguard_ae_audit_set_path()` for explicit runtime/startup policy configuration. An empty or cleared path disables logging.

The configured path is validated and rejects control characters, the record delimiter (`|`), `/`, `.`/`..`, and Amiga volume roots ending in `:`.

No ARexx command for changing the audit destination is exposed in M4.5. This avoids allowing an arbitrary automation client to silently redirect the audit trail.

## Record format

Records are append-only text lines with a deterministic machine-readable format:

```
AMIGUARD-AUDIT|1|<EVENT>|<STATUS>|<SUBJECT>|<DETAIL>
```

Fields reject newlines, carriage returns, control characters and `|` so one operation cannot inject additional records or fields.

## Dispatcher events

When audit logging is configured, the dispatcher records these security-relevant command families:

- `SCAN` and `SCANFILE` as event `SCAN`;
- `QUARANTINE`;
- `RESTORE`;
- `SIGNATURE.UPDATE`.

Dispatcher return codes map to audit status as follows:

- RC 0 -> `OK`;
- RC 5 -> `WARN`;
- other dispatcher errors -> `ERROR`.

The subject is the command argument and detail is the deterministic dispatcher result text.

## Failure semantics

M4.5 logging is best-effort after command dispatch. Failure to append an audit record does not rewrite the result of an operation that has already completed. Therefore M4.5 must not be described as a transactional or fail-closed audit journal.

A later policy hardening milestone may expose audit-health status and warning semantics for configured-but-unwritable logs without falsely reporting an already committed file operation as failed.

## Qualification

Automated qualification covers:

- disabled-by-default behavior;
- explicit audit-path configuration;
- deterministic append format;
- delimiter/control-character rejection;
- SCAN, SIGNATURE.UPDATE, QUARANTINE and RESTORE dispatcher records;
- strict host tests;
- Bebbo `-m68000` native linking with the audit implementation;
- FS-UAE/AROS native 68k qualification path.

For HEAD `16ead0a0367626b85c18522d81c7f17070bcd83b`, host-check #221 and FS-UAE/AROS #212 passed.

Classic AmigaOS public `AMIGUARD` ARexx transport qualification remains deferred to the existing local runtime gate.
