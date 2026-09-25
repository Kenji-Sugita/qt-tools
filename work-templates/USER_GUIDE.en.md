---
genpdf:
  format: book
  title: |-
    Work Context Management
    User Guide
  subtitle: |-
    Recording, Resuming, and Sharing Work
    with NEXT, backlog, and DECISIONS
  author: ""
  version: 1.0
  date: 2026-09-19
  font_size: 11pt
  page_numbers: true
  table:
    header_background: "#e8f1ff"
    header_color: "#1f2937"
    border_color: "#cbd5e1"
    border_width: "1px"
    stripe: true
    stripe_background: "#f8fafc"
    cell_padding: "0.45em 0.65em"
    font_size: "0.95em"
    compact: false
    header_align: center
    cell_align: left
---

# Work Context Management User Guide

## 1. Purpose

This system organizes the context that accumulates during work into Markdown files with distinct roles, preserving it for continued use by people and AI.

Its main purposes are to:

- Prevent excessive growth of context stored in AI conversation histories and similar places.
- Permanently preserve important context, including work history, decision rationale, specifications, and constraints.
- Share context across hosts, development environments, and AI sessions.
- Reduce the amount to read and the time needed to review it when resuming work.
- Separate current work, long-term issues, and lasting decisions so information is easy to find.
- Enable work to continue after interruptions, staff changes, or changes of AI tools.
- Record design decisions and rejected alternatives to avoid repeating investigations and discussions.
- Track changes in Git so outdated information can be organized safely.

<div class="page-break"></div>

## 2. Quick Start: Just Three Things to Remember

### 2.1 Initial Setup

