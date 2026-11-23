# Sites 正本の同期（2026-11-24）

- 同期元: Sites `618894eb93495964721f602bc532edc80ce28b52`。4ツールの配布更新は `9365591`。
- 公開側の開始点: `24766f9`。fetch後、`origin/main` の `8f2f471` までfast-forwardし、対外紹介の管理記録を保持した。
- Sites `ee33ffd` 以降の変更を PUBLICATION_MAP と照合した。既存公開対象の更新は4ツールと Work Templates の実践ガイドPDF。HTML、検索索引、Sites作業記録、未収録の新規ツール・記事は取り込まない。

## 反映内容

- qtcling 1.2.0: Qt 6.12.0 の既定キット選択、各OSの起動スクリプト、Windows callback DLL再構成、日英利用ガイド、日本語PDF、配布ZIP。PDFの収録に合わせ日英READMEの案内を復元。
- icpp 1.2.0: 同じQtキットでのビルドとPropertyEditor導入手順、日英利用ガイド、日本語PDF、配布ZIP。
- ObjectSelector 1.0.0: Qt 6.12.0 のビルド例、ビルド先の分離、日英利用ガイド、日本語PDF、配布ZIP。
- PropertyEditor 1.0.0: 同じQtキットを使うビルド例、日英利用ガイド、現行の表示文字列に合わせたテスト、日本語PDF、配布ZIP。
- Work Templates: `next_md_practice_guide.pdf` を正本から収録し、日英READMEへ日本語PDFとしてリンク。
- 製品版番号は正本と同じまま。PDF・ZIPは再生成せず正本からコピーした。

## 保持した公開側の差分

- ObjectSelectorの `BUILD_TESTING=OFF` とQt Test依存の説明は正本でも修正済み。公開側では既存の明示的なコマンド例を維持して更新を統合した。
- icppの `QTCREATOR_GUIDE.md/.pdf` は公開側が新しいため維持。ZIP内の旧ガイドで巻き戻さない。`NEXT.md`・`backlog.md` も公開側の管理記録を維持し、配布ZIP内の開発元記録で上書きしない。
- qtclingの `build.sh` は公開側の `-j 8` と今回の正本が一致するため変更なし。
- genpdf、SeminarTimerの公開用文書補正、各ツールの旧版ZIP、未追跡 `release-order.txt` は保持。
- 同期時にはルートの管理記録を変更せず、後続の利用者の更新・コミット依頼で NEXT.md / DECISIONS.md / backlog.md に同期結果と継続方針・保留事項を反映した。

## 検証

- [manifest.json](manifest.json): コピー対象26対応のSHA-256を照合（ZIP内と単体PDFの重複対応を含む）。配布ZIP4件のCRC検査成功。
- qtclingの変更シェル7件で `sh -n` 成功。既定キット選択4ケースと環境変数上書き1ケースを確認。
- macOS ARM64でqtclingの `--check` 成功、`--show-config` が既定の Qt 6.12.0 を選択することを確認。
- PropertyEditorは Qt 6.12.0 でライブラリー・デモ・テストをビルドし、`QT_QPA_PLATFORM=offscreen ctest` で1件成功。
  - 最初の `/tmp` 配下のビルドはmoc生成ヘッダーの相対パス解決で失敗。`/private/tmp` 指定もCMakeが `/tmp` へ正規化したため同じ失敗。ソースを変更せず、`/Users/sugita/.cache/qt-tools-sync-20261124-propertyeditor` をビルド先にして成功した。
  - 構成時に任意Qtプラグイン依存のprotobuf/protoc警告あり。構成・ビルド・CTestは成功。
- 変更Markdownの相対リンクで新規のリンク切れがないこと、コードフェンスの対、`git diff --check` を確認。
- PDFの全ページ表示確認・再生成、Cling/LLVM全体の再ビルド、対話GUI、Linux/Windows実機試験は今回未実施。Windowsの既存実機確認はQt 6.11.1であり、6.12.0の実機確認とは区別する。
