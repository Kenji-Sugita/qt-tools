# backlog

このファイルには、次回すぐには着手しない長期の残作業、問題点、保留事項を書く。
現在の状態と次回の作業再開に直接必要な項目は `NEXT.md`、今後も有効な仕様、設計判断、不採用理由、運用制約は `DECISIONS.md` に書く。
完了履歴や検証ログの保管場所にはしない。

## 残作業

- 今回の依頼範囲ではなし。

## 問題点

- 2026-11-09の正本照合では、Sitesの配布ZIPが公開用より古い対象があった。LaserPointerのメニュー／トレイ対応、seminartimerの最前面切替・設定保存、cutterの `--cut` 系と同形式保存、qmlprofileranalyzerのconvert機能を旧ZIPで巻き戻していない。Sites側の配布版を改訂する場合は、対応する開発元と採用版を確認する。
- icppのQt Creatorガイドは公開用0.3.1を保持した。Sitesと配布ZIPの0.3.0を採用し直したものではない。clingの公開用 `build.sh` と配布ZIPの並列数差（8と2）も更新根拠がないため維持した。
- ワークフロー0.2.3の事例文書には非同梱の隣接デモリポジトリーへの参照14箇所（8種類）がある。正本に参照先の前提と観測事実が記載されており、同期時には正本を改変せず保持した。詳しくは `app-generation-workflow/NEXT.md`。

## 保留事項

- qeyesのmacOS復帰・モーダルダイアログの実機操作は今回未検証。Whiteboardはoffscreenの自動テストまでで、別OS・実媒体障害・電源断・手動GUI操作の追加確認は `whiteboard/backlog.md` を参照する。
- Sitesで既存の原稿・実装と一致したnow、tl、qmlprop、VuStereo、ObjectSelector、PropertyEditor、genpdf、ai-cmakeについて、PDFが未収録であるだけの追加はしていない。qmlprofileranalyzerの単体PDFも同様。PDFの生成・表示確認を追加する場合は、同期時のバイト照合とは分けて対象を決める。
