# DECISIONS

このファイルには、今後も有効な仕様、設計判断、不採用理由、未決論点、判断基準、運用制約を書く。
現在の状態と次回再開に必要な手順は `NEXT.md`、次回すぐには着手しない長期の残作業、問題点、保留事項は `backlog.md` に書く。

## 採用済み

- 入力形式は `.qtd` / XML と `.qzt` / binary の両方を扱う。
- `convert` は `.qzt -> .qtd` と `.qtd -> .qzt` の双方向に対応する。
- `.qtd` / `.qzt` writer は `convert` 用に使い、CLI の汎用保存コマンドとしては公開しない。
- `hotspots` の集約キーは `eventIndex` ではなく、`type + file + line + column` 系の安定キーを使う。
- `compare` は現時点では `hotspots` 差分比較に限定する。
- 初回 / 再訪比較は機能実装ではなく、まず運用ガイドで扱う。

## 不採用

- 現時点では JSON / CSV / 機械可読 structured output を優先実装しない。
- 現時点では Markdown table / compact text 出力を優先実装しない。
- 現時点では CI 自動判定を優先実装しない。

## まだ決めていないこと

- `problemtrace.qtd` のような大容量 trace を git 管理対象にするか。
- `compare` に `ranges` 自動比較を追加するか。
- `compare` / `hotspots` / `ranges` の絞り込みオプションを追加するか。
- file / line 詳細統計、caller / callee 解析、flame graph 相当の集計を追加するか。

## 判断基準

- 現行の text 出力は人が読め、情報量を維持しており、AI に渡して解析・報告書作成できているため、出力形式改善は急がない。
- 利用中に出力量や比較結果の確認で困った場合に、`compare` / `hotspots` / `ranges` の絞り込みを検討する。
- 大容量ファイル向け最適化は、現時点では実測や具体事例による裏付けがないため、問題点ではなく予測リスクとして扱う。読み込み時間、メモリ、`compare` の遅さが確認できた場合に問題点へ昇格して検討する。
- Qt バージョン差異は、読み込み失敗や分類差が見つかった時点で対応する。

## 継続する仕様・運用制約

- 指示にない機能追加や仕様変更は、実装前に提案して許可を得る。
- リファクタでは既存仕様の維持を優先し、必要最小限の変更にとどめる。
- `CMakeLists.txt` を作成または修正する前に `~/AGENTS.cmake.md` を読む。
- `src` と `include` ディレクトリの構成を維持し、必要に応じて新しいファイルを追加する。
- ひとつのファイルに複数のクラスを定義することは避け、クラスごとにファイルを分ける。
- 配布用 ZIP はコミット対象外の成果物として扱う。
- リリース用 ZIP のトップディレクトリは `qmlprofileranalyzer/` とする。
- 配布 ZIP には `build/`, `asaki/`, 既存 `.zip`, `.DS_Store`, `NEXT.md`, `advice.txt` を含めない。
- 配布 ZIP の `docs/` は `docs/FORMAT.md` のみ含める。
