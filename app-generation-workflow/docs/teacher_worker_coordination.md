# Teacher-Worker から外部コーディネーションへ

## 位置づけ

外部コーディネーションは、Teacher-Worker engineering で定義した役割分離と文書に
よる受け渡しを、実行可能な仕組みにする次の段階である。

```text
Teacher-Worker engineering
        ↓ 役割と受け渡しを形式化する
Coordination engineering
```

外部コーディネーターは、Teacher や Worker の判断を置き換えない。
Teacher と Worker を別のプロセスまたは作業コンテキストで起動し、
`worker_task.md` と `worker_report.md` の受け渡し、状態遷移、停止を管理する。

## 発展段階

```text
1. 役割分離
   Teacher と Worker を別の作業コンテキストにする

2. 文書による受け渡し
   worker_task.md と worker_report.md を使う

3. 手動コーディネーション
   人が各 AI を起動し、文書を受け渡す

4. 半自動コーディネーション
   外部ツールが AI を起動し、人が工程ごとに承認する

5. 自動コーディネーション
   外部ツールが状態遷移、再実行、停止を管理する
```

完全自動化を最終目標にする必要はない。
重要な仕様変更、対象外の変更、リスク受容、完了判断では人の承認を残すため、半自動
コーディネーションが実務上の安定した到達点になり得る。

## 基本構成

```text
人が目的、背景、制約、期待する成果を伝える
    ↓
コーディネーターが Teacher を起動する
    ↓
Teacher が worker_task.md を作成する
    ↓
コーディネーターが Worker を起動する
    ↓
Worker が作業し、worker_report.md を作成する
    ↓
コーディネーターが Teacher を再起動する
    ↓
Teacher が完了、次の作業、人への確認のいずれかを判断する
```

Teacher と Worker が直接通信するのではなく、外部コーディネーターが文書と状態を
介して両者を接続する。

## コーディネーターの責務

- Teacher と Worker を別プロセスまたは別コンテキストで起動する。
- `worker_task.md` と `worker_report.md` の作業 ID が対応していることを確認する。
- 実行中、報告待ち、レビュー待ち、完了などの状態を管理する。
- タイムアウト、実行失敗、無限ループを検出して停止する。
- 人の承認が必要な場合に自動実行を停止する。
- 実行履歴、判断理由、参照した正本を記録する。
- Teacher や Worker が権限外のファイルを変更しないよう境界を適用する。

コーディネーターは、仕様の採用判断や成果物の内容判断を行わない。
これらは Teacher または人の責務とする。

## 状態モデル

最低限、次の状態を持つ。

```text
WAITING_FOR_REQUEST
    ↓
TEACHER_PLANNING
    ↓
WAITING_FOR_WORKER
    ↓
WORKER_RUNNING
    ↓
WAITING_FOR_TEACHER_REVIEW
    ↓
COMPLETED
```

通常経路から、次の状態へ移れるようにする。

- `WAITING_FOR_HUMAN`: 人の判断または承認を待つ。
- `FAILED`: 続行できない失敗が発生した。
- `STOPPED`: 人または停止条件によって中止した。

Teacher のレビュー結果が次の Worker 作業を要求する場合は、作業 ID を更新した
`worker_task.md` を作成して `WAITING_FOR_WORKER` へ戻る。

## 最初の実装方針

最初からファイル監視による常駐処理や完全自動実行にはしない。
各工程を明示的に進める半自動のコマンドとして実装する。

```text
coordinator start <request>
coordinator run-worker
coordinator review
coordinator status
coordinator stop
```

各工程の終了後に、人が状態、作業指示、完了報告を確認してから次へ進める。
この運用で受け渡し形式、完了条件、停止条件が安定した後に、自動遷移する範囲を増やす。

## 実装方式の候補

### AI CLI を起動する方式

外部プログラムから Teacher 用と Worker 用の AI CLI を別プロセスで起動する。
既存の開発環境を利用しやすいが、入出力形式、終了判定、プロセス異常時の回復を管理
する必要がある。

### AI API を呼び出す方式

外部プログラムが API を使い、Teacher と Worker の状態を個別に管理する。
構造化出力、再試行、タイムアウトを制御しやすいが、認証、費用、API の状態管理が
必要になる。

どちらの方式でも、Teacher と Worker のコンテキストを混ぜず、文書による受け渡しを
正本とする。

## 近い既存構想

Teacher-Worker Coordinator に近い構想として、中央管理型のマルチエージェント、
Agent の handoff、状態を保存できるワークフロー、人の承認による中断と再開がある。

| 構想 | 近い点 | Teacher-Worker Coordinator との違い |
|---|---|---|
| Magentic-One | Orchestrator が計画、作業割当、進捗確認、再計画を行う | Orchestrator 自身が Teacher に近い判断も行う |
| LangGraph Supervisor | 中央 Supervisor が Agent の起動、通信、作業委譲を管理する | Supervisor と実行基盤の責務を分けることは必須ではない |
| LangGraph Runtime | 状態保存、中断、再開、失敗回復、人の承認を扱う | 役割設計ではなく、Coordinator の実装基盤に相当する |
| OpenAI Agents SDK | Manager、handoff、コードによる順序制御、人の承認を提供する | 会話または RunState を中心に受け渡す |
| AutoGen Group Chat | Manager が次に動く Agent と終了条件を管理する | 共通メッセージスレッドによる通信を中心とする |

