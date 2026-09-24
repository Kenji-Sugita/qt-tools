# NEXT

このファイルを読む、または更新する前に、`~/AGENTS.next.md` を読み、その指示に従うこと。
`~/AGENTS.next.md` が存在しない場合は、このファイルを整理・削除せず、利用者に確認すること。
このルールは、ユーザーから明示的な指示がない限り変更しない。

## 次にやること

- 外部ツールの責務を正確かつ控えめに表すため、構想名を
  `Teacher-Worker Coordination`、ツール名を `Teacher-Worker Coordinator` へ変更した。
  中心文書、補足スライド、生成スクリプト、管理文書、ファイル名へ反映済み。
  補足PDFは再生成し、A4横1ページの表示を確認済み。
- 2026-08-03 に、`arch/seminar_presenter_notes.epub` を使って最終リハーサルを行い、
  説明本編48分と移動・間の最大2分を合わせて50分以内に収め、Q/Aを10分確保できる
  ことを確認する。
- セミナー後にスライドと一緒に配布する参加者向け説明書を
  `arch/seminar_slides_guide.md` / `.pdf` に作成した。発表者用の時間管理や話し方は含めず、
  `arch/seminar_slides.pdf` の全34ページに対応する要点と補足説明を、参加者向けの
  です・ます調で記載している。PDFはA4縦10ページで、全ページの表示を確認済み。
- 主催者へ共有したドラフトから大きな構成変更は行わず、Loop engineering 修正後の
  スライド9ページ、原稿のページ参照、Q/A開始時刻を最終確認する。
- `arch/seminar_slides.pdf` は Loop engineering 修正後も全34ページで、スライド9ページ
  の内側の実行ループと外側の改善ループに文字切れや重なりがないことを確認済み。
- 現在の `genpdf` 出力には以前の余分な2ページ目がないため、
  `python3 arch/generate_seminar_slides.py` は空白ページ確認で停止する。今回は
  `genpdf arch/seminar_slides.md --output arch/seminar_slides.pdf` で生成済み。恒久対応は
  `backlog.md` を参照する。
- Loop engineering は、AIを使う作業について目標、検証方法、停止条件、状態の引き
  継ぎを定め、実行、検証、修正を完了まで反復できるようにする設計として整理した。
  セミナーでは内側の実行ループと外側のハーネス改善ループに分けて説明する。
- アプリ生成ワークフローにはレビュー結果から前工程へ戻る土台があるが、開始条件、
  停止条件、状態管理、自動反復までを明示した仕組みではないことを原稿へ明記した。
- 発表者原稿のアプリ生成ワークフロー章を12分から14分へ増やした。重要ロジックを別
  仕様へ括り出す判断と追跡情報を仕様化前ノートへ戻し、ヘッダー定義とVDM-SLを人が
  レビューした後、同じ種類のAIを別コンテキストで起動したWorkerが実装する運用案を
  追加した。アプリ本体のWorkerは公開ヘッダーから利用し、内部を変更しない。
- Teacher がレビューと成果物修正を兼ねると目の前の完成を優先しやすいため、Teacher
  は問題、戻り先、制約、判断理由を示し、実際の修正はWorkerへ返す説明に整理した。
- `arch/seminar_presenter_notes.epub` は上記修正後に再生成済みで、章見出し、重要
  ロジックの説明、Teacherの説明が反映され、EPUBアーカイブに破損がないことを確認済み。
- ホワイトボードへの日常的な機能追加でアプリ生成ワークフローを継続利用しており、
  ワークフローなしの場合より意図違いと確認漏れが減り、修正時の戻り先が明確になって
  作業が楽になるという定性的な手応えを得ている。定量評価はまだ行っていない。
- qtcling、icpp、genpdf には現在の三点セット中心のアプリ生成ワークフローを適用して
  いない。UIアプリと技術基盤では適する作業構造が異なるため、一律適用しない。
