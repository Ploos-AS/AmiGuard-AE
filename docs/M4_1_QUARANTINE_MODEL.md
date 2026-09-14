# M4.1 Quarantine model

M4.1 defines the safety boundary before AmiGuard AE is allowed to move or remove a suspicious file.

## Non-destructive gate

The M4.1 API only creates a quarantine plan. It does **not** rename, move, delete, overwrite, truncate, chmod or otherwise modify the source.

A plan records:

- original source path,
- source CRC32,
- source size,
- deterministic quarantine ID.

The current ID is `Q` + eight hexadecimal CRC32 digits + seven hexadecimal low size digits. It is deterministic metadata, not a security token and not by itself a globally unique identity.

## Path guards

Planning rejects:

- empty paths,
- paths longer than the model limit,
- `/`,
- Amiga volume roots such as `SYS:`,
- `.` and `..`,
- paths containing control characters.

M4.1 intentionally does not claim complete canonical-path or symlink protection. Those checks belong to the actual mutation backend gate.

## M4.2 requirements

Before `QUARANTINE <path>` can mutate a file, M4.2 must add a quarantine store/provider with transactional semantics:

1. Validate source and destination.
2. Refuse destination overwrite.
3. Copy/stage content and metadata.
4. Verify staged content against the source fingerprint.
5. Commit quarantine metadata.
6. Only then remove/rename the source.
7. On failure before commit, preserve the source and clean incomplete staging where safe.

Restore must likewise refuse silent overwrite of an existing destination.

## Qualification

Host qualification proves root/path rejection, deterministic planning, source fingerprint/size capture and that repeated planning leaves the source present. Native 68000 compilation is covered because the model is part of the normal core source set.
