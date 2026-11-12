# Qt Tools

This repository brings together Qt/C++ development tools, interactive programming
environments, document creation tools, application development workflows and
document templates, templates for managing work context, and everyday utilities.

Most of the applications and libraries use Qt. The repository also includes
Python and Bash tools, along with files that support Qt development using
generative AI.

[日本語](README.md)

## Find Resources by Purpose

| Area | Contents | Start here |
|---|---|---|
| Tools | Executable tools and libraries for applications | [Tools by purpose](tools/README.en.md) |
| Workflows | Procedures, instructions, and templates to copy and apply | [Workflow catalogue](workflows/README.en.md) |
| Docs | Learning and reference material, with reading routes to existing guides | [Documentation and reading order](docs/README.en.md) |

## Featured: Interactive Qt/C++

Experiment interactively with Qt Cling and icpp, and inspect running objects with ObjectSelector and PropertyEditor.
See the [detailed introduction and tool catalogue](tools/README.en.md#featured-interactive-qtc).

## Featured: App Generation Workflow

Connect requirements, source notes, use-case/UI/business specifications, implementation, and review through documents.
See the [detailed introduction and getting-started guide](workflows/README.en.md#featured-app-generation-workflow).

## Featured: Work Templates

Record current state, decision reasons, and longer-term work in Markdown to hand work across people, PCs, and AI sessions.
See the [detailed introduction and examples](workflows/README.en.md#featured-work-templates).

## Documentation

Keep project-specific guides and examples with their tools or workflows.
Use [docs/](docs/README.en.md) for reading routes and [PUBLICATION_MAP.md](PUBLICATION_MAP.md) for Sites sources, published destinations, and previous locations.

Japanese and English documents share directories, using `.md` and `.en.md` (`.txt` for Cling's README).
The root `NEXT.md`, `DECISIONS.md`, and `backlog.md` are maintainer working records. Reusable templates are under [work-templates](workflows/work-templates/README.en.md).


## Platforms

Supported environments vary by tool.

The main target platforms and framework version are:

- macOS
- Linux
- Windows
- Qt 6

The command-line tools target macOS and Linux, with a few exceptions such as
genpdf.

Some tools have specific restrictions or requirements:

- VuStereo is available only on macOS.
- `tl` requires authentication to be configured for Google Cloud Translation.
- Qt Cling and icpp require Cling and a compatible Qt environment.

See each tool's documentation for details.

## About This Repository

Many of these tools were created to meet practical needs in Qt/C++ development,
seminars, and document preparation, and have been improved through continued use.

The emphasis is on tools that can be used independently and combined as needed,
rather than on building a large integrated environment.
