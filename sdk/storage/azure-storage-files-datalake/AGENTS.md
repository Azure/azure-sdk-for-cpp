# AGENTS.md

Guidance for AI agents working in `sdk/storage/azure-storage-files-datalake`.

## Scope

This file applies to:

- `sdk/storage/azure-storage-files-datalake/**`

It extends `sdk/storage/AGENTS.md` with datalake-specific guidance.

---

## Naming and Package Note

The Data Lake client code lives under `azure-storage-files-datalake` in this repository. When making changes, ensure paths, includes, namespaces, and conventions target this package.

---

## Data Lake Domain Model (mental map)

Typical hierarchy includes:

- **DataLakeServiceClient** (account/service scope)
- **FileSystemClient** (filesystem/container scope)
- **Path clients** (file or directory scope), often with dedicated file/directory operations

Data Lake builds on Blob Storage primitives; maintain alignment where behavior intentionally overlaps.

---

## Data Lake-Specific Concepts to Preserve

- **Hierarchical namespace semantics** (directories and path operations).
- **Path operations:** create, rename, move, delete.
- **File operations:** append + flush workflow (ordering and position semantics matter).
- **Directory semantics:** recursive operations and path traversal/listing.
- **Access control:** ACLs, permissions, owner/group where applicable.
- **Lease/conditions** behavior when exposed through datalake APIs.
- **SAS/authorization** parity with storage-common/blob foundations where applicable.

---

## Common Pitfalls (avoid)

- Treating datalake path behavior as identical to flat blob behavior.
- Breaking rename/move atomicity assumptions or destination-conditions handling.
- Mis-handling append/flush offsets and finalization semantics.
- Regressing recursive delete/list behavior with deep trees.
- Inconsistent URL/path encoding treatment for special characters.

---

## Implementation Guidance

- Follow existing path client patterns for request construction and response parsing.
- Reuse shared storage/common mechanisms for auth/pipeline/retries.
- Keep distinctions between directory and file operations clear and explicit.
- Ensure datalake behavior remains coherent with underlying blob constraints.

---

## Testing Focus Areas

When modifying datalake code, prioritize tests for:

1. Path create/delete/list operations (including continuation/paging).
2. Rename/move behavior and conflict/condition handling.
3. Append/flush positional correctness.
4. ACL/permission operations (where touched).
5. Deep directory recursive scenarios (if applicable).
6. Error mapping and exception behavior for common service failures.

Include edge cases for path encoding and unusual path names when relevant.

---

## API & Compatibility Guardrails

- Preserve public path/file/directory client API shape and naming conventions.
- Avoid behavioral changes that silently diverge from documented datalake semantics.
- Keep option structs backward compatible and additive when possible.

---

## Reliability Expectations

- Preserve idempotency where expected.
- Keep retry behavior safe for multi-step operations (append/flush, rename flows).
- Avoid partial-state changes in client abstractions when operations fail mid-flow.

---

## Review Triggers

Require/flag maintainer review for:

- Rename/move semantic changes
- Append/flush logic changes
- ACL/permission handling changes
- Public API changes across path clients
- Any cross-library behavior divergence vs blobs/common
