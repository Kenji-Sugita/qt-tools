# Shared Rules for Work Management Files

## Scope

Apply these rules when reading or updating `NEXT.md`, `DECISIONS.md`, or `backlog.md`.

- When asked to read `NEXT.md`, also read `DECISIONS.md` and `backlog.md` to understand the work state from all three files. Do not change files when the request is only to read them.
- When asked to update `NEXT.md`, also read `DECISIONS.md` and `backlog.md`, and update all three as needed to reflect the current work state. Complete the checks, classification, updates, validation, and reporting below.
- The user does not need to list `DECISIONS.md` and `backlog.md` in the request.

## Checks Before Updating

Before updating, review Git diffs, recent commits, all three work management files, and related documents.
Protect existing uncommitted changes and do not modify files unrelated to the request.

## Classifying Information

- Put the current state, next actions, and procedures and precautions needed to resume next time in `NEXT.md`.
- Put long-term remaining work, problems, and deferred items that will not be tackled immediately next time in `backlog.md`. Reference them from `NEXT.md` as needed.
- Put lasting specifications, adopted design decisions and their rationale, reasons for rejection, unresolved questions, decision criteria, and operating constraints in `DECISIONS.md`. Reference them from `NEXT.md` as needed.

As a guideline, retain dated work records in `NEXT.md` from the most recent five actual working days, or until the current work topic is completed.
An actual working day is a day on which work, decisions, validation, or commits took place in that repository.
Do not use session counts or calendar days alone to determine the cutoff.
This time guideline applies only to work history, not to still-valid specifications, design decisions, rejection rationale, or operating constraints.

## Protecting Information

Do not delete information solely because of its date, length, or age.
Before deleting completion history or validation logs, verify that the same information remains in `CHANGELOG.md`, `README.md`, or an identifiable Git commit.
Before deleting conclusions, rationale, or constraints that affect future decisions, preserve them in a tracked document.
If no destination exists, move the information to an appropriate document before deleting it.

If deleting entire sections, deleting multiple independent items, deleting design decisions or rejection rationale, or major restructuring is necessary, do not make the changes yet.
Present the targets, reasons, and destinations, and obtain the user's approval in advance.
Otherwise, additions, item moves, and corrections to reflect the current state are routine updates.

## Validation and Reporting After Updating

Review the diff after updating and run `git diff --check` if possible.
Finally, report the changes and validation results separately for `NEXT.md`, `DECISIONS.md`, and `backlog.md`.

Do not change these rules unless the user explicitly instructs you to do so.