- Loop engineering 化構想の1ページ補足資料を
  `docs/app_generation_loop_engineering_concept.md` / `.pdf` に作成済み。セミナー本編へは
  追加せず、Q/Aで関連質問があった場合に使い、後日配布では本編、スライド説明書、
  Loop engineering化構想、Teacher-Worker Coordinator構想の4 PDFを別ファイルで配布
  する。
- Teacher-Worker の説明は、人が目的、背景、制約を Teacher へ伝えるところから始まり、
  `worker_task.md` で Worker へ作業を渡し、`worker_report.md` で結果を返す具体的な流れ
  へ更新済み。中心文書、発表者原稿、スライド23ページへ反映した。
- Teacher-Worker 更新後の `arch/seminar_slides.pdf` は再生成済み。表紙を含む34ページ、
  通しページ番号、原稿のページ参照、スライド23ページの図の表示を確認済み。
- Teacher-Worker の次段階として、外部ツールが AI の起動、2文書の受け渡し、状態、停
  止を管理する Teacher-Worker Coordinator の構想を
  `docs/teacher_worker_coordination.md` に保存済み。
- Teacher-Worker Coordinator の1ページ補足スライドを
  `docs/teacher_worker_coordinator_concept.md` / `.pdf` に作成済み。Magentic-One、
  LangGraph、OpenAI Agents SDKとの簡潔な比較を含み、A4横1ページの表示を確認済み。
- 2026-08-04 実施のセミナー資料ドラフトは `arch/seminar_presenter_notes.txt`、`arch/seminar_slides.md`、`arch/seminar_slides.pdf` に作成済み。
- セミナー資料は、スライドと発表者原稿のページ対応、表記、AI コンテキスト劣化の説明を確認済み。
- セミナー資料には、実装設計契約、アプリ設計、設計書レビューを含む本来のフローと、既存工程への部分適用を追加済み。
- 発表者原稿には、`アプリ生成ワークフロー` の「アプリ」は主に UI を持つアプリケーションを指し、UI のないライブラリや共通ロジックはヘッダー定義や VDM-SL など別手法を併用する補足を追加済み。
- 2026-08-04 実施の `arch/` セミナー資料は、説明50分、Q/A 10分の構成。説明本編48分
  に、移動、間、時間調整の最大2分を加え、`arch/seminar_slides.pdf` は表紙を含む34ページ。
