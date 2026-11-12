# Workflows

[日本語](README.md) / [Qt Tools](../README.en.md)

Procedures, instructions, and templates to copy and apply to your work.

Relative paths in this index start from this directory. Run individual builds from the relevant project directory.

### Development Workflows, Instructions and Templates

| Directory | Tool | Description |
|---|---|---|
| [app-generation-workflow](app-generation-workflow/app-generation-workflow/README.en.md) | App generation workflow | A document-driven AI development harness focused on GUI applications. It uses documents to connect requirements gathering, specification, implementation, testing, and review. |
| [ai-cmake](ai-cmake/USER_GUIDE.en.md) | AI CMake instructions | Instructions and a validation environment for generative AI to create and modify Qt 6 `CMakeLists.txt` files. |
| [work-templates](work-templates/README.en.md) | Work context templates | Manage work status, long-term tasks, and design decisions separately in Markdown so work can resume when the person, host, or AI session changes. |

These resources support the development process (`app-generation-workflow`),
the creation of build definitions (`ai-cmake`), and continuity of work
(`work-templates`), respectively. Both app-generation-workflow and work-templates
can also be used in projects that do not use Qt.

## Featured: App Generation Workflow

`app-generation-workflow` is a document-driven AI development harness focused on
GUI applications. It provides procedures and templates that connect requirements
gathering, specification, implementation, testing, and review.

Describe what you want to build, organize usage scenarios, screens and
interactions, and business rules, then turn them into implementation instructions
for AI. After generation, check behavior against the specifications and document
how the implementation corresponds to them.

### From Requirements to Implementation and Verification

```text
Requirements notes (.txt)
       │ Requirements review
       ▼
Pre-specification notes (source_memo.md)
       │ Organize goals, operations, rules, exclusions, and open questions
       ▼
Three key specifications
  ├─ Use cases      Usage scenarios and expected outcomes
  ├─ UI             Screens, input, display, and interactions
  └─ Business layer Data, validation, calculation, and persistence
       │ Review, then select the implementation approach
       ▼
Implementation prompt → Application implementation
       │ Build, tests, manual checks, and implementation review
       ▼
Record specification-to-code correspondence → Application design document
```

AI drafts documents, identifies gaps and contradictions, implements code, writes
tests, and checks consistency. People decide what to build, which specifications
to adopt, and whether to accept the generated results. Reviews at each stage
check the conditions for proceeding and where to return when changes are needed.

### The Three Key Specifications

The material gathered in the pre-specification notes is organized into three
perspectives.

| Document | File | What it describes |
|---|---|---|
| Use-case specification | `01_usecase_spec.md` | User goals, interaction sequences, success conditions, and behavior in exceptional cases |
| UI specification | `02_ui_spec.md` | Screen layout, input and display, conditions for available operations, and error presentation |
| Business-layer specification | `03_business_spec.md` | UI-independent data, validation, calculation, persistence, and state changes |

For example, for an "open file" feature, the use-case specification defines the
usage sequence and failure behavior, the UI specification defines the open
action and error presentation, and the business-layer specification defines
loading and data validation. Comparing the same feature from these perspectives
helps reveal inconsistencies between the interface and its underlying behavior.

The implementation language and framework are specified in the implementation
prompt after this preparation. The workflow can also be used in projects that do
not use Qt.

### Verification and Revision After Generation

Key verification and revision steps:

- Derive expected test results from the specifications and compare them with the generated code's output
- Perform necessary manual checks, such as screen interactions, alongside automated tests
- When a specification decision changes, update the pre-specification notes, the three specifications, the implementation prompt, and then the code
- When the specifications are correct and only transcription or implementation is faulty, confirm the source specification and return to the affected stage
- Recheck artifacts affected by upstream changes and record both verified results and items that remain unverified
- After implementation review confirms that no major inconsistencies remain, document specification-to-code correspondence and the overall application design

Reuse these documents for subsequent changes, so feature additions and
corrections to misunderstood intent can be made while checking the connections
between requirements, specifications, and implementation.

### Getting Started

