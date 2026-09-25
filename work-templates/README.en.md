# Work Templates

This directory contains document templates for starting new work or projects.
They separate information needed to resume work, long-term outstanding tasks,
and design decisions to keep `NEXT.md` manageable and prevent important decisions
from being lost. The aim is to restore work context from the repository even
when the host, development environment, or AI session changes.

## Background

The three-file arrangement and filenames are specific to this template, but the
separation of responsibilities adapts established practices for small or personal projects.

| Template | Comparable practice |
|---|---|
| `NEXT.md` | Cline Memory Bank's `activeContext.md`: current work, recent changes, and next steps |
| `DECISIONS.md` | Architecture Decision Records (ADRs) and decision logs: decisions, reasons, alternatives, and constraints |
| `backlog.md` | GitHub Issues / Projects backlogs: outstanding or deferred work outside the immediate scope |

References:

- [Cline Memory Bank](https://cline.bot/blog/memory-bank-how-to-make-cline-an-ai-agent-that-never-forgets)
- [Architectural Decision Records](https://adr.github.io/)
- [GitHub Projects Quickstart](https://docs.github.com/en/issues/planning-and-tracking-with-projects/learning-about-projects/quickstart-for-projects)

The template consolidates this information into three Markdown files tracked by
Git, without requiring external services or a large document set. If
`DECISIONS.md` grows, it can be split into one ADR per decision. If the backlog
grows or changes frequently, it can move to GitHub Issues / Projects or a similar tool.
AI is one of the participants that maintain and use these files. The same
documents also support human handoffs, PC migration, remote environments, and switching AI tools.

## File Roles

| English template | Installed filename | Role |
|---|---|---|
| [AGENTS.next.en.md](AGENTS.next.en.md) | `~/AGENTS.next.md` | Shared operating rules for `NEXT.md` across projects; place in the home directory |
| [NEXT.en.md](NEXT.en.md) | `NEXT.md` | Current state, next steps, and notes needed to resume work |
| [DECISIONS.en.md](DECISIONS.en.md) | `DECISIONS.md` | Accepted and rejected design decisions and reasons, open questions, decision criteria, and lasting specifications and constraints |
| [backlog.en.md](backlog.en.md) | `backlog.md` | Long-term outstanding tasks, problems, and deferred items not to be addressed immediately |

English templates live alongside the unchanged Japanese templates in this directory.
Copy them to the standard filenames shown above so existing references continue to work.
The rest of this README uses those installed filenames.

## Starting New Work

Run these commands from the root of your cloned repository (the directory containing
`work-templates/`). First, place the English shared rules in your home directory:

```sh
cp work-templates/AGENTS.next.en.md ~/AGENTS.next.md
```

Then copy the three English templates into an existing target project directory.
Replace `/path/to/your/project` with its actual path. For a project that already has
work-management files, keep those files rather than overwriting them with templates.

```sh
cp work-templates/NEXT.en.md /path/to/your/project/NEXT.md
cp work-templates/DECISIONS.en.md /path/to/your/project/DECISIONS.md
cp work-templates/backlog.en.md /path/to/your/project/backlog.md
```

Replace the instructional placeholders in `NEXT.md`, `DECISIONS.md`, and
`backlog.md` with actual content. Where nothing applies, write `- None` instead
of leaving placeholders. Before reading or updating `NEXT.md`, follow its opening
instructions and read `~/AGENTS.next.md` first.

## Resuming Across Hosts

- Track all three work-management files in Git so they can be retrieved from another host.
- Place the same `AGENTS.next.md` in each host's home directory.
- Prefer repository-relative references over absolute paths tied to a personal environment.
- Record any required branch, baseline commit, tool version, external dependency, or environment condition in `NEXT.md`.
- If uncommitted changes exist, record their presence and intended handling in `NEXT.md`. Save the changes themselves in a transferable form, such as a commit or patch.
- Record design decisions and constraints in `DECISIONS.md` rather than leaving them only in conversation history.
- Do not put credentials or secrets in these files.

## Separating Information

### What Belongs in NEXT.md

- The purpose and current state of the work
- One or two tasks to start with next time
- Outstanding items directly needed for the next work decision
- Files to modify next
- Immediate cautions and references

For dated work records, use the most recent five actual workdays or the completion
of the current theme as a guideline. This time limit is not a reason to remove
still-valid design decisions or operating constraints.

### What Belongs in DECISIONS.md

- Adopted approaches and reasons
- Rejected alternatives and reasons
- Open design questions
- Criteria to prioritize when making decisions
- Specifications and operating constraints that remain valid

Work whose start date is merely deferred belongs in `backlog.md`, not among
unresolved design decisions.

### What Belongs in backlog.md

- Long-term work not to be started immediately
- Known problems and their impact
- Deferred items with reasons and conditions for reconsideration

Do not use it to store completion history, verification logs, or design decisions.

## Updating After Work

Ask Codex to update `NEXT.md` to reflect the current state. The referenced
`~/AGENTS.next.md` defines the procedure: read `DECISIONS.md` and `backlog.md`,
review Git changes, recent commits, and related documents, and update all three files as needed.

The normal update flow is:

1. Reflect completed work in source, README, CHANGELOG, Git commits, or similar records.
2. Update the current state and immediate work in `NEXT.md`.
3. Record new design decisions in `DECISIONS.md`.
4. Move outstanding or deferred work outside the immediate scope to `backlog.md`.
5. Check diffs and references, and run `git diff --check` if possible.

## Major Reorganization

Deleting entire sections, multiple independent items, design decisions, or
rejection reasons, and major restructuring are not performed automatically as
routine updates. First have the assistant present what to keep, move, or delete,
with reasons and destinations. Review that proposal before directing it to apply changes.

Example instruction:

```text
Propose a reorganization of NEXT.md, DECISIONS.md, and backlog.md.
Do not change files yet. Show what to keep, move, or delete, with reasons.
```

## Completion History

There is no need to keep accumulating long completion records or verification
logs in work-management files. Before removing them, however, check that the
same information is available in one of:

- `CHANGELOG.md`
- `README.md` or a user guide
- Verification records
- An identifiable Git commit

Conclusions, reasons, and constraints that affect future decisions should remain
in `DECISIONS.md`, rather than being removed as completion history.