- `arch/` セミナー資料は、詳細案、発表者原稿、スライドの内容とページ対応、および PDF の全34ページの表示を確認済み。`arch/seminar_slides.pdf` は主催者へ「確認用ドラフト」として共有済み。
- USER_GUIDE レビュー報告を反映し、`app-generation-workflow/USER_GUIDE.md` に「役割の読み方」、工程ごとの `人主体` / `AI主体` / `共同`、各章冒頭の `主体:` と `出力:` を追加済み。バージョンは `0.2.2`。
- Teacher-Worker engineering は、Teacher が Worker より上位の作業文脈を管理し、Planner、Navigator、Guardian、Reviewer、Mentor の機能を統合する一般化としてスライドと発表者原稿へ反映済み。
- Teacher と Worker には基本的に同じ種類の AI を使うが、一つの AI の中で役割を切り替えず、ワークフロー作成・保守用と成果物作成用の異なるコンテキストを持つ別の AI として扱う。
- 発表者原稿の想定質問は14問。既存コードでは、逆生成三点セットを AI、人の順でレビューし、逆生成レビュー用仕様化前ノートも AI、人の順でレビューしてから、正式な三点セットを作成する流れを記載済み。
- 主催者への確認用ドラフト共有は完了。次の節目は `arch/seminar_share_schedule.txt` の方針どおり、2026-07-30 のほぼ最終版確認。
- セミナー案内文 `arch/seminar.txt` は参加者向け告知文として維持し、説明原稿や詳細資料を混ぜない。
- 各プロジェクトで複数回実施する「アプリ生成ワークフロー」セミナーの共通資料を `seminars/app-generation-workflow/` に作成済み。60分を説明15分、要求からコード生成・ビルド・実行までのデモ30分、Q/A15分で構成する。
- 新セミナーを実施する前に、`seminar_presenter_notes.txt` のプロジェクト名、開催日時、デモ題材、作業ディレクトリ、構成・ビルド・実行コマンド、進行スライド表示方法を差し替え、`seminar_plan.md` に従って工程ごとの復旧地点を用意する。
- 新セミナーのスライドは、60分の進行、デモ7工程、デモ後の整理が分かる22ページ構成。デモ7工程には現在位置、確認内容、次へ進む条件を表示する。仕様化前ノートと三点セットは、AIレビュー後に人が確認してから次工程へ進む。想定質問は12問。
- ワークフロー本体は `../app-generation-workflow/app-generation-workflow/` として扱う。
- この管理メモは `../app-generation-workflow/NEXT.md` に置く。
- 必要なら `../app-generation-workflow/` をワークフロー配布用の作業ディレクトリとして git 管理する。
- `app-generation-workflow/` ディレクトリを含む形で `app-generation-workflow.zip` を作成済み。
- 逆生成レビュー用テンプレート `app-generation-workflow/templates/source_memo_reverse_review.md` を追加済み。
- 要求定義テンプレート `app-generation-workflow/templates/requirement.txt` を追加済み。
- 仕様化前ノート前に AI が要求定義レビューを行う流れを追加済み。
- `USER_GUIDE.md` の genpdf フロントマターに `version: 0.1.0` と作成日付 `date: 2026-05-07` を追加済み。
- AI 指示プロンプト例 `docs/prompt_examples.md` を追加済み。
- 社内説明用資料 `docs/internal_demo_request.md`、`docs/internal_demo_key_points.md`、`docs/ai_usage_guidance.md` を追加済み。
- Qt 開発への AI 適用を説明するスライド `docs/qt_ai_workflow_slides.md` / `.pdf` を追加済み。
- スライドには、UIありは三点セット、UIなしはヘッダー定義や VDM-SL など別手法を併用する方針を追加済み。
- テスト説明用スライド `docs/qt_ai_test_workflow_slides.md` / `.pdf` を追加済み。
- GitHub Copilot 補完活用ガイド `docs/copilot_completion_guide.md` / `.pdf` を追加済み。
- セミナー後の長期作業は `backlog.md` を参照する。
- 次回以降の git commit メッセージは日本語にする。

## 背景

- `app-generation-workflow/app-generation-workflow/` は、stretch-log アプリ作成時に整理した汎用アプリ生成ワークフロー。
- 仕様化前ノート、三点セット、実装プロンプト、Qt/CMake 実装までの流れを、アプリ依存性をなくして再利用できる形にしたもの。
- 汎用ワークフローは、`app-generation-workflow/three_key_documents_workflow.md` を読ませるだけで、何も知らない Codex CLI が仕様化前ノートと三点セットの役割を理解できることを狙っている。

## 構成