Read the [User Guide](app-generation-workflow/app-generation-workflow/USER_GUIDE.en.md).
Copy the repository's inner `app-generation-workflow/app-generation-workflow/`
directory to `app-generation-workflow/` in your working location. Prepare a `.txt`
file containing your requirements (`requirement.txt` in this example), then
start with an instruction to AI such as:

```text
Read app-generation-workflow/three_key_documents_workflow.md.
Review the requirements in requirement.txt.
Identify missing information and open questions before creating the pre-specification notes.
```

Keep application-specific specifications, implementation prompts, and source
code in a separate application directory, apart from the templates. See the
[workflow README](app-generation-workflow/app-generation-workflow/README.en.md)
and [User Guide](app-generation-workflow/app-generation-workflow/USER_GUIDE.en.md)
for the directory structure and detailed procedure.

## Featured: Work Templates

`work-templates` provides document templates for recording the current state of
work, next steps, and the reasons behind decisions in three Markdown files.
Use them to hand work to another person, move to another PC, or resume in a new
AI session.

Store what is complete, what remains unverified, and why an approach was chosen
alongside the project so they can be read again during the next session. When
combined with the app generation workflow, these files record references to
specifications and code, review progress, and the next stage to check.

### Handing Over Work Through Three Files

```text
Work, verification, and decisions
       │ Update the records at a natural stopping point
       ▼
Project
  ├─ NEXT.md       Current state and next steps
  ├─ DECISIONS.md  Decisions, reasons, and lasting constraints
  └─ backlog.md    Future work and deferred items
       │ Share through Git together with the necessary changes
       ▼
Next person, PC, or AI session
       │ Read the records and compare them with actual files and Git state
       ▼
Resume work
```

| File | Information to keep | Example entry |
|---|---|---|
| `NEXT.md` | Current state, the next one or two tasks, files to work on, and notes for resuming | File loading is implemented. Next, check what happens when opening a corrupt file. |
| `DECISIONS.md` | Accepted and rejected decisions with reasons, open questions, and lasting specifications or constraints | Preserve the current data if loading fails, so work in progress is not lost. |
| `backlog.md` | Work outside the immediate scope, known problems, reasons for deferral, and conditions for reconsideration | Consider loading performance for large files after verifying normal loading behavior. |

In this example, the next person can start by checking failure behavior. They
can separately find the reason for the expected behavior and the work outside
the current scope. Keep detailed verification logs and completion history in
reports, Git history, or other reference material.

### Shared Rules and Everyday Use

Place `AGENTS.next.md` at `~/AGENTS.next.md` on each host to use the same reading
and updating procedure across projects. The opening instructions in each
project's `NEXT.md` direct the reader to consult these shared rules first.

At a natural stopping point, ask AI:

```text
Update NEXT.md to reflect the current state of the work.
```

The shared rules instruct AI to review Git diffs, recent commits, and related
documents, then update the three files as needed. Put immediate next steps in
`NEXT.md`, lasting decisions in `DECISIONS.md`, and future work in `backlog.md`.

When resuming, ask:

```text
Read NEXT.md.
```

Following the shared rules, AI also reads the other two files and uses the
records to check the work state. The handoff relies on rereading saved documents
rather than conversation history or AI memory alone. At a stopping point, AI
prompts for an update when changes remain unrecorded; updates are made when
requested.

### Getting Started and Resuming on Another PC

1. Copy `work-templates/AGENTS.next.en.md` to `~/AGENTS.next.md`.
2. For a new project without work-management files, copy `NEXT.en.md`, `DECISIONS.en.md`, and `backlog.en.md` from `work-templates/` as `NEXT.md`, `DECISIONS.md`, and `backlog.md`. If these files already exist, retain their contents without overwriting them.
3. Replace instructional placeholders in the copied templates with actual work information.
4. Track the three files in Git and make the source and document changes needed to resume available alongside them.

On another PC, install the same shared rules and obtain the necessary changes
before rereading the records. If uncommitted changes exist, record their
presence and transfer the changes themselves through a commit or patch. The
required branch, baseline commit, tool versions, and environment conditions
also help the next session resume.

See the [work-templates README](work-templates/README.en.md) and
[User Guide](work-templates/USER_GUIDE.en.md) for copying instructions and example entries.