For English templates from a repository checkout, follow [Starting New Work](README.en.md#starting-new-work).
Copy the `.en.md` templates to the standard filenames used throughout this guide.

If using the ZIP distribution instead, extract `work-templates.zip` and place the shared operating rules and work management files in their locations.

- Install `AGENTS.next.md` as `~/AGENTS.next.md` on each host.
- Place these three files in the target project:
  - `NEXT.md`
  - `backlog.md`
  - `DECISIONS.md`

`work-templates.zip` contains `README.md`, `AGENTS.next.md`, and the three templates above. The Markdown and PDF versions of this user guide are not included in the ZIP; consult them separately on the distribution page.

If the work management files already exist, there is no need to overwrite them with templates.

### 2.2 When Finishing Work

Ask the AI to update `NEXT.md` to reflect the current state of the work.

### 2.3 When Resuming Work

Ask the AI to read `NEXT.md`, then resume work.

The beginning of `NEXT.md` refers to the shared operating rules in `~/AGENTS.next.md`, which also instruct the AI to read `backlog.md` and `DECISIONS.md`.

## 3. System Overview

The system consists of one shared set of operating rules and three information stores.

| File | Role |
|---|---|
| `AGENTS.next.md` | Defines shared operating rules for `NEXT.md` across projects |
| `NEXT.md` | Stores the current state, next actions, and information needed to resume work |
| `backlog.md` | Stores remaining work, problems, and deferred items that will not be tackled immediately next time |
| `DECISIONS.md` | Stores lasting decisions, their rationale, specifications, decision criteria, and operating constraints |

`work-templates.zip` contains `README.md`, `AGENTS.next.md`, `NEXT.md`, `backlog.md`, and `DECISIONS.md`. Consult the user guide separately as Markdown or PDF on the distribution page.

Place the distributed `AGENTS.next.md` in each host's home directory as `~/AGENTS.next.md`. `NEXT.md` explicitly instructs the reader to read that file. If it is missing, the rules require asking the user before organizing or deleting work management files.

Track the project's three files in Git alongside the project and keep `~/AGENTS.next.md` identical across hosts. This makes the same context and rules available when the host or AI session changes.

## 4. Where Information Belongs

If you are unsure where to put information, decide in this order:

1. If it is directly needed to resume work next time, put it in `NEXT.md`.
2. If it describes work, problems, or deferred items that will not be tackled immediately next time, put it in `backlog.md`.
3. If it is a conclusion, rationale, specification, or constraint that affects future decisions, put it in `DECISIONS.md`.
4. Keep completion history and long validation logs in `CHANGELOG.md`, `README.md`, validation records, or identifiable Git commits.

Briefly:

- Needed next time: `NEXT.md`
- Work for later: `backlog.md`
- Decisions that remain relevant: `DECISIONS.md`

Do not duplicate detailed information across files. Choose one authoritative file and refer to it from other files using relative paths or heading names.

## 5. Using `NEXT.md`

### 5.1 Role

`NEXT.md` stores information directly needed to resume work next time. It is used to quickly understand the current position and next action, rather than as a work history archive. After copying the distributed template, replace its illustrative entries with the minimum information needed to resume.

Keep instructions at the beginning to read `~/AGENTS.next.md` first, along with the protection rule for when it is missing. Do not change these opening rules without an explicit user instruction.

### 5.2 When to Update It

- When the work objective or state changes
- When the next action changes
- When interrupting work
- When completing a work topic
- When new precautions or references are needed to resume

### 5.3 Writing Each Section

#### Current State

Describe the objective, completed scope, remaining scope, and current position. Include only the work history and validation details needed for the next decision.

#### Next Actions

State the first actions to take next time. Normally, limit this to one or two concrete, actionable items. Even when there are multiple topics, list only the next step for each.

#### Incomplete Items

Include only incomplete items needed for the next work decision. Move long-term remaining work and detailed problems to `backlog.md`, and lasting decisions and operating constraints to `DECISIONS.md`.

#### Files to Change

List the files expected to change next time and why, using repository-relative paths whenever possible.

#### Notes

Record constraints, accident-prevention measures, pending approvals, fragile areas, environmental requirements, handling of uncommitted changes, and anything else that must be recalled immediately next time.

#### Related Files

List the files to consult when resuming and explain their roles. `DECISIONS.md` is the reference for lasting decisions, specifications, and operating constraints; `backlog.md` is the reference for remaining work, problems, and deferred items not to be tackled immediately next time. Identify relevant headings when useful.

### 5.4 Keeping It Compact

As a guideline, retain dated work records from the last five actual working days, or until the current work topic is completed. An actual working day is a day on which work, decisions, validation, or commits occurred.

This period is a guide for organizing work history, not a reason to delete still-valid specifications, decisions, rejection rationale, or operating constraints. Keep those in `DECISIONS.md`.

### 5.5 Example

```markdown
## Current State

- The search screen display implementation is complete.
- Keyboard interaction still needs validation.

## Next Actions

- Validate keyboard interaction with the search results list.

## Incomplete Items

- For long-term issues, see "Improving Search Speed" in `backlog.md`.

## Files to Change

- `src/search-view.js`
- `tests/search-view.test.js`

## Notes

- Do not change the existing search URL.

## Related Files

- "Keep the Existing Search URL" in `DECISIONS.md`
```

## 6. Using `backlog.md`

### 6.1 Role

`backlog.md` stores long-term remaining work, problems, and deferred items that will not be tackled immediately next time. It does not store the current work state or completion history.

### 6.2 Remaining Work

Describe work that is necessary but is not an immediate target. Include why it is needed or its completion criteria, as well as related files and references.

### 6.3 Problems

Record known problems, their impact, and references to investigation results or reproduction conditions. Put problems to solve immediately next time in `NEXT.md`.

### 6.4 Deferred Items

Record what is deferred, why, and the conditions for reconsideration. Work whose start date is simply postponed belongs here, rather than under unresolved design decisions.

### 6.5 Moving Items Back to `NEXT.md`

Once the next item to tackle is chosen, move the information directly needed to carry it out to `NEXT.md`. If detailed investigation records remain useful, leave them in `backlog.md` and refer to them from `NEXT.md`.

### 6.6 Example

```markdown
## Remaining Work

### 1. Improving Search Speed

- Reduce search time for large datasets.
- Complete when response time is within one second for 10,000 records.
- Related: `src/search-index.js`

## Deferred Items

### 1. Introducing an External Search Service

- Do not introduce it at present.
- Deferred because operating costs are not yet established.
- Reconsider when monthly searches exceed 100,000.
```

## 7. Using `DECISIONS.md`

### 7.1 Role

`DECISIONS.md` stores decisions that remain valid for future work, together with the context needed to understand them. Recording the rationale as well as the policy prevents repeated discussions.

### 7.2 Accepted

Record the adopted policy and its rationale. Include any constraints and priorities that affected the decision.

### 7.3 Rejected

Record the alternative considered, why it was rejected, and the conditions for reconsideration. This prevents reconsidering the same proposal when its underlying assumptions have not changed.

### 7.4 Undecided

Record unresolved matters requiring a decision, rather than implementation tasks. State what needs to be checked before deciding. Work awaiting only a start date belongs in `backlog.md`.

### 7.5 Decision Criteria

State what to prioritize when comparing alternatives, such as preserving existing URLs, compatibility, maintainability, or operating costs.

### 7.6 Lasting Specifications and Operating Constraints

Record specifications to preserve in future changes and operating constraints to observe during work. Include the rationale so you can later determine whether each constraint remains necessary.

### 7.7 Changing a Policy

Do not erase an old policy without explanation. Record the new decision, reason for the change, date or related commit, and how the old policy is treated. If detailed history is needed, consider moving to ADRs with one decision per file.

### 7.8 Example

```markdown
## Accepted

### 1. Keep the Existing Search URL

- Continue using `/search`.
- This avoids breaking links from external sites and users' bookmarks.

## Rejected

### 1. Move the Search Screen to Another Domain

- Considered separating its operation.
- Rejected because authentication and monitoring would become more complex.
- Reconsider if the current configuration can no longer handle the search load.
```

## 8. Basic Operating Flow

### 8.1 Starting Work

On a host using this system for the first time, extract `work-templates.zip` and install the included `AGENTS.next.md` as `~/AGENTS.next.md`. Place the three work management files in the target project. If work information already exists, organize it into the three stores using the criteria in Chapter 4.

### 8.2 During Work

Update `NEXT.md` when the next action changes, `backlog.md` when a long-term issue is found, and `DECISIONS.md` when a lasting decision is made. Do not leave important decisions only in conversation history.

### 8.3 Interrupting Work

Ask the AI to update `NEXT.md`. Under the shared operating rules, the other two files are also updated as needed, leaving `NEXT.md` sufficient to understand the first action and precautions for the next session.

### 8.4 Resuming Work

Ask the AI to read `NEXT.md`. Following its opening instructions, the AI first checks `~/AGENTS.next.md` and also reads the other two files. When resuming manually, read `NEXT.md` first and consult the other files as needed.

### 8.5 Completing Work

Ask the AI to update `NEXT.md`. Follow Chapter 9 for the detailed checks.

## 9. Updates at Work Completion

### 9.1 Confirm What Was Completed

Review Git diffs, recent commits, related documents, and validation results to establish what is complete and what remains.

### 9.2 Organize the Three Files

- Organize completed temporary items in `NEXT.md` and describe the next work state.
- Store remaining long-term issues, problems, and deferred items in `backlog.md`.
- Store policies, rationale, specifications, and constraints established during work in `DECISIONS.md`.

### 9.3 Preserve Completion History and Validation Results

Ensure completion history and long validation logs can be traced through one of:

- `CHANGELOG.md`
- `README.md` or user guides
- Dedicated validation records
- Identifiable Git commits

Save conclusions, rationale, and constraints that affect future decisions in `DECISIONS.md`; do not discard them as completion history.

### 9.4 Check the Diff

Review the Git diff after updating. Where possible, also run `git diff --check` to detect whitespace errors and similar issues.

### 9.5 Record in Git

Make the work and the work management files traceable together. If resuming on another host, publish the necessary commits or changes to a shared repository or another location accessible from that host.

## 10. Update Procedure with `AGENTS.next.md`

### 10.1 Role

`AGENTS.next.md` defines a shared procedure for the AI to use `NEXT.md` as the entry point, read the three work management files, and update them to match the current state.

### 10.2 When to Use It

- When interrupting work
- When changing work topics
- When completing work
- When organizing the three files to reflect the current state

### 10.3 How to Ask the AI

In the target project's working environment, ask: "Update `NEXT.md` to reflect the current state of the work." The AI reads `~/AGENTS.next.md` as instructed at the beginning of `NEXT.md`, then checks Git diffs, recent commits, all three management files, and related documents. You do not need to list `backlog.md` and `DECISIONS.md` in the request.

### 10.4 Routine Updates and Prior Confirmation

Adding information, moving items, and correcting the current state are routine updates.

For the following changes, the shared rules instruct the AI to seek confirmation before making changes:

- Deleting entire sections
- Deleting multiple independent items
- Deleting design decisions or rejection rationale
- Major restructuring

Review the proposed targets, reasons, and destinations before instructing the AI whether to proceed.

### 10.5 Report After Updating

Receive a file-by-file report from the AI describing changes to `NEXT.md`, `backlog.md`, and `DECISIONS.md`, along with validation results such as diff checks.

## 11. Rules for Safe Organization

- Do not delete information solely because of its date, length, or age.
- Before deleting, check where conclusions, rationale, and constraints affecting future decisions will be preserved.
- Organize completion history and validation logs only when the same information remains in traceable documents or Git commits.
- Seek prior confirmation for deleting sections, multiple items, or important decisions, and for major restructuring.
- Protect existing uncommitted changes and do not change unrelated files.
- Do not record credentials, secrets, or personal information that should not be public.
- Avoid host-specific absolute paths; use repository-relative paths wherever possible.

## 12. Sharing Across Hosts

Track the three work management files in Git and make them accessible from other hosts.

Share the operating rules by installing identical copies of the ZIP's `AGENTS.next.md` as `~/AGENTS.next.md` on each host. When updated, distribute the change to all hosts in use.

When finishing work, record the branch, baseline commit, tool versions, external dependencies, and environmental requirements needed to resume in `NEXT.md` as appropriate. If uncommitted changes exist, record their presence and handling, and preserve the changes themselves in a transferable form such as a commit or patch.

When resuming on another host, verify that `~/AGENTS.next.md` is installed. Check local uncommitted changes and possible conflicts before fetching the latest state, then ask the AI to read `NEXT.md`. The shared rules also cause it to read the other two files.

## 13. Usage Examples

### 13.1 Interrupting Work in Progress

1. Ask the AI to update `NEXT.md`.
2. Confirm that `NEXT.md` states the current position and next action.
3. Confirm that new long-term issues and decisions are saved in the appropriate files.
4. Make the necessary changes traceable in Git.

### 13.2 Finding a Long-Term Issue

Add issues outside the current work to `backlog.md`. When an issue becomes the next target, move the information needed to start into `NEXT.md`.

### 13.3 Deciding a Design Policy

Record the adopted policy and its rationale in `DECISIONS.md`. If it directly affects the next session, reference that decision from `NEXT.md`.

### 13.4 Completing a Work Topic

1. Preserve the results in source, user documentation, validation records, Git commits, or similar locations.
2. Ask the AI to update `NEXT.md`.
3. Confirm that remaining work is in `backlog.md` and lasting decisions are in `DECISIONS.md`.
4. Align `NEXT.md` with the next work topic.

### 13.5 Resuming on Another Host

1. Retrieve the necessary changes from the shared source.
2. Ask the AI to read `NEXT.md`. The shared rules cause it to read the other two files as well.
3. Resume from "Next Actions" in `NEXT.md`.

## 14. Common Mistakes

### Keeping All History in `NEXT.md`

This buries the information needed to resume. Keep completion history in traceable documents or Git and move lasting decisions to `DECISIONS.md`.

### Leaving Long-Term Issues in `NEXT.md`

Move items that will not be tackled immediately next time to `backlog.md`.

### Putting Tasks in `DECISIONS.md`

Tasks merely waiting to be implemented belong in `backlog.md`. Use `DECISIONS.md` for matters requiring a decision and for policies already decided.

### Recording Policies Without Rationale

This prevents evaluating decisions later. Include reasons for acceptance and rejection, and conditions for reconsideration.

### Deleting Completed Items Without Checking Their Destination

Results, decisions, and constraints may be lost. Confirm a traceable destination before organizing them.

### Duplicating Detailed Information Across Files

This leads to inconsistent updates. Choose an authoritative source and reference it from other files.

<div class="page-break"></div>

## 15. Post-Update Checklist

- [ ] `NEXT.md` alone explains the current state and next actions.
- [ ] Illustrative entries in `NEXT.md` have been replaced with the minimum information needed to resume.
- [ ] `~/AGENTS.next.md` is installed and consistent across hosts in use.
- [ ] Issues not to be tackled immediately next time are separated into `backlog.md`.
- [ ] Lasting decisions, rationale, specifications, and constraints are saved in `DECISIONS.md`.
- [ ] Completion history and validation results have a traceable location.
- [ ] Cross-file references are correct.
- [ ] No information was deleted solely because of age.
- [ ] Existing uncommitted changes and unrelated files were not changed.
- [ ] The Git diff was reviewed.
- [ ] `git diff --check` was run where possible.
- [ ] The information and changes needed to resume on another host are accessible.

<div class="page-break"></div>

## 16. Quick Reference

### Where to Store Information

| Information | Destination |
|---|---|
| Current state | `NEXT.md` |
| Next actions | `NEXT.md` |
| Precautions for the next session | `NEXT.md` |
| Long-term remaining work | `backlog.md` |
| Problems and their impact | `backlog.md` |
| Deferred items and reconsideration conditions | `backlog.md` |
| Accepted policies and rationale | `DECISIONS.md` |
| Rejected alternatives and rationale | `DECISIONS.md` |
| Unresolved decisions | `DECISIONS.md` |
| Lasting specifications and constraints | `DECISIONS.md` |
| Completion history and long validation logs | `CHANGELOG.md`, `README.md`, validation records, Git commits, etc. |

### Actions by Timing

| Timing | Action |
|---|---|
| Initial setup | Install the ZIP's `AGENTS.next.md` in the home directory and the three files in the target project. |
| Finishing work | Ask the AI to update `NEXT.md`. |
| Resuming work | Ask the AI to read `NEXT.md`. |
