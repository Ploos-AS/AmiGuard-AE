# M4.2 Transactional quarantine store

M4.2 adds the first mutating quarantine backend. The operation is deliberately staged and fail-safe.

## Commit sequence

1. Validate the source path and quarantine plan.
2. Refuse an existing quarantine destination or busy staging object.
3. Copy the source into a per-object staging file.
4. Recompute CRC32 over the staged object and require an exact match with the plan.
5. Rename the verified staged object to its final quarantine object.
6. Write and commit per-object metadata.
7. Only after the quarantine object and metadata are committed, remove the source.

If staging, verification or metadata commit fails, the source is preserved and incomplete staging is removed where possible.

If source removal itself fails after commit, the quarantine copy remains and the API reports `quarantine committed; source removal pending`; callers must not interpret that as a clean move.

## No-overwrite rule

An existing quarantine object is rejected. The backend never intentionally opens the final quarantine object for writing and does not silently replace an existing object.

The current implementation performs an existence check before the final rename. This is a single-process AmigaOS baseline, not a claim of cross-process atomic no-replace semantics. A later hardening milestone can add an OS-specific exclusive-create primitive where available.

## Metadata

Each quarantine object gets a matching `<ID>.meta` record containing:

- format version,
- quarantine ID,
- original source path,
- CRC32,
- source size.

The metadata is committed before source removal.

## Scope

M4.2 does not yet expose `QUARANTINE` through ARexx, nor does it implement restore. It establishes and qualifies the safe backend primitive first.
