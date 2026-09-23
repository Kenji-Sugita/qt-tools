# NEXT

このファイルを読む、または更新する前に、`~/AGENTS.next.md` を読み、その指示に従うこと。
`~/AGENTS.next.md` が存在しない場合は、このファイルを整理・削除せず、利用者に確認すること。
このルールは、ユーザーから明示的な指示がない限り変更しない。

## 次にやること

- 次回は、必要なら `VuStereo.dmg` と `release/vu-stereo-1.0.0-source.zip` の配布内容を
  最終確認する。
- ソース配布 zip は `cmake --build build --target source_release` で作成する。
- ソース配布 zip のトップディレクトリ名は `vu-stereo/`。
- ソース配布 zip には `CMakeLists.txt`、`cmake/`、`src/`、`include/`、`images/`、
  `translations/`、`README.md`、`CHANGELOG.md`、`VERSION`、`PRIVACY.md`、
  `SIGNING.md`、`USER_GUIDE.md`、`USER_GUIDE.pdf` を入れている。
- `/Users/sugita/Sites/tools/vu-stereo/` には、配布ページ、利用ガイド、PDF、
  プライバシーポリシー、README、CHANGELOG、`vu-stereo.zip`、
  `vu-stereo-1.0.0-source.zip` をコピー済み。
- `VuStereo.dmg` はアプリ配布用。ユーザーが作成済み。今回の source release 作成では
  更新していない。
- code signing は復旧済み。ユーザー環境では `vu-stereo Local Code Signing` が
  `security find-identity -v -p codesigning` に出るようになり、`build/VuStereo.app`
  の `codesign --verify --deep --strict --verbose=2` も通った。
- `VuStereo` の狙いは、正確な計器ではなく、オーディオ機器らしい見た目と針の動きを
  楽しむデスクトップアクセサリ。README と USER_GUIDE に反映済み。

## 未完了

- 次回の作業判断に必要な未完了だけを書く
- 詳細な残作業、問題点、保留事項は `backlog.md` に移す

- 未完了の実装タスクはなし。
- `VuStereoArc.png` と `VuStereoLinear.png` が未追跡。配布ページ用スクリーンショット
  として使っているが、コミットするかどうかは未決。

## 触るファイル

- `CMakeLists.txt`
- `README.md`
- `CHANGELOG.md`
- `VERSION`
- `PRIVACY.md`
- `USER_GUIDE.md`
- `USER_GUIDE.pdf`
- `cmake/make_source_release.cmake`
- `release/vu-stereo-1.0.0-source.zip`
- `DECISIONS.md`
- `NEXT.md`

## 現在の状態

- 直近コミット:
  - このファイル更新時点では、次のコミットで `NEXT.md`、バージョン管理、
    source release 配布物、README、CHANGELOG、PRIVACY、USER_GUIDE をまとめて
    コミットする予定。
- `git status --short -- qt/tools/vu-stereo` で残っている未追跡:
  - `qt/tools/vu-stereo/:-`
  - `qt/tools/vu-stereo/build/`
  - `qt/tools/vu-stereo/VuStereo.dmg`
  - `qt/tools/vu-stereo/VuStereoArc.png`
  - `qt/tools/vu-stereo/VuStereoLinear.png`
  - `qt/tools/vu-stereo/vu-stereo.zip` が存在する場合は旧来名の配布コピー。現在は
    `release/vu-stereo-1.0.0-source.zip` を正とする。
- 今回コミット対象:
  - `CMakeLists.txt`
  - `README.md`
  - `CHANGELOG.md`
  - `VERSION`
  - `PRIVACY.md`
  - `USER_GUIDE.md`
  - `USER_GUIDE.pdf`
  - `cmake/make_source_release.cmake`
  - `release/vu-stereo-1.0.0-source.zip`
  - `DECISIONS.md`
  - `NEXT.md`
- `:-` は `codesign` の cdhash 出力が誤って保存された作業ゴミ。
- `build/`、`VuStereo.dmg`、`vu-stereo.zip` は生成物。ユーザー指示なしに削除しない。

## 注意

- `CMakeLists.txt` を修正する前に `~/AGENTS.cmake.md` を読む。
- ユーザー指示なしに機能追加・仕様変更しない。
- `build/`、`.app`、`.zip`、`.dmg`、`.DS_Store`、`:-` はコミットしない。
- 例外として、`release/*-source.zip` は source release 配布物としてコミットする。
- `.app` は上位 `.gitignore` の `*.app` で無視されている。
- アプリ名は `VuStereo`。ただし互換性のため bundle identifier は `jp.co.sra.vu-stereo`、
  署名証明書名は `vu-stereo Local Code Signing`、QSettings 用内部名は `vu-stereo` のまま。
- バージョン番号は `VERSION` を更新してから `cmake -S . -B build` する。
- source release は `cmake --build build --target source_release` で作る。
- `cmake --build build --target VuStereo` はアプリ本体だけを作る。`macdeployqt` 前はこのターゲットを使う。
- `cmake --build build` は `sign_VuStereo` も走る。こちらの環境では最後の署名で
  `vu-stereo Local Code Signing` が見えず失敗するが、`VuStereo.app/Contents/MacOS/VuStereo`
  は残る。
- ユーザー環境では、秘密鍵付き `.p12` を `login.keychain-db` に import し直して
  `vu-stereo Local Code Signing` が有効な codesigning identity になった。
- 署名後の確認コマンド:
  `codesign -dvvv build/VuStereo.app 2>&1 | grep -E 'Authority|Signature|Identifier'`
  と `codesign --verify --deep --strict --verbose=2 build/VuStereo.app`。
- `macdeployqt` 後は `.app` に再署名してから zip/dmg 化する。詳細は `SIGNING.md`。
- `/next` はクライアント側のスラッシュコマンドとして弾かれる場合がある。疑似ショートカットにするなら `next` や `@next` のように `/` 以外を使う。
