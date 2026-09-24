# DECISIONS

## 採用済み

- 2026-08-04 セミナーは、説明50分、Q/A 10分とする。説明本編は48分とし、スライド
  移動、間、時間調整に最大2分を確保する。アプリ生成ワークフロー章は14分とする。
- セミナー資料は、参加者向け案内文、説明詳細案、発表者用原稿、スライドを分ける。
  主催者への事前共有は `arch/seminar_slides.pdf` のみを基本とする。
- セミナー後の参加者向け配布では、`arch/seminar_presenter_notes.txt` をそのまま配布
  せず、スライドごとの要点、図表の読み方、補足説明へ再構成したスライド説明書を
  `arch/seminar_slides_guide.md` / `.pdf` に作成し、`arch/seminar_slides.pdf` と一緒に
  配布する。スライド説明書は参加者向けのです・ます調とする。
- Teacher は Worker より上位の作業文脈を管理し、Planner、Navigator、Guardian、
  Reviewer、Mentor の機能を状況に応じて統合する立場として扱う。
- Teacher と Worker には同じ種類の AI を使用できるが、一つの AI の作業中に役割を
  切り替えず、ワークフロー作成・保守用と成果物作成用の異なるコンテキストで動かす。
- Consultant AI は Teacher-Worker の恒常的な第三役割にせず、現在の作業文脈へ直接
  含めたくない一時的な調査、事実確認、専門的助言に使用する。回答は人へ返し、必要な
  内容だけを本作業へ反映する。
- Teacher-Worker の作業は、人が Teacher へ目的、背景、期待する成果、制約、対象外を
  伝えるところから始める。Teacher が目的を自分で決める運用にはしない。
- Teacher から Worker への作業指示は `worker_task.md`、Worker から Teacher への完了
  報告は `worker_report.md` で受け渡す。Teacher の上位コンテキストや会話履歴全体は
  Worker へ渡さない。
- Teacher は Worker へ渡す前に不足、矛盾、未決事項を整理し、人の判断が必要な事項を
  人へ確認する。Worker の報告後も、Teacher が完了を判断できない重要事項は人へ返す。
- Teacher-Worker の役割分離と文書受け渡しを実行可能にする構想を
  `Teacher-Worker Coordination`、それを実行する外部ツールを
  `Teacher-Worker Coordinator` と呼ぶ。外部ツールの責務は計画や評価ではなく、AIの
  起動、文書の受け渡し、状態、停止の管理であるため、役割を過大に見せない
  `Coordination` / `Coordinator` を使用する。
- Teacher-Worker Coordinator は、AI の起動、`worker_task.md` と
  `worker_report.md` の受け渡し、状態、タイムアウト、失敗、停止を管理する。計画、
  境界、評価は Teacher、仕様の採用や重要事項の最終判断は人に残す。
- Teacher-Worker Coordinator は最初から完全自動化せず、工程ごとに人が承認する半自
  動運用から始める。
- Teacher-Worker Coordinator は、既存の Supervisor 型、durable workflow、
  human-in-the-loop を、文書正本と Teacher・Coordinator の責務分離で具体化する構想
  と位置づけ、完全に新しい種類の構造とは断定しない。
- Loop engineering は、AIを使う作業について目標、検証方法、停止条件、状態の引き
  継ぎを定め、実行、検証、修正を完了まで反復できるループを設計する考え方として扱う。
  セミナーでは、今回の作業を完了させる内側の実行ループと、繰り返す問題を次回の
  ハーネス改善へ戻す外側の改善ループに分けて説明する。
- アプリ生成ワークフローには前工程へ戻るループの土台があるが、開始条件、停止条件、
  状態管理、自動反復までを明示した Loop engineering の仕組み化は途中と位置づける。
  最初は手動でループ定義を運用し、安定した部分だけを自動化する。仕様の採用と重要な
  業務判断は人に残す。