### Magentic-One

Magentic-One は、Lead Orchestrator が Task Ledger と Progress Ledger を管理し、専門
Agent へ作業を割り当てる。進捗がなければ計画を更新し、作業を続行する。
計画、割当、進捗確認、再計画を中央で扱う点は、Teacher-Worker Coordinator に最も
近い。

一方、Magentic-One の Orchestrator は計画と評価も担う。
今回の構想では、計画、境界、評価は Teacher、起動、文書受け渡し、状態、停止は
Coordinator として責務を分ける。

参考:

- [Magentic-One: A Generalist Multi-Agent System for Solving Complex Tasks](https://www.microsoft.com/en-us/research/articles/magentic-one-a-generalist-multi-agent-system-for-solving-complex-tasks/)
- [Magentic-One paper](https://arxiv.org/abs/2411.04468)

### LangGraph

LangGraph の Supervisor パターンでは、中央 Supervisor が専門 Agent を調整し、通信と
作業委譲を管理する。LangGraph Runtime は、チェックポイントによる状態保存、中断、
再開、失敗回復、人の承認を提供する。

Teacher-Worker Coordinator を実装する場合、状態機械、永続化、human-in-the-loop の
実装基盤として利用できる。ただし、今回の構想では Supervisor に Teacher の判断と
Coordinator の実行制御をまとめず、別の責務として扱う。

参考:

- [LangGraph Multi-Agent Supervisor](https://github.com/langchain-ai/langgraph-supervisor-py)
- [LangGraph persistence](https://docs.langchain.com/oss/python/langgraph/persistence)
- [LangChain human-in-the-loop](https://docs.langchain.com/oss/python/langchain/human-in-the-loop)

### OpenAI Agents SDK

OpenAI Agents SDK は、中央 Manager が専門 Agent をツールとして呼ぶ方式、Agent 間で
制御を移す handoff、コードによるオーケストレーションを提供する。
RunState を保存し、人の承認を待って処理を再開することもできる。

今回の構想では、会話履歴または内部 RunState を受け渡しの正本にせず、
`worker_task.md` と `worker_report.md` に必要な情報を限定する。

参考:

- [OpenAI Agents SDK: Agent orchestration](https://openai.github.io/openai-agents-python/multi_agent/)
- [OpenAI Agents SDK: Handoffs](https://openai.github.io/openai-agents-python/handoffs/)
- [OpenAI Agents SDK: Human-in-the-loop](https://openai.github.io/openai-agents-python/human_in_the_loop/)

### AutoGen

AutoGen の Group Chat パターンでは、Group Chat Manager が次に発言する Agent を選び、
終了条件まで順番を管理する。Agent は共通トピックへメッセージを発行する。

今回の構想では、共通の会話スレッドを通信の正本にせず、作業 ID に対応した2つの文書
で Teacher と Worker の境界を明示する。

参考:

- [AutoGen: Group Chat](https://microsoft.github.io/autogen/stable/user-guide/core-user-guide/design-patterns/group-chat.html)
- [AutoGen: Message and Communication](https://microsoft.github.io/autogen/stable/user-guide/core-user-guide/framework/message-and-communication.html)

## 今回の構想の位置づけ

Teacher-Worker Coordinator は、完全に新しい種類のマルチエージェント構造と断定する
ものではない。既存の Supervisor 型、durable workflow、human-in-the-loop を、次の
運用原則で具体化する構想と位置づける。

- Teacher と Coordinator の責務を分ける。
- `worker_task.md` と `worker_report.md` を受け渡しの正本にする。
- Teacher の上位コンテキストや会話履歴全体を Worker へ渡さない。
- Coordinator に仕様や成果物の採用判断をさせない。
- 重要な判断では処理を止め、人へ返す。

## 安全上の原則

- 同じ AI の作業中に Teacher と Worker の役割を切り替えない。
- Worker は `worker_task.md` に記載された範囲だけを変更する。
- Teacher は成果物を直接修正しない。
- 重要な未決事項を AI の推測で確定しない。
- 仕様変更、対象外の変更、リスク受容は人の承認で停止する。
- 同じ失敗を繰り返した場合は自動再試行を止め、人へ返す。
- 完了条件を満たした証拠がない状態を完了にしない。

## 未決事項

- Teacher と Worker を起動する具体的な AI CLI または API
- `worker_task.md` と `worker_report.md` の正式なスキーマ
- 状態を保存するファイル形式
- 同時に実行できる作業数
- タイムアウトと再試行回数
- 人の承認を要求する条件
- 実行ログと完了済み受け渡しの保存期間

これらは実装前に決定し、Teacher-Worker の役割境界を変える場合は先に中心文書へ反映
する。
