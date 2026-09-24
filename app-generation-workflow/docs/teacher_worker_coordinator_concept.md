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

<h1 align="center">Teacher-Worker Coordinator</h1>
<p class="slide-subtitle">構想: 文書の受け渡しを半自動化する</p>

Teacher-Worker の役割分離を、実行可能な仕組みにします。

!dot{scale=0.76}
```dot
digraph G {
  graph [rankdir=LR, bgcolor="transparent", margin=0.04, nodesep=0.55, ranksep=0.65];
  node [shape=box, style="rounded,filled", fontname="Hiragino Sans", fontsize=16, margin="0.17,0.11"];
  edge [color="#374151", arrowsize=0.72, fontname="Hiragino Sans", fontsize=12];

  Human [label="人\n依頼・承認・最終判断", fillcolor="#dbeafe", color="#2563eb"];
  Coordinator [label="Teacher-Worker Coordinator\n起動・受け渡し・状態・停止", fillcolor="#f3e8ff", color="#7e22ce"];
  Teacher [label="Teacher AI\n計画・境界・評価", fillcolor="#fef3c7", color="#d97706"];
  Worker [label="Worker AI\n成果物の作成・検証", fillcolor="#dcfce7", color="#16a34a"];

  Human -> Coordinator [label="依頼"];
  Coordinator -> Human [label="承認待ち"];
  Coordinator -> Teacher [label="起動"];
  Teacher -> Coordinator [label="worker_task.md"];
  Coordinator -> Worker [label="作業を起動"];
  Worker -> Coordinator [label="worker_report.md"];
}
```

| 自動化すること | 自動化しないこと |
| --- | --- |
| AIの起動、文書の受け渡し、状態と停止の管理 | 仕様の採用、リスク受容、重要事項の最終判断 |

**近い既存構想**

| 構想 | 近い点 | 今回の違い |
| --- | --- | --- |
| Magentic-One | 計画、割当、進捗を管理する | Teacher と Coordinator を分離する |
| LangGraph | 状態保存、中断、再開、人の承認を扱う | 2つの文書を受け渡しの正本にする |
| OpenAI Agents SDK | Agent の起動、handoff、承認を管理する | 会話履歴全体を Agent 間で共有しない |

**最初は、工程ごとに人が承認する半自動運用から始めます。**
