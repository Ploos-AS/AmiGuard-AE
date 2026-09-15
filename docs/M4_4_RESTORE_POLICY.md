# M4.4 Restore and Policy

M4.4 introduces the verified restore core for quarantined objects.

## Safety contract

Restore is fail-closed and uses the configured quarantine directory from M4.3. The implementation:

1. resolves `<ID>.qtn` and `<ID>.meta` inside the configured quarantine directory;
2. parses the versioned metadata and requires the metadata ID to match the requested ID;
3. validates the recorded original destination with the quarantine path-safety rules;
4. refuses restore if the original destination already exists;
5. verifies quarantine-object CRC32 and size against committed metadata;
6. copies to a destination-side staging path;
7. verifies the staged CRC32 before commit;
8. renames the verified stage to the original path;
9. retains the quarantine object and metadata after restore for audit/recovery.

No silent overwrite is permitted. A corrupt or tampered quarantine object is never restored.

## Current boundary

This commit qualifies the M4.4 restore core and policy semantics. The `RESTORE <id>` dispatcher/ARexx command is the next M4.4 sub-gate. Classic AmigaOS ARexx transport qualification remains deferred to the existing local transport gate.

The no-overwrite check is the portable single-process baseline. This milestone does not claim a cross-process atomic no-replace primitive on every supported Amiga filesystem.
