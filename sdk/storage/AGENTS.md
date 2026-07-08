# AGENTS.md

This file provides guidance for AI agents working in `sdk/storage` of the Azure SDK for C++ repository.

## Scope

This AGENTS file applies to:

- `sdk/storage/**`
- Cross-cutting storage concepts shared across blobs, file shares, datalake, queues, and storage common.

This file supplements the repository-root guidance in `/AGENTS.md`.  
Agents working in `sdk/storage` must follow both files: apply root repository-wide policies first, then apply storage-specific guidance from this file.

For service-specific guidance, prefer the local AGENTS files:

- `sdk/storage/azure-storage-blobs/AGENTS.md`
- `sdk/storage/azure-storage-files-datalake/AGENTS.md`
- `sdk/storage/azure-storage-files-shares/AGENTS.md`

If guidance conflicts, the most local AGENTS file takes precedence.

---

## Storage Architecture Basics (C++ SDK)

Azure Storage packages in this repo generally follow this layering:

1. **azure-core**  
   HTTP pipeline, credentials, retries, diagnostics, policies, transport abstractions.

2. **azure-storage-common**  
   Shared storage primitives (SAS helpers, shared key auth policy integrations, common request/response helpers, shared models).

3. **Service libraries**
   - `azure-storage-blobs`
   - `azure-storage-files-datalake`
   - `azure-storage-files-shares`
   - `azure-storage-queues`

### Typical client structure

Storage libraries commonly expose:

- **Service clients** (account/service-level operations)
- **Container/share/filesystem clients** (scope-level operations)
- **Item clients** (blob/file/path/message operations)
- **Options/Models** for API inputs and outputs

Maintain consistency with existing naming, API style, and option-object patterns.

---

## Agent Priorities

When making changes, prioritize:

1. **API consistency** with surrounding storage clients.
2. **Minimal diff** and surgical edits.
3. **Backward compatibility** for public APIs.
4. **Tests first/with changes** (unit + live/recording updates as needed).
5. **Documentation and changelog updates** where required.

---

## Do / Don’t

### Do

- Follow existing patterns in neighboring storage packages before introducing new abstractions.
- Reuse shared helpers in `azure-storage-common` when appropriate.
- Keep error handling aligned with azure-core and existing storage behavior.
- Preserve ABI/API compatibility unless explicitly requested and reviewed.
- Add or update tests for every functional change.
- Keep generated code and hand-written code boundaries intact.

### Don’t

- Don’t introduce breaking public API changes without explicit review.
- Don’t duplicate common logic that belongs in storage common.
- Don’t hardcode secrets, account keys, SAS tokens, or connection strings.
- Don’t manually edit generated assets when source-generation workflow is expected.
- Don’t broaden scope into unrelated storage services in one change.

---

## Key Concepts Agents Should Understand

- **Authentication modes:** Shared Key, SAS, TokenCredential (Microsoft Entra ID via azure-identity).
- **SAS types:** account SAS, service SAS, user delegation SAS.
- **Conditional requests:** ETag / match conditions.
- **Data integrity:** transactional hashes/validation where supported.
- **Paging:** continuation tokens and pageable responses.
- **Retries/timeouts:** pipeline retry policies and per-call options.
- **Service-version-sensitive behavior:** preserve compatibility and existing defaults.

---

## Testing Guidance

Before finalizing changes:

1. Build impacted storage package(s).
2. Run targeted tests for the modified package.
3. If request/response wire behavior changes, ensure playback/recording compatibility strategy is considered.
4. Prefer narrowly scoped tests over broad suite expansion unless needed.

Use PLAYBACK mode for fast validation where applicable, and only require LIVE/RECORD updates when behavior actually changed.

---

## Expected Change Hygiene

For non-trivial changes:

- Update package `CHANGELOG.md`.
- Ensure samples/docs remain accurate.
- Keep includes, namespaces, and public headers aligned with package conventions.
- Validate CMake updates are minimal and correctly scoped to the package.

---

## Escalation / Human Review Triggers

Flag for maintainer review if change involves:

- Public API shape changes.
- Authentication/signing logic.
- Retry/pipeline policy semantics.
- Cross-package refactors in multiple storage libraries.
- Service-version default changes.
- Test proxy / recording sanitization policy adjustments.

---

## Quick Checklist for Agents

- [ ] Scope confirmed to correct storage package.
- [ ] Followed nearest existing code pattern.
- [ ] Added/updated tests.
- [ ] No secrets introduced.
- [ ] No unintended public API break.
- [ ] Changelog/docs updated (if applicable).
- [ ] Build and targeted tests pass locally/CI.
