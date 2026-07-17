# AGENTS.md

Guidance for AI agents working in `sdk/storage/azure-storage-files-shares`.

## Scope

This file applies to:

- `sdk/storage/azure-storage-files-shares/**`

It extends `sdk/storage/AGENTS.md` with Azure Files (SMB/NFS cloud file share) specific guidance.

---

## File Shares Domain Model (mental map)

Typical client hierarchy includes:

- **ShareServiceClient** (service/account scope)
- **ShareClient** (share scope)
- **Directory and File clients** (path/item scope)
- Optional related clients for handles/permissions/snapshots depending on API surface

Preserve consistency of this hierarchy and existing naming conventions.

---

## File Shares-Specific Concepts to Preserve

- **Share vs directory vs file** operation boundaries.
- **Create/update ranges** and sparse file range semantics.
- **Handles/sessions** management operations where exposed.
- **SMB/NFS-relevant properties** (permissions/attributes/protocol-oriented metadata as applicable).
- **Snapshots** of shares and snapshot-targeted operations.
- **Quota/provisioning properties** and service/share-level settings.
- **Rename/move semantics** where available in API versions.

---

## Common Pitfalls (avoid)

- Confusing blob-style operations with file share path/range behaviors.
- Incorrect range write/clear behavior or off-by-one range boundaries.
- Regressing directory traversal/listing with large shares.
- Breaking snapshot-aware request targeting.
- Mis-handling path normalization/encoding for nested directories.

---

## Implementation Guidance

- Reuse existing request builders and models for range operations.
- Keep directory/file client responsibilities well separated.
- Preserve existing option structs and overload patterns.
- Align error handling with current file shares conventions and azure-core patterns.

---

## Testing Focus Areas

When modifying file shares code, cover:

1. Share create/delete/get properties flows.
2. Directory and file CRUD operations.
3. Range upload/clear/download behavior.
4. Listing and paging continuation.
5. Snapshot create/list/targeting behavior (if touched).
6. Protocol/permission-related operations (if touched).
7. Error-path behavior and condition headers.

Add boundary tests for empty files, large ranges, nested paths, and existing/non-existing resources.

---

## API & Compatibility Guardrails

- Do not break public client/type names or existing method contracts.
- Keep changes additive when possible.
- Preserve behavioral expectations for range and path operations.

---

## Performance & Reliability Expectations

- Avoid unnecessary buffering/copies for large file/range operations.
- Keep retries safe for range writes and other potentially non-idempotent calls.
- Avoid adding extra service calls in common path operations.

---

## Review Triggers

Flag for maintainer review when changes include:

- Range write/read logic
- Snapshot handling
- Permission/attribute semantics
- Rename/move/path semantics
- Public API changes
