# backlog

このファイルには、次回すぐには着手しない長期の残作業、問題点、保留事項を書く。
現在の状態と次回の作業再開に直接必要な項目は `NEXT.md`、今後も有効な仕様、設計判断、不採用理由、運用制約は `DECISIONS.md` に書く。
完了履歴や検証ログの保管場所にはしない。

## 残作業

- `compare` は `hotspots` 差分比較に限定しており、`ranges` の自動比較は未対応。
- `compare` の上位 N 件表示、差分しきい値、`changed` のみ表示などの絞り込みは未対応。
- `hotspots` / `ranges` の `limit`, `file`, `type`, `from`, `to` などの絞り込みは未対応。
- file / line 単位の詳細統計は未対応。
- caller / callee 解析は未対応。
- flame graph 相当の集計は未対応。
- Markdown table 出力は未対応。
- JSON 出力、CSV 出力、機械可読 structured output は未対応。
- CI 自動判定は未対応。
- Qt バージョン差異の吸収は未確認。

## 問題点

- すぐに対応が必要な既知の問題点はない。
- 大容量ファイル向け最適化は未対応。現時点では実測や具体事例による裏付けはないため、問題点ではなく予測リスクとして扱う。読み込み時間、メモリ、`compare` の遅さが確認できた場合に問題点へ昇格して検討する。

## 保留事項

- `problemtrace.qtd` は 184 MB と大きいため、git 管理対象にするかは要確認。現状は git には入れない方針が自然。
- `problemtrace-analysis-package.zip` は `.qtd` を含むためメール添付にはやや大きい。
- メール共有用には `problemtrace-analysis-package-lite.zip` を使う。
- 必要なら `problemtrace.qtd` を含む完全版 ZIP は大容量共有に回す。
- 外れ値判定の閾値設計は今後の調整余地がある。
- バースト判定ルールは今後の調整余地がある。