- `NEXT.md`
- `workflow_applications.md`
- `docs/workflow_improvements.md`
- `docs/post_generation_workflow.md`
- `docs/prompt_examples.md`
- `docs/internal_demo_request.md`
- `docs/internal_demo_key_points.md`
- `docs/ai_usage_guidance.md`
- `docs/qt_ai_workflow_slides.md`
- `docs/qt_ai_workflow_slides.pdf`
- `docs/qt_ai_test_workflow_slides.md`
- `docs/qt_ai_test_workflow_slides.pdf`
- `docs/copilot_completion_guide.md`
- `docs/copilot_completion_guide.pdf`
- `docs/workflow_evolution_summary.md`
- `docs/teacher_worker_loop_engineering.md`
- `docs/related_concepts_for_teacher_worker_workflow.md`
- `docs/teacher_worker_coordination.md`
- `docs/teacher_worker_coordinator_concept.md`
- `docs/teacher_worker_coordinator_concept.pdf`
- `docs/generate_teacher_worker_coordinator_concept.py`
- `docs/app_generation_loop_engineering_concept.md`
- `docs/app_generation_loop_engineering_concept.pdf`
- `docs/generate_app_generation_loop_engineering_concept.py`
- `arch/seminar.txt`
- `arch/seminar_detail.txt`
- `arch/seminar_presenter_notes.txt`
- `arch/seminar_presenter_notes.epub`
- `arch/seminar_slides.md`
- `arch/seminar_slides.pdf`
- `arch/seminar_slides_guide.md`
- `arch/seminar_slides_guide.pdf`
- `arch/seminar_share_schedule.txt`
- `arch/qt_reply_note.txt`
- `seminars/app-generation-workflow/seminar.txt`
- `seminars/app-generation-workflow/seminar_detail.txt`
- `seminars/app-generation-workflow/seminar_plan.md`
- `seminars/app-generation-workflow/seminar_presenter_notes.txt`
- `seminars/app-generation-workflow/seminar_slides.md`
- `seminars/app-generation-workflow/seminar_slides.pdf`
- `app-generation-workflow/README.md`
- `app-generation-workflow/USER_GUIDE.md`
- `app-generation-workflow/three_key_documents_workflow.md`
- `app-generation-workflow/templates/requirement.txt`
- `app-generation-workflow/templates/source_memo.md`
- `app-generation-workflow/templates/01_usecase_spec.md`
- `app-generation-workflow/templates/02_ui_spec.md`
- `app-generation-workflow/templates/03_business_spec.md`
- `app-generation-workflow/templates/implementation_constraints.md`
- `app-generation-workflow/templates/implementation_prompt.md`

## 注意

- `app-generation-workflow/USER_GUIDE.md` を正本とする。
- `app-generation-workflow/USER_GUIDE.md` は UTF-8 BOM 付き。必要なら `xxd -g 1 -l 8 app-generation-workflow/USER_GUIDE.md` で `ef bb bf` を確認する。
- `USER_GUIDE.md` のファイル名は `USER_GUID.md` ではなく `USER_GUIDE.md`。
- `USER_GUIDE.md` の genpdf フロントマターには `version` と作成日付としての `date` を記載する。
- バージョンは `0.1.0` から開始し、配布リリースごとに変更内容に応じて更新する。
- `app-generation-workflow/USER_GUIDE.md` を修正したら、`app-generation-workflow/` 側で `USER_GUIDE.pdf` を再生成してから `app-generation-workflow.zip` を更新する。
- `arch/seminar.txt` はセミナー案内文の正本。説明を詰めた内容は `arch/seminar_detail.txt`、当日横の PC で見る話し言葉の原稿は `arch/seminar_presenter_notes.txt` に分ける。
- `arch/qt_reply_note.txt` は個人的な締め・返答メモであり、スライドや説明原稿には入れない。
- `arch/seminar_slides.pdf` は `python3 arch/generate_seminar_slides.py` で生成する。内部で `genpdf` を使い、余分な空白ページを除いて通しページ番号を補正する。PDF 1ページ目は表紙、本文スライドは2ページ目から始まる。
- `arch/seminar_slides_guide.pdf` は
  `genpdf arch/seminar_slides_guide.md --output arch/seminar_slides_guide.pdf` で生成する。
- `docs/teacher_worker_coordinator_concept.pdf` は
  `python3 docs/generate_teacher_worker_coordinator_concept.py` で生成する。自動表紙を
  除き、A4横1ページの補足スライドにする。
- `docs/app_generation_loop_engineering_concept.pdf` は
  `python3 docs/generate_app_generation_loop_engineering_concept.py` で生成する。自動表紙
  を除き、A4横1ページの補足スライドにする。
