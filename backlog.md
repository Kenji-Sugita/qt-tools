# backlog

このファイルには、次回すぐには着手しない長期の残作業、問題点、保留事項を書く。
現在の状態と次回の作業再開に直接必要な項目は `NEXT.md`、今後も有効な仕様、設計判断、不採用理由、運用制約は `DECISIONS.md` に書く。
完了履歴や検証ログの保管場所にはしない。

## 残作業

- 今回の依頼範囲ではなし。

## 問題点

- 構成変更前からある未解決Markdownリンク70件は今回の対象外として保持した。外部・隣接リポジトリーや未同梱資料への参照を含むため、追加修正時は実在する参照先と配布範囲を確認し、推測したファイルを追加しない。
- 2026-11-09の正本照合では、Sitesの配布ZIPが公開用より古い対象があった。LaserPointerのメニュー／トレイ対応、seminartimerの最前面切替・設定保存、cutterの `--cut` 系と同形式保存、qmlprofileranalyzerのconvert機能を旧ZIPで巻き戻していない。Sites側の配布版を改訂する場合は、対応する開発元と採用版を確認する。
- icppのQt Creatorガイドは公開用0.3.1を保持した。Sitesと配布ZIPの0.3.0を採用し直したものではない。clingの公開用 `build.sh` と配布ZIPの並列数差（8と2）も更新根拠がないため維持した。
- ワークフロー0.2.3の事例文書には非同梱の隣接デモリポジトリーへの参照14箇所（8種類）がある。正本に参照先の前提と観測事実が記載されており、同期時には正本を改変せず保持した。詳しくは `workflows/app-generation-workflow/NEXT.md`。

## 保留事項

- 2026-11-13追加のfileservermcpはQt 6.12.0/macOS arm64でビルド・CLI13項目まで確認。旧MCP教材であり、認証・TLS・Origin検証を備えた実用化や現行transport対応は未実装。warikanは静的Web版の構文・代表計算・集金ロジックまでで、今回の公開作業ではGUIを再検証していない。
- Qt Creator MCP 19.0.0手順の再実行、現行版の接続、別OS、練習帳の初心者による実践は未検証。Codexのヒアリング資料など未選定ファイルは収録待ちの自動追加対象としない。プロンプト集の権利者の確定、実案件での期待効果の測定も文書改訂と区別して扱う。

- Anker LayoutのGUI手動操作、Qt Cling／icppの対話例、macOS以外の環境は未検証。追加依頼があった場合は対象OS・Qt／Qt Cling／icppの版と確認項目を決める。PDF再生成・全ページ表示確認も、今回の配布物とのバイト照合とは分けて必要な対象を決める。
- qeyesのmacOS復帰・モーダルダイアログの実機操作は今回未検証。Whiteboardはoffscreenの自動テストまでで、別OS・実媒体障害・電源断・手動GUI操作の追加確認は `tools/whiteboard/backlog.md` を参照する。
- Sitesで既存の原稿・実装と一致したnow、tl、qmlprop、VuStereo、ObjectSelector、PropertyEditor、genpdf、ai-cmakeについて、PDFが未収録であるだけの追加はしていない。qmlprofileranalyzerの単体PDFも同様。PDFの生成・表示確認を追加する場合は、同期時のバイト照合とは分けて対象を決める。
