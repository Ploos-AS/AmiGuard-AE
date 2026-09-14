# M4.3 - QUARANTINE command

M4.3 exposes the transactional M4.2 quarantine backend through the dispatcher/ARexx command surface.

## Command

`QUARANTINE <path>`

The command is deliberately fail-closed. No quarantine directory is configured by default. Production/runtime setup must explicitly configure a quarantine directory through the quarantine policy API before the command can mutate files.

## Return semantics

- RC 0: quarantine object and metadata committed and the source removed.
- RC 5: quarantine object and metadata committed, but source removal is still pending.
- RC 10: missing path, unavailable quarantine directory, planning failure, staging/verification failure, destination collision, metadata failure, or other rejected operation.

Successful deterministic result form:

`QUARANTINED ID=<id> PATH=<stored-path>`

Source-removal-pending result form:

`QUARANTINED ID=<id> PATH=<stored-path> SOURCE=REMOVAL_PENDING`

## Policy boundary

`amiguard_ae_quarantine_set_directory()` configures the directory used by the mutating command. Clearing the directory disables quarantine again. The default process state has no configured directory.

The policy rejects empty/unsafe directory forms and must not be interpreted as a complete canonical-path or symlink security boundary. Production configuration should use a dedicated directory such as `RAM:AmiGuard-Quarantine` or an equivalent persistent location, not a volume root.

## Safety sequence

The command reuses the M4.1/M4.2 safety pipeline:

1. validate source path;
2. build a deterministic plan with CRC32 and size;
3. stage the source into the quarantine directory;
4. verify staged CRC32;
5. commit the quarantine object;
6. commit per-object metadata;
7. remove the source only after both commits succeed.

Existing quarantine objects are never silently overwritten.

## Qualification

Host qualification exercises the dispatcher with quarantine disabled first, then configures `build` as a disposable quarantine directory and verifies deterministic RC/result semantics plus object/metadata creation and source removal.

The Bebbo 68000 build gate compiles the quarantine model/store into the production-shaped Amiga binaries. Classic AmigaOS ARexx transport qualification remains deferred to the local runtime gate shared with M1-M3.

## Next

M4.4 adds restore and explicit policy/configuration semantics, including refusal to silently overwrite an existing restore destination.