- `seminars/app-generation-workflow/` は各プロジェクトで再利用するセミナー共通資料。プロジェクト固有のデモ成果物と復旧地点は、この共通資料へ直接混ぜない。
- 新セミナーの PDF は `genpdf seminars/app-generation-workflow/seminar_slides.md --output seminars/app-generation-workflow/seminar_slides.pdf` で生成する。
- `NEXT.md` はワークフロー本体には含めず、`../app-generation-workflow/` 直下の引き継ぎ用メモとして扱う。
- 配布 zip には `NEXT.md` と `request.txt` を含めない。
- 配布 zip には `docs/` ディレクトリ全体を含める。
- `app-generation-workflow.zip` は配布用生成物として扱い、git commit には含めない。
- `genpdf` の slides で日本語を含むフロー図を作る場合、Mermaid は矩形内テキストの右端が欠けることがある。発表用スライドでは Graphviz dot 図を優先する。
- 逆生成レビュー用テンプレートは配布 zip に含める。
- 要求定義テンプレート `templates/requirement.txt` は配布 zip に含める。
- 共通実装制約テンプレート `templates/implementation_constraints.md` は配布 zip に含める。
- `implementation_constraints.md` は対象アプリ用ディレクトリ直下に置く任意ファイルとし、三点セットや仕様化前ノートへ混ぜず、実装プロンプト作成時に参照する。
- 実装プロンプトでは、実装時の作業ディレクトリを対象アプリ用ディレクトリとして明記する。
- 実装後は実装結果レビューを行い、重大な不整合、仕様への反映漏れ、未解決の仮決めがある場合は先に仕様へ戻して整合させる。
- 実装プロンプトでは、標準ではテストコード生成を指示しない。コード生成後に自動テストを追加できる構造にすることを指示する。
- テストコードと手動確認項目は、コード生成後に、三点セット、実装プロンプト、実装コードを参照して作成する。コードだけから期待値を作らない。
- `implementation_design_contract.md` と `app_design.md` は対象アプリ用ディレクトリ配下に置く後工程文書とし、未解決の重大な不整合がある場合は完成扱いにしない。
- ユーザーの作業場所にコピー済みのワークフローだけを参照し、他の場所にあるワークフローは参照しない。
- コピー済みワークフロー本体はテンプレートとして扱い、対象アプリ固有の内容を書き込まない。
- 5分割仕様化前ノートのディレクトリ名は `five_source_memos/` に統一済み。
- 最初から規模が大きい場合は `source_memo.md` を作らず、最初から `five_source_memos/` を作る。
- 途中で大きくなった場合は `source_memo.md` から `five_source_memos/` へ移行する。
- 5分割へ移行後は、ワークフロー上の正本を `source_memo.md` ではなく `five_source_memos/` 配下の分割ノート群とする。
- 5分割移行後の修正、三点セット作成、実装プロンプト更新では `source_memo.md` ではなく `five_source_memos/` を参照する。

## ワークフローの流れ

- 仕様化前ノートと三点セットの理解
- 任意形式の要求 `.txt`
- 対象アプリ用ディレクトリ作成
- 要求 `.txt` から仕様化前ノート作成
- 三点セット
- 必要なら共通実装制約
- 実装プロンプト
- アプリ実装
- コード生成後のテスト作成・確認
- 実装結果レビュー
- 必要なら仕様への反映
- 実装設計契約
- アプリ設計書
- アプリ設計書レビュー
- 仕様への反映

## 意図違いの修正フロー

- 意図違いの指摘
- 仕様化前ノート修正
- 三点セット修正
- 実装プロンプト修正
- コード修正
- 仕様への反映確認

## 意図違い時の禁止事項

