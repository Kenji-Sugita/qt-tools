# AGENTS

## アプリ実装方針

`qeyes` は、マウスポインターを追う目を常時表示する小さな Qt Widgets アプリとして実装する。

### 基本方針

- `Qt6::Widgets` を使う。
- 描画は `QPainter` で行う。
- QML / Qt Quick は使わない。
- 軽量な常駐ツールとして振る舞うことを優先する。
- 不要に大きな依存や複雑な構成を追加しない。

### UI 方針

- ウィンドウはフレームレスとする。
- 背景は透過にする。
- 最前面表示を維持する。
- 左ドラッグで移動できるようにする。
- 左ダブルクリックで終了できるようにする。
- 設定 UI は右クリックから開けるようにする。

### 描画方針

- 目はコードで動的に描画する。
- 画像アセットに依存しない。
- 目の構成要素は輪郭、白目、虹彩、瞳孔とする。
- 視線はマウスポインター方向へ追従する。
- 追従時も各要素が白目の外にはみ出さないようにする。

### 設定方針

- 永続化には `QSettings` を使う。
- 保存対象は明示された設定項目に限る。
- 保存キーは `colors/...` のように用途ごとに整理する。
- 一時的な見た目調整のために過剰な設定項目を増やさない。

### プラットフォーム方針

- まず共通実装を優先する。
- macOS 固有の見た目差分は必要最小限に留める。
- 他プラットフォームに不要な特例は持ち込まない。

### 変更時の注意

- 既存操作を壊す変更は避ける。
- 見た目の変更は小さなツールとしての一貫性を保つ。
- 新機能追加時は、操作方法、保存仕様、文書も合わせて更新する。

## リソース運用方針

### アイコン

- アイコン関連ファイルは `assets/` 配下で管理する。
- 正本画像は `assets/icon-master.png` とし、各形式はこれを元に展開する。
- ファイル名は既存の命名を維持し、カレントディレクトリー名や別名を付けない。
- アプリ用アイコンを変更するときは、少なくとも次を整合させる。
- `assets/qeyes.icns`
- `assets/qeyes.ico`
- `assets/icon.iconset/icon_16x16.png`
- `assets/icon.iconset/icon_16x16@2x.png`
- `assets/icon.iconset/icon_32x32.png`
- `assets/icon.iconset/icon_32x32@2x.png`
- `assets/icon.iconset/icon_128x128.png`
- `assets/icon.iconset/icon_128x128@2x.png`
- `assets/icon.iconset/icon_256x256.png`
- `assets/icon.iconset/icon_256x256@2x.png`
- `assets/icon.iconset/icon_512x512.png`
- `assets/icon.iconset/icon_512x512@2x.png`
- `CMakeLists.txt` や `qeyes.rc` から参照されるファイル名は原則変更しない。

### 変更時の注意

- リソース差し替え時は、正本が `assets/icon-master.png` であることを前提に更新する。
- 生成物を更新した場合は、必要な派生ファイルもまとめて更新する。
- 一部プラットフォームだけ古いアイコンが残る状態を避ける。

## 文書運用ルール

このプロジェクトの文書は `docs/` 配下に次の構成で配置する。

```text
docs/
  *.md
  guides/
    USER_GUIDE-template.md
    USER_GUIDE-<version>.md
  releases/
    ReleaseNote-template.md
    ReleaseNote-<version>.md
  history/
    RELEASE_HISTORY.md
```

## ルール

- 利用ガイドは `docs/guides/USER_GUIDE-<version>.md` に置く。
- リリースノートは `docs/releases/ReleaseNote-<version>.md` に置く。
- 全体の変更履歴は `docs/history/RELEASE_HISTORY.md` にまとめる。
- テンプレートは `docs/guides/USER_GUIDE-template.md` と `docs/releases/ReleaseNote-template.md` に置く。
- `docs/` 直下の補助文書は、運用メモや判断材料の記録として置いてよい。
- ドキュメントのファイル名は固定命名とし、カレントディレクトリー名やプロジェクト名を接頭辞・接尾辞として付けない。
- `USER_GUIDE.md`、`ReleaseNote.md`、`ReleaseNote.txt` のような重複ファイルは新たに作らない。
- 新しいバージョンを追加するときは、次をまとめて更新する。
- ビルド定義上のバージョン番号
- `docs/guides/USER_GUIDE-<version>.md`
- `docs/releases/ReleaseNote-<version>.md`
- `docs/history/RELEASE_HISTORY.md`
- テンプレートの更新が必要ならそれも含める
- 同じ内容を `.md` と `.txt` の両方で管理しない。
- 過去バージョンの状態は Git タグで追跡する。
- Finder などが生成する `.DS_Store` のような不要ファイルは追跡しない。
- 文書を追加した後は `git status` を確認し、未追跡のまま残っていないことを確認する。
- 他ホストで参照させる前に、必要な文書が add・commit・push 済みであることを確認する。

### 文書追加時の更新ルール

- 既存種別の文書を追加した場合は、必要に応じて `docs/history/RELEASE_HISTORY.md` を更新する。
- 新しい文書種別を追加した場合は、`docs/README.md` にその種別と役割を追記する。
- 利用者が参照すべき主要文書を追加した場合は、ルート `README.md` のリンクも更新する。
- 文書配置や命名ルール自体に影響する場合は、`AGENTS.md` も更新する。

### 工程ごとの修正ルール

- 各工程で問題が起きた場合は、まずその工程の文書を修正対象とする。
- 修正対象には、その工程の入力文書、テンプレート、`AGENTS.md`、`GENERATION_INSTRUCTION.md` を含めて確認する。
- `request` の問題は `REQUEST.md` と `docs/request/qeyes/AGENTS.md` を中心に見直す。
- `requirements` の問題は要件定義書、要件テンプレート、`docs/requirements/qeyes/AGENTS.md` を中心に見直す。
- `architecture` の問題は概要設計書、概要設計テンプレート、`docs/architecture/qeyes/AGENTS.md` を中心に見直す。
- `design` の問題は詳細設計書、詳細設計テンプレート、`docs/design/qeyes/AGENTS.md` を中心に見直す。
- `implementation` の問題は実装工程文書と `docs/implementation/qeyes/AGENTS.md` を中心に見直す。
- `verification` の問題は検証工程文書と `docs/verification/qeyes/AGENTS.md` を中心に見直す。
- 問題が上流工程に起因すると判断できる場合は、その上流工程まで戻って修正する。

## README 運用ルール

- ルートの `README.md` が存在する場合は、このリポジトリの入口として扱う。
- `README.md` を作成または更新する場合は、少なくとも次を含める。
- プロジェクト概要
- ビルド方法
- 起動方法
- 主な操作
- 関連文書へのリンク
- `docs/README.md` は `docs/` 配下の案内として扱い、詳細な運用ルールは必要に応じて別文書へ分ける。
- `README.md` には詳細な文書運用ルールを重複して長く書きすぎない。
- 新機能追加やバージョン更新で利用者向けの説明が変わる場合は、`README.md` も更新対象に含める。