- Loop engineering化構想と Teacher-Worker Coordinator構想は、確定済みのセミナー
  本編へ組み込まず、Q/A用および後日配布用の独立した1ページ補足資料とする。後日配布
  では、本編、スライド説明書、Loop engineering化構想、Teacher-Worker Coordinator
  構想の4 PDFを結合せず、別ファイルとして配布する。
- 現在のアプリ生成ワークフローは、利用者の操作、画面、業務ロジックをUC仕様書、
  UI仕様書、ビジネスレイヤー仕様書へ分けられるUIアプリを主な適用対象とする。すべて
  のソフトウェア開発へ一律に適用しない。
- qtcling、icpp、genpdf のように、言語処理、実行環境、変換処理、CLI、技術的制約、
  小さな実験と検証が中心となる技術基盤には、現在の三点セット中心のワークフローを
  適用しない。必要な場合は、対象の性質に合う別の軽量ワークフローを検討する。
- 重要ロジックを別仕様へ括り出す場合、仕様化前ノートには詳細仕様を重複記載せず、
  対象、理由、元のビジネスレイヤー仕様、ヘッダー定義とVDM-SLの保存先、後続文書への
  反映先を追跡情報として残す。ヘッダー定義とVDM-SLは人がレビューしてから実装する。
- 重要ロジックの別仕様化とTeacher-Workerを組み合わせる運用案では、同じ種類のAIを
  別コンテキストで起動したWorkerが重要ロジックを実装する。アプリ本体のWorkerには
  公開ヘッダーから利用させ、重要ロジックの内部を変更させない。これは現行の手順と
  して実装済みではなく、正式化を今後検討する運用案と位置づける。

## 不採用

- `Teacher-Worker Orchestration` / `Teacher-Worker Orchestrator` は使用しない。
  外部ツールが計画や評価まで統括する印象があり、Teacher の責務と混同しやすいため。
- `Teacher-Worker Conductor` は使用しない。指揮や計画まで担う印象が Teacher と重なり、
  既存のワークフロー製品やツールとも名称が衝突しやすいため。

## まだ決めていないこと

- Teacher と Worker を起動する AI CLI または API
- `worker_task.md` と `worker_report.md` の正式なスキーマ
- Coordinator の状態を保存するファイル形式
- 同時に実行できる作業数
- タイムアウトと再試行回数
- 人の承認を要求する条件
- 実行ログと完了済み受け渡しの保存期間

## 判断基準

- セミナー直前の修正では、説明を増やすことより、50分以内に収めてQ/Aを10分確保する
  ことを優先する。
- 主催者へのドラフト共有後は、指摘への対応または明確な不整合の修正を除き、大きな
  構成変更を避ける。

## 継続する仕様・運用制約

- `arch/seminar.txt` は参加者向け案内文の正本とし、説明原稿や詳細資料を混ぜない。
- `arch/qt_reply_note.txt` の個人的な締め・返答は、スライドや説明原稿へ追加しない。
- `arch/seminar_slides.pdf` は `python3 arch/generate_seminar_slides.py` で生成する。
  `genpdf` が作る余分な空白ページを除き、表紙を含む通しページ番号へ補正する。
- `arch/seminar_slides_guide.md` をスライド説明書の正本とし、配布用PDFは
  `genpdf arch/seminar_slides_guide.md --output arch/seminar_slides_guide.pdf` で生成する。
- `worker_task.md` と `worker_report.md` へ複数の作業を無制限に追記しない。履歴が必要
  な場合は、完了した受け渡しを作業 ID 別に保存し、古い指示と現在の指示を混在させな
  い。
- `NEXT.md` は作業再開の入口として使い、個々の `worker_task.md` と
  `worker_report.md` の代わりにはしない。
- Teacher-Worker Coordinator の構想資料と補足スライドは `docs/` に置き、2026-08-04
  セミナーの確定に近い本編 `arch/` へは、構想が詰まるまで追加しない。