- 指摘直後にコードだけを修正しない。
- `source_memo.md` を更新せず、三点セットだけを修正しない。
- 実装プロンプトを更新せず、コードだけを修正しない。
- 三点セットにない仕様をコードへ直接追加しない。
- 仮決めを仕様へ戻さず、コードだけに残さない。

## ヒアリング9項目

- 作りたいアプリの目的
- 利用者
- アプリ種別
- やりたいこと
- 入力する情報
- 表示したい情報
- 守るべきルール
- 作らない機能
- まだ決めていないこと

## レビュー後の修正済み事項

- `記録項目` という stretch-log 固有表現を汎用表現へ変更。
- ビジネスレイヤー仕様テンプレートを `BL-001` など処理別に書ける形へ変更。
- 実装プロンプトの `保存条件` を `保存・永続化条件` に変更し、保存不要アプリにも対応。
- 実装結果が意図と違う場合、まず `source_memo.md` を修正するようにワークフローと README へ追記。
- `USER_GUID.md` のタイポを `USER_GUIDE.md` に修正。
- `USER_GUIDE.md` に5分割仕様化前ノートの説明を追加。
- 対象アプリ固有の内容をテンプレートへ書かない指示を明確化。
- 仕様化前ノートの未決事項に「決定後の反映先」を1行で付ける運用を追加。
- 三点セットテンプレートにアプリ種別別の確認観点、状態・処理、任意パターンを追加。
- 任意形式の要求 `.txt` を起点にする流れへ更新。
- 最初に対象アプリ用ディレクトリを作成し、仕様化前ノート、三点セット、実装プロンプト、ソースコードをそこへ置く運用を追加。
- 他の場所にあるワークフローを参照せず、ユーザー作業場所のコピー済みワークフローだけを参照する運用を追加。
- コピー済みワークフロー本体を変更しない注意を追加。
- 未決事項を `- 未決事項: ...。決定後の反映先: ...` の1行形式へ統一。
- 未決事項を「なしにする」場合は `- なし`、なしにしない場合は AI が推薦案を提示する流れを追加。
- `templates/requirement.txt` を追加し、未決事項を減らすための推奨入力形式を用意。
- 仕様化前ノートを作る前に、AI が要求定義レビューで不足、確認質問、追記候補、推測箇所を提示する運用を追加。
- 要求定義レビューで実装や三点セット生成に影響する不足がある場合は、先へ進まず `requirement.txt` を修正する運用を追加。
- `USER_GUIDE.md` に `version: 0.1.0` と作成日付 `date: 2026-05-07` を追加し、PDF 表紙へバージョン表記を出す運用を開始。
- `implementation_prompt.md` が使える状態でない場合に実装へ進まず警告する運用を追加。
- `app-generation-workflow.zip` を作成し、`unzip -l` で `NEXT.md` と `request.txt` が含まれないことを確認。
- 三点セットを中間表現として使う応用例を `workflow_applications.md` に保存。
- 逆生成レビュー用テンプレートを追加し、README、USER_GUIDE、手順書へ導線を追加。
- 逆生成レビュー用テンプレートでは、現行実装から読み取れる挙動、仕様として採用する挙動、バグ候補・要確認事項、境界値確認を分けて扱う。
- AI 生成後のレビュー必須箇所を USER_GUIDE と手順書に明記。
- AI 指示プロンプト例を `docs/prompt_examples.md` に分離して作成。
- 社内デモや発表で見てもらうための依頼文、説明要点、AI 活用方針を `docs/` に保存。
- Qt 開発への AI 適用、三点セット、仕様化前ノート、5分割仕様化前ノート、分割コード生成を説明するスライドを `docs/qt_ai_workflow_slides.md` と `docs/qt_ai_workflow_slides.pdf` に作成。
- スライド内のフロー図は Mermaid から Graphviz dot へ変更し、日本語ラベルの右端欠けを回避。
- スライドに、UIなしライブラリーはヘッダー定義、VDM-SL、事前条件、事後条件、不変条件など別の仕様化手法を併用する説明を追加。
- スライドに、AI の介入ポイント、人が判断すること、やってはいけないこと、この方法で良くなることを追加。
- テスト説明用スライドを `docs/qt_ai_test_workflow_slides.md` と `docs/qt_ai_test_workflow_slides.pdf` に作成。
- テスト説明用スライドには、三点セットによるテスト分類、自動テストと手動テストの分離、AI のテスト介入ポイント、UIあり/なし、大規模分割単位ごとのテストを追加。
- GitHub Copilot 補完活用ガイドを `docs/copilot_completion_guide.md` と `docs/copilot_completion_guide.pdf` に作成。
- Copilot 補完活用ガイドでは、レビューへの回答、AI が生成しない部分、定型 UI 実装、ビジネスルール、テスト、リファクタリング、ドキュメント作成で補完を使う範囲を整理。
- `USER_GUIDE.pdf` を再生成。
- `app-generation-workflow.zip` を再作成し、`source_memo_reverse_review.md`、`templates/requirement.txt`、`docs/` 全体が含まれることを確認。
- 配布 zip を Git 追跡から外し、`.gitignore` に追加。
- インタラクション詳細確認追加後のテンプレート末尾スペースを修正し、`USER_GUIDE.pdf` を Markdown と同期。
- 共通実装制約テンプレート `templates/implementation_constraints.md` を追加し、ビルド互換性、コード生成方針、移植性などを分離できるようにした。
- `implementation_prompt.md` に、実装時の作業ディレクトリを対象アプリ用ディレクトリとする指示と、`implementation_constraints.md` 参照を追加。
- `source_memo.md`、`USER_GUIDE.md`、`three_key_documents_workflow.md`、`README.md`、`docs/prompt_examples.md` に、実装制約を仕様化前ノートや三点セットへ混ぜない運用を追記。
- `USER_GUIDE.pdf` と `app-generation-workflow.zip` を再生成し、zip に `templates/implementation_constraints.md` が含まれ、`NEXT.md` と `request.txt` が含まれないことを確認。
- `../whiteboard` 側で共通実装制約を取り込んだスクラッチ検証が通った。
- `../app-generation-workflow-demo2` のスクラッチ検証結果を反映し、実装結果レビュー、実装設計契約 `implementation_design_contract.md`、アプリ設計書 `app_design.md`、アプリ設計書レビューをワークフローへ追加。
- 実装結果レビューで重大な不整合、仕様への反映漏れ、未解決の仮決めがある場合は、実装設計契約やアプリ設計書を作らず、先に仕様へ戻す運用を追加。
- `USER_GUIDE.md` のバージョンを `0.2.0` に更新し、`USER_GUIDE.pdf` と `app-generation-workflow.zip` を再生成。
- ルート直下の `USER_GUIDE.md` / `USER_GUIDE.pdf` は削除し、`app-generation-workflow/USER_GUIDE.md` を正本に戻した。
- 正本 `app-generation-workflow/USER_GUIDE.md` の作成日付を `2026-06-23` に更新し、`USER_GUIDE.pdf`、`app-generation-workflow.zip` へ反映済み。
- コード生成後の作業手順を `docs/post_generation_workflow.md` として追加し、生成コード確認、仕様照合、ビルド、自動テスト、手動確認、実装結果レビュー、仕様への戻し、設計文書化、運用判断、再開メモ、コミット前確認を整理。
- `app-generation-workflow/USER_GUIDE.md` に `docs/post_generation_workflow.md` への導線と、コード生成後手順を他章と同じ粒度で説明する章を追加。
- `app-generation-workflow/README.md` に、コード生成後の確認、設計文書化、運用判断の文書として `docs/post_generation_workflow.md` を追記。
- `USER_GUIDE.pdf` と `app-generation-workflow.zip` を再生成し、zip に `docs/post_generation_workflow.md` が含まれ、`NEXT.md` と `request.txt` とルート直下 `USER_GUIDE.md` が含まれないことを確認。
- コード生成後にテストを作成する運用へ変更。`templates/implementation_prompt.md` から実装時のテストコード生成指示を外し、テスト可能性の指示に変更。
- `docs/post_generation_workflow.md`、`docs/prompt_examples.md`、`app-generation-workflow/USER_GUIDE.md`、`three_key_documents_workflow.md` に、三点セット、実装プロンプト、実装コードを参照して自動テストと手動確認項目を作成する手順を追加。
- テスト工程追加後のレビューを反映し、正本 `three_key_documents_workflow.md` と `docs/prompt_examples.md` の順序を「コード生成後のテスト作成・確認」から「実装結果レビュー」へ統一。`docs/post_generation_workflow.md` の開始前提から `tests/` 必須に見える記述を外した。
- これまでのワークフロー改良内容を `docs/workflow_evolution_summary.md` に保存。
- ティーチャー・実務 AI 分離ワークフローを、ループエンジニアリングを安定して回す上位構造として `docs/teacher_worker_loop_engineering.md` に整理。
- ティーチャー・実務 AI 分離ワークフローに近い既存概念を調査し、ループエンジニアリング、マルチエージェント、Actor-Critic、Reflexion、監督レベル設計との関係を `docs/related_concepts_for_teacher_worker_workflow.md` に保存。
- `docs/teacher_worker_loop_engineering.md` に、ティーチャー AI の型としてガーディアン型、指導型、レビュー型、分類型、調査型、戦略型、移植型、品質保証型、メモリ型を追加し、高次文脈管理ロール群として一般化。
- 会議議題として、Prompt engineering、Context engineering、Harness engineering、Loop engineering、仕様化の目的、Teacher-Worker engineering の流れで説明する構成を整理。
- セミナースライドに「仕様不在による AI コンテキスト劣化」を追加し、Context Rot、Requirement Ambiguity、Tacit Knowledge Loss、Spec-Driven Development を近い研究・概念として追記。`arch/seminar_presenter_notes.txt` のPDFページ対応と説明も更新。
- スライドと発表者原稿の表記は、`UC仕様書`、`UI仕様書`、`ビジネスレイヤー仕様書` に統一済み。
- セミナースライドの末尾に参考スライドと参考リンクを追加。参考リンクは出典を確認し、`Loop engineering` は `Engineering the Loops`、`Tacit Knowledge Loss` は `Knowledge Lever Risk Management`、`Supervisor / Worker` は `Autonoma` を参照する形へ変更。VDM は読みやすさ重視で `Vienna Development Method` を維持。
- genpdf の `slide-subtitle` を使い、セミナースライドを「導入」「AI活用の進化」「仕様化」「アプリ生成ワークフロー」「Teacher-Worker engineering」「まとめ」「参考」「参考リンク」「Q/A」の主タイトルと、ページごとのサブタイトルに再構成。サブタイトルの番号は削除済み。
- 現在までのワークフロー改良を、セミナー説明用メモとして `arch/workflow_improvement_memo.md` に保存済み。

## 未完了

- セミナー後に着手する長期の残作業は `backlog.md` を参照する。

## 触るファイル

- `arch/seminar_presenter_notes.txt`
- `arch/seminar_slides.md`
- 修正した場合は `arch/seminar_slides.pdf`
- `arch/seminar_slides_guide.md`
- 修正した場合は `arch/seminar_slides_guide.pdf`
- Teacher-Worker の運用説明を修正する場合は `docs/teacher_worker_loop_engineering.md`
- Teacher-Worker Coordinator の構想を修正する場合は
  `docs/teacher_worker_coordination.md`、`docs/teacher_worker_coordinator_concept.md`、
  `docs/generate_teacher_worker_coordinator_concept.py`、生成後の `.pdf`
