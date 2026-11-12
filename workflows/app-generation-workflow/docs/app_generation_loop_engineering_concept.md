---
genpdf:
  format: slides
  font_size: 20px
  page_numbers: false
  table:
    header_background: "#e8f1ff"
    header_color: "#1f2937"
    border_color: "#cbd5e1"
    border_width: "1px"
    stripe: true
    stripe_background: "#f8fafc"
    cell_padding: "0.35em 0.55em"
    font_size: "0.86em"
    compact: true
    align: left
    header_align: center
    cell_align: left
---

<h1 align="center">アプリ生成ワークフローの Loop engineering 化</h1>
<p class="slide-subtitle">構想: 戻り方を、再現可能な仕組みにする</p>

現在の戻り手順に、開始条件、停止条件、状態管理を加えます。

!dot{scale=0.68}
```dot
digraph G {
  graph [rankdir=LR, bgcolor="transparent", margin=0.04, nodesep=0.42, ranksep=0.58];
  node [shape=box, style="rounded,filled", fontname="Hiragino Sans", fontsize=15, margin="0.15,0.09"];
  edge [color="#374151", arrowsize=0.70, fontname="Hiragino Sans", fontsize=11];

  Run [label="実行", fillcolor="#eef2ff", color="#4f46e5"];
  Verify [label="検証", fillcolor="#eef2ff", color="#4f46e5"];
  Classify [label="原因を分類", fillcolor="#fef3c7", color="#d97706"];
  Return [label="戻り先を決定", fillcolor="#fef3c7", color="#d97706"];
  Fix [label="修正・再実行", fillcolor="#eef2ff", color="#4f46e5"];
  Repeat [label="繰り返す問題", fillcolor="#fee2e2", color="#dc2626"];
  Improve [label="ハーネス改善", fillcolor="#dcfce7", color="#16a34a"];

  Run -> Verify;
  Verify -> Classify;
  Classify -> Return;
  Return -> Fix;
  Fix -> Run [label="内側: 完了まで"];
  Classify -> Repeat [label="再発"];
  Repeat -> Improve [label="外側"];
  Improve -> Run [label="次回から使用"];
}
```

| 現在 | 仕組み化後 |
| --- | --- |
| 戻り先を手順で示す | 条件から戻り先を決める |
| 人が進行を管理する | 状態を記録して再開できる |
| 完了をレビューで判断する | 停止条件と人への確認条件を明示する |
| 問題ごとに修正する | 繰り返す問題をハーネスへ戻す |

**最初は手動でループ定義を運用し、安定した部分だけを自動化します。**

仕様の採用と重要な業務判断は、人が行います。
