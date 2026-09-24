# Application Creation Workflow

This directory provides a general workflow for organizing application creation
before implementation, using pre-specification notes and a three-document set.

Human users should start with:

- `USER_GUIDE.en.md`

To give an AI the working context, have it read:

- `three_key_documents_workflow.md`

For post-generation checks, design documentation, and operational decisions, use:

- `docs/post_generation_workflow.md`

Then use the templates in `templates/` in the following order. Refer only to the
workflow copied into the user's workspace, not to workflow copies elsewhere.
Place application-specific specifications, implementation prompts, and source code
under a newly created directory for the target application.

```text
Understand pre-specification notes and the three-document set
  ↓
Request .txt in any format
  ↓
Requirements review
  ↓
Revise requirement.txt if needed
  ↓
Create the target application's directory
  ↓
Create pre-specification notes from the request .txt
  ↓
Three-document set
  ↓
Shared implementation constraints, if needed
  ↓
Implementation prompt
  ↓
Application implementation
  ↓
Create and check tests after code generation
  ↓
Implementation review
  ↓
Reflect changes in specifications if needed
  ↓
Implementation design contract
  ↓
Application design document
  ↓
Review the design document
  ↓
Check that changes are reflected in specifications
```

If the implementation needs correction, return upstream rather than starting
with downstream code changes.

```text
Report a mismatch with intent
  ↓
Revise pre-specification notes
  ↓
Revise the three-document set
  ↓
Revise the implementation prompt
  ↓
Revise code
```

## File Structure

```text
app-generation-workflow/
├─ README.md
├─ USER_GUIDE.md
├─ three_key_documents_workflow.md
└─ templates/
   ├─ requirement.txt
   ├─ source_memo.md
   ├─ source_memo_reverse_review.md
   ├─ 01_usecase_spec.md
   ├─ 02_ui_spec.md
   ├─ 03_business_spec.md
   ├─ implementation_constraints.md
   └─ implementation_prompt.md
```

`requirement.txt` is a recommended template for organizing requests before
writing pre-specification notes. Requests may use any format, but this template
helps reduce unresolved questions.

Use `source_memo.md` when creating new pre-specification notes from a request `.txt`.
Use `source_memo_reverse_review.md` to reconstruct notes from an existing
implementation, a generated three-document set, or code. During reverse generation,
do not automatically make observed implementation behavior the official
specification. Separate observed behavior, behavior adopted as specification,
and possible bugs or items requiring confirmation.

`implementation_constraints.md` is an optional template for implementation
constraints shared across a split specification workflow. Keep it separate from
the three-document set and refer to it when creating implementation prompts.
Prompts must explicitly identify the target application's directory as the working directory.

`implementation_design_contract.md` records the correspondence between the
three-document set and implementation code after implementation review.
Do not establish this contract while major inconsistencies remain unresolved.

`app_design.md` records the overall application design after the implementation
design contract is created. Review consistency among the three-document set,
the contract, and the code before using it for subsequent decisions.

## Usage

For human users:

```text
Read USER_GUIDE.en.md.
```

To give an AI the working context:

```text
Read app-generation-workflow/three_key_documents_workflow.md.
```

`three_key_documents_workflow.md` is the authoritative procedure.
