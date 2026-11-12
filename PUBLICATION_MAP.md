# Sites正本・公開先・旧配置の対応 / Publication Map

正本は `~/Sites`、このリポジトリーはGitHub公開用です。正本は今回移動しません。
The canonical sources remain in `~/Sites`; this repository holds the public distribution.

| Sites source (relative to ~/Sites) | Published directory | Previous directory |
|---|---|---|
| `tools/LaserPointer/` | [tools/LaserPointer/](tools/LaserPointer/) | `LaserPointer/` |
| `technical-document/ai-cmake/` | [workflows/ai-cmake/](workflows/ai-cmake/) | `ai-cmake/` |
| `tools/anker-layout/` | [tools/anker-layout/](tools/anker-layout/) | `anker-layout/` |
| `technical-document/app-generation-workflow/` | [workflows/app-generation-workflow/](workflows/app-generation-workflow/) | `app-generation-workflow/` |
| `tools/qtcling/` | [tools/cling/](tools/cling/) | `cling/` |
| `tools/cutter/` | [tools/cutter/](tools/cutter/) | `cutter/` |
| `tools/dateorder/` | [tools/dateorder/](tools/dateorder/) | `dateorder/` |
| `tools/dtree/` | [tools/dtree/](tools/dtree/) | `dtree/` |
| `tools/icpp/` | [tools/icpp/](tools/icpp/) | `icpp/` |
| `tools/genpdf/` | [tools/markdown/](tools/markdown/) | `markdown/` |
| `tools/now/` | [tools/now/](tools/now/) | `now/` |
| `tools/objectselector/` | [tools/objectselector/](tools/objectselector/) | `objectselector/` |
| `tools/propertyeditor/` | [tools/propertyeditor/](tools/propertyeditor/) | `propertyeditor/` |
| `tools/qeyes/` | [tools/qeyes/](tools/qeyes/) | `qeyes/` |
| `tools/qmlprofileranalyzer/` | [tools/qmlprofileranalyzer/](tools/qmlprofileranalyzer/) | `qmlprofileranalyzer/` |
| `tools/qmlprop/` | [tools/qmlprop/](tools/qmlprop/) | `qmlprop/` |
| `tools/seminartimer/` | [tools/seminartimer/](tools/seminartimer/) | `seminartimer/` |
| `tools/tl/` | [tools/tl/](tools/tl/) | `tl/` |
| `tools/vu-stereo/` | [tools/vu-stereo/](tools/vu-stereo/) | `vu-stereo/` |
| `tools/whiteboard/` | [tools/whiteboard/](tools/whiteboard/) | `whiteboard/` |
| `work-templates/ + tools/work-templates/` | [workflows/work-templates/](workflows/work-templates/) | `work-templates/` |
| `technical-document/threekeydoctolargesystem/` | [docs/development/three-key-documents/](docs/development/three-key-documents/) | 新規 / New |
| `technical-document/tech-seminar/next-md/` | [docs/seminars/next-md/](docs/seminars/next-md/) | 新規 / New |
| `technical-document/ai-assisted-book-writing/` | [docs/practice/ai-assisted-book-writing/](docs/practice/ai-assisted-book-writing/) | 新規 / New |
| `technical-document/markdown/` | [docs/basics/markdown-for-ai/](docs/basics/markdown-for-ai/) | 新規 / New |
| `technical-document/ai-usage-tips/` | [docs/basics/ai-usage-tips/](docs/basics/ai-usage-tips/) | 新規 / New |
| `technical-document/codex/` | [docs/safety/personal-ai-usage-rules/](docs/safety/personal-ai-usage-rules/) | 新規 / New |

## 同期・移行時の注意 / Synchronization and Migration

- 個別プロジェクトの内部構成、API、ビルドターゲット、ZIP・PDFは保持する。 / Preserve internal project layouts, APIs, build targets, ZIPs, and PDFs.
- Whiteboardの現行ソースZIPは `tools/whiteboard/whiteboard-app/` に対応する。アプリ生成ワークフローの内側の配布本体は `workflows/app-generation-workflow/app-generation-workflow/` に対応する。 / These nested distribution roots remain unchanged.
- 新規の横断文書は `docs/` のカテゴリーへ資料単位で配置し、各資料のREADMEへ正本の場所を記録する。 / Add new cross-project documents as self-contained resources under the appropriate docs category and record their canonical source in their README.
- 日本語・英語・PDF・付属物は資料ごとにまとめる。 / Keep languages, PDFs, and supporting files together.
- 同じ本文を複数箇所にコピーせず、索引から参照する。 / Link from indexes instead of duplicating documents.
- 古いGitHub URLやcheckout直下のパスは変更される。過去版はGit履歴で参照できる。 / Old GitHub URLs and checkout-relative paths change; Git history preserves earlier layouts.
- 過去のPDF・ZIP内のパスは配布時点の記録として残す。 / Paths inside existing PDFs and ZIPs describe their original distribution.
