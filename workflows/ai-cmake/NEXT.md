# NEXT

このファイルを読む、または更新する前に、`~/AGENTS.next.md` を読み、その指示に従うこと。
`~/AGENTS.next.md` が存在しない場合は、このファイルを整理・削除せず、利用者に確認すること。
このルールは、ユーザーから明示的な指示がない限り変更しない。

## 現在の目的

- `ai-cmake` は、生成 AI に Qt 6 用 `CMakeLists.txt` を生成・修正・更新させるとき
  の出力方針をそろえるための指示ファイル配布物。
- 旧ディレクトリ名 `codex-cmake` は `mv` で `ai-cmake` に変更済み。
- Codex 専用ではなく、「生成 AI 向け CMake 指示ファイル」として説明する方針。

## 現在の主なファイル

- `agents/AGENTS.md`
  - 作業対象リポジトリに置く基本指示。
  - `CMakeLists.txt` を新規作成または更新するときは、ソースコードだけから自由に判
    断せず、必ず `~/AGENTS.cmake.md` の方針と該当パターンに従う、という指示に更
    新済み。
- `agents/AGENTS.cmake.md`
  - Qt 6 用 CMake 方針と 33 パターンを含む本体。
  - 既存 21 パターンに加えて、QML 応用 12 パターンを追加済み。
- `agents/README.md`
  - `agents/` 配下の使い方説明。作成・修正・更新の表現に更新済み。
- `USER_GUIDE.md`
  - 「生成 AI 向け CMake 指示ファイル 利用ガイド」。
  - コード追加時の `CMakeLists.txt` 更新、qmake から CMake への移行、短い Codex 依
    頼例を追記済み。
- `USER_GUIDE.pdf`
  - `USER_GUIDE.md` から再生成済み。
- `ai-cmake.zip`
  - 配布用 zip。トップディレクトリは `ai-cmake/`。
  - 33 パターン追加後に再作成済み。

## 配布 zip の方針

- `ai-cmake.zip` のトップは `ai-cmake/`。
- zip に含めるもの:
  - `ai-cmake/USER_GUIDE.pdf`
  - `ai-cmake/USER_GUIDE.md`
  - `ai-cmake/agents/AGENTS.cmake.md`
  - `ai-cmake/agents/AGENTS.md`
  - `ai-cmake/agents/README.md`
- zip に含めないもの:
  - `NEXT.md` はコンテキスト復元用なので除外。
  - `.DS_Store` は除外。
  - `validation/` は検証用なので配布 zip には入れない方針。

## 検証状況

- `validation/` に CMake 検証用プロジェクトを作成済み。
- 目的はアプリ機能検証ではなく、`AGENTS.cmake.md` に含まれるパターンの
  `CMakeLists.txt` が実際の Qt 6 環境で configure/build できることの確認。
- 検証環境:
  - CMake 3.30.5
  - Ninja
  - Qt prefix: `/usr/local/qt/Qt/6.10.2/macos`
- 結果:
  - 21/21 パターン configure/build 成功。
  - Qt Test 系の CTest も 3/3 成功。
  - 追加候補 QML 12/12 パターン configure/build 成功。
  - 追加候補 QML の CTest も 1/1 成功。
- 検証関連ファイル:
  - `validation/generate_projects.py`
  - `validation/run_builds.py`
  - `validation/projects/`
  - `validation/builds/`
  - `validation/logs/`
  - `validation/validation_report.md`
  - `validation/generate_qml_candidate_projects.py`
  - `validation/run_qml_candidate_builds.py`
  - `validation/qml_candidates/`
- 注意:
  - `run_builds.py` の Qt パスは現在 `/usr/local/qt/Qt/6.10.2/macos` に固定。
  - 他ホストで検証するなら、Qt パスをその環境に合わせる必要がある。
  - 他人に見せるなら `validation/README.md` を追加し、「CMake パターン検証であ
    り、アプリ機能検証ではない」と明記するとよい。

## 次にやること

- 必要なら `validation/README.md` を追加して、検証の目的と実行方法を明文化する。
- zip を作り直す場合は、必ず `NEXT.md`、`.DS_Store`、`validation/` を除外する。

## 未完了

- 次回の作業判断に必要な未完了だけを書く
- 詳細な残作業、問題点、保留事項は `backlog.md` に移す

- `validation/README.md` はまだ未作成。
- `ai-cmake.zip` は 33 パターン追加後に再作成済み。配布直前に内容確認する。

## 注意

- `NEXT.md` は zip には含めない。
- `validation/` はリリース検証用。配布物に含めるかは要確認だが、現時点の方針では
  含めない。
- `AGENTS.cmake.md` の翻訳パターンは `LinguistTools` コンポーネントが必要。検証用
  プロジェクトでは `find_package(Qt6 REQUIRED COMPONENTS Widgets LinguistTools)`
  で通している。
