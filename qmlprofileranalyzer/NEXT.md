# NEXT

このファイルを読む、または更新する前に、`~/AGENTS.next.md` を読み、その指示に従うこと。
`~/AGENTS.next.md` が存在しない場合は、このファイルを整理・削除せず、利用者に確認すること。
このルールは、ユーザーから明示的な指示がない限り変更しない。

## 現在の状態

- `qmlprofileranalyzer-release-20260508.zip` はリリース済み。
- 次の判断はユーザーとコンサルタントの反応待ち。
- すぐに必要な機能追加はない。
- 現行の text 出力は人が読め、情報量を維持しており、AI に渡して解析・報告書作成できている。
- 配布用 ZIP はコミット対象外の成果物として扱う。

## 次にやること

- ユーザーまたはコンサルタントから反応が来たら内容を確認し、必要な対応を判断する。
- 問題、要望、追加実装の判断時は `backlog.md` と `DECISIONS.md` を確認する。
- メール共有用には `problemtrace-analysis-package-lite.zip` を使う。
- 必要なら `problemtrace.qtd` を含む完全版 ZIP は大容量共有に回す。

## 参照

- 長期の残作業、問題点、保留事項: `backlog.md`
- 有効な仕様、設計判断、運用制約: `DECISIONS.md`
- 実装状況: `docs/STATUS.md`
- 利用者向け説明: `README.md`, `USER_GUIDE.md`
- フォーマット情報: `docs/FORMAT.md`

## 配布 ZIP

- ファイル: `qmlprofileranalyzer-release-20260508.zip`
- 状態: 配布済み、コミット対象外
- 内容:
  - トップディレクトリあり: `qmlprofileranalyzer/`
  - 60 files
  - 約 2.9 MB
  - `docs/` は `docs/FORMAT.md` のみ
  - `NEXT.md` と `advice.txt` は含めない
  - `build/`, `asaki/`, 既存 `.zip`, `.DS_Store` は含めない

## 注意

- `AGENTS.md` の方針により、指示にない機能追加や仕様変更は実装前に提案して許可を得る。
- 既存の未追跡ファイルや他ディレクトリの変更は勝手に削除しない。
- `CMakeLists.txt` を作成または修正する前に `~/AGENTS.cmake.md` を読む。
