# AGENTS.md

Guidance for AI agents working in `sdk/storage/azure-storage-blobs`.

## Scope

This file applies to:

- `sdk/storage/azure-storage-blobs/**`

It extends `sdk/storage/AGENTS.md` with blob-specific guidance.

---

## Blob Domain Model (mental map)

Primary client hierarchy typically includes:

- **BlobServiceClient** (account-level blob service operations)
- **BlobContainerClient** (container-level operations)
- **Blob clients** (blob-level operations), including specializations such as:
  - Block blob
  - Append blob
  - Page blob

Keep API behavior and naming consistent with this hierarchy.

---

## Blob-Specific Concepts to Preserve

- **Blob types:** block / append / page have distinct semantics.
- **Upload semantics:** single-shot vs staged/block uploads.
- **Download/range behavior:** partial reads and offset/length options.
- **Leases:** acquire/renew/change/release/break behavior.
- **Snapshots and versions:** immutable references and operation targeting.
- **Soft delete / undelete** and retention interactions.
- **Metadata vs tags:** separate concerns and APIs.
- **Access tiers / rehydration** behavior and option constraints.
- **Copy operations:** sync/async copy patterns and polling/status behavior.

---

## Common Pitfalls (avoid)

- Mixing block/blob/page operation constraints.
- Breaking conditional header behavior (If-Match, If-None-Match, etc.).
- Inconsistent treatment of empty payloads and zero-length blobs.
- Changing defaults that affect transfer performance or memory behavior.
- Regressing large-transfer chunking/parallel upload logic without benchmarks/tests.

---

## Implementation Guidance

- Reuse existing upload/download helper logic rather than introducing alternative pathways unless required.
- Keep transfer option parsing and defaults centralized and consistent.
- Maintain compatibility of public models/options and response types.
- Prefer additive changes over replacing established abstractions.

---

## Testing Focus Areas

When touching blob code, ensure tests cover impacted areas:

1. Upload/download happy path.
2. Conditional request behavior.
3. Paging/listing continuation flows.
4. Error-path mapping for key service responses.
5. Blob-type-specific behavior (where relevant).
6. Lease/version/snapshot semantics (if modified).

If behavior changes at protocol level, ensure recording/playback implications are addressed.

---

## API & Compatibility Guardrails

- Avoid renaming or removing public methods/types without approved breaking-change process.
- Keep overload patterns and option structs aligned with existing blob client style.
- Preserve expected exception/error surfaces unless explicitly justified.

---

## Performance & Reliability Expectations

- Be cautious with memory allocations for large transfers.
- Keep retry-friendly behavior for transient failures.
- Avoid introducing extra network calls on hot paths.
- Preserve streaming behavior where expected.

---

## Review Triggers

Explicit human review recommended for:

- Transfer engine/chunking changes
- Lease logic changes
- Copy/polling workflow changes
- Version/snapshot targeting semantics
- Public API additions/changes
