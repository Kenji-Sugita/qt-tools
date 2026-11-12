# DECISIONS

## 採用済み

- 表示名とアプリケーションバンドル名は `VuStereo` とする。
- 画面収録権限と既存設定の継続性を優先し、bundle identifier は
  `jp.co.sra.vu-stereo`、QSettings 用 applicationName は `vu-stereo` のまま維持する。
- `VuStereo` は計器としての正確性より、オーディオ機器らしい見た目と針の動きを
  楽しむためのデスクトップアクセサリとして設計する。
- バージョン番号はルートの `VERSION` を単一の情報源にし、CMake project version と
  macOS bundle version はそこから設定する。
- ソース配布物は `cmake --build build --target source_release` で作成し、
  `release/vu-stereo-<version>-source.zip` として保存する。
- ソース配布 zip のトップディレクトリ名は、従来の配布互換性を優先して
  `vu-stereo/` に固定する。
- ローカル開発と身近な環境への配布では、自己署名の
  `vu-stereo Local Code Signing` を使う。
- Developer Program 登録が完了するまでは、Developer ID Application 署名と
  notarization は使わない。

## 不採用

- なし

## まだ決めていないこと

- なし

## 判断基準

- なし

## 継続する仕様・運用制約

- `macdeployqt` は `.app` の内容を書き換えるため、実行後に必ず再署名する。
- 配布用に `macdeployqt` する前は、署名ターゲットを含まない
  `cmake --build build --target VuStereo` でアプリ本体を作る。
- `cmake --build build` は `sign_VuStereo` も実行する。署名 identity が見えない環境では
  署名ターゲットだけ失敗するが、`VuStereo.app/Contents/MacOS/VuStereo` は残す。
- 画面収録権限は bundle identifier とコード署名 identity に依存するため、
  証明書を作り直した場合は再許可が必要になることがある。
- `USER_GUIDE.pdf` と `release/*-source.zip` は、`icpp` と同様にリリース配布物として
  履歴管理する。
