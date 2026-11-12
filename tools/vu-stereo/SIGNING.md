# macOS の署名と画面収録権限

このアプリは macOS 13.x で ScreenCaptureKit を使ってシステム音声を取得します。
Finder からの起動と画面収録権限はアプリのコード署名 identity に依存するため、
開発ビルドでも安定したローカル証明書で署名する必要があります。

表示名とアプリケーションバンドル名は `VuStereo` ですが、画面収録権限との互換性を
保つため、コード署名証明書名 `vu-stereo Local Code Signing` と bundle identifier
`jp.co.sra.vu-stereo` は維持しています。

## ローカルコード署名証明書を作成する

1. `キーチェーンアクセス` を開く。
2. メニューから `キーチェーンアクセス` > `証明書アシスタント` > `証明書を作成...`
   を選ぶ。
3. 次のように設定する。
   - 名前: `vu-stereo Local Code Signing`
   - 識別情報のタイプ: `自己署名ルート`
   - 証明書のタイプ: `コード署名`
4. `デフォルトを無効化` にチェックを入れる。
5. ウィザードを進め、保存先は `ログイン` キーチェーンを選ぶ。
6. キーチェーンアクセスで `ログイン` キーチェーンを選び、
   `vu-stereo Local Code Signing` を探す。
7. 証明書を開き、`信頼` を展開し、
   `この証明書を使用するとき` を `常に信頼` に変更する。
8. ウィンドウを閉じ、求められたら macOS のパスワードを入力する。

ターミナルで確認する。

```bash
security find-identity -v -p codesigning
```

期待する出力:

```text
"vu-stereo Local Code Signing"
```

## ビルド

証明書が見えているターミナルからビルドする。

```bash
cd /Users/sugita/src/qt/tools/vu-stereo
cmake --build build
```

リンク後に `cmake/sign_app.cmake` が実行されます。
`vu-stereo Local Code Signing` が見えない場合はビルドを失敗させます。
これは、画面収録権限と一致しない ad-hoc 署名版を誤って作らないためです。

署名は `sign_VuStereo` ターゲットとして実行されます。署名に失敗した場合でも、
`VuStereo.app/Contents/MacOS/VuStereo` は残るため、原因確認や再署名ができます。

署名を確認する。

```bash
codesign -dvvv build/VuStereo.app 2>&1 | grep -E 'Authority|Signature|Identifier'
```

期待する出力:

```text
Identifier=jp.co.sra.vu-stereo
Authority=vu-stereo Local Code Signing
```

## macdeployqt 後に署名する

`.app` をコピーして `macdeployqt` を実行した場合は、`macdeployqt` の後に署名します。
`macdeployqt` は `.app` の中へ Qt Framework やプラグインを追加し、内容を書き換える
ため、先に署名してもその署名は無効になります。

`macdeployqt` は、実行ファイルを含むクリーンな `.app` に対して実行してください。
署名失敗後や途中で失敗した `macdeployqt` の残骸がある場合は、その `.app` を作り直して
から実行します。

配布用に `macdeployqt` する場合は、署名ターゲットを含む既定ビルドではなく、まず
アプリ本体だけを作ります。

```bash
cmake --build build --target VuStereo
```

途中で失敗した `macdeployqt` の残骸が残っている場合は、`build/VuStereo.app` を
作り直してからやり直してください。`VuStereo.app/Contents/MacOS/VuStereo` が存在
しない状態で `macdeployqt` を実行すると、`Could not find bundle binary` になります。

手順:

```bash
cd /Users/sugita/src/qt/tools/vu-stereo

macdeployqt path/to/VuStereo.app

codesign --force --deep --timestamp=none \
  --sign "vu-stereo Local Code Signing" \
  path/to/VuStereo.app
```

署名を確認する。

```bash
codesign -dvvv path/to/VuStereo.app 2>&1 | grep -E 'Authority|Signature|Identifier'
```

期待する出力:

```text
Identifier=jp.co.sra.vu-stereo
Authority=vu-stereo Local Code Signing
```

より厳しく確認する場合:

```bash
codesign --verify --deep --strict --verbose=2 path/to/VuStereo.app
```

`.app` の中身を後から変更した場合は、もう一度 `codesign` を実行してください。

## macdeployqt で dmg を作る場合

`macdeployqt -dmg` で `.dmg` を作る場合も、重要なのは `.dmg` の中に入る前の
`.app` を署名しておくことです。画面収録権限で macOS が見るのは、`.app` の
bundle identifier とコード署名 identity です。

順番:

```bash
macdeployqt path/to/VuStereo.app

codesign --force --deep --timestamp=none \
  --sign "vu-stereo Local Code Signing" \
  path/to/VuStereo.app

codesign --verify --deep --strict --verbose=2 path/to/VuStereo.app

macdeployqt path/to/VuStereo.app -dmg
```

`macdeployqt -dmg` は、署名済みの `.app` を入れた `.dmg` を作る手順として使います。
この手順で作る配布用ディスクイメージ名は `VuStereo.dmg` です。
`.dmg` 自体も署名できますが、画面収録権限との対応で重要なのは `.app` 側の署名です。
個人利用やローカル配布では、まず `.app` が正しく署名されていれば十分です。
一般配布では、Apple Developer ID 署名と notarization を使うのが適切です。

## zip で他の Mac に持っていく場合

`macdeployqt` した `.app` に署名してから `.zip` にし、他の Mac に持っていって
動作することを確認済みです。

手順:

```bash
macdeployqt path/to/VuStereo.app

codesign --force --deep --timestamp=none \
  --sign "vu-stereo Local Code Signing" \
  path/to/VuStereo.app

codesign --verify --deep --strict --verbose=2 path/to/VuStereo.app

ditto -c -k --keepParent path/to/VuStereo.app VuStereo.zip
```

受け取り側の Mac では、初回起動時に画面収録権限の許可が必要です。
画面収録を許可した後、`VuStereo` を完全に終了して再起動します。

注意:

- `.zip` にする前に `.app` を署名します。
- `.zip` から展開した後に `.app` の中身を変更した場合は、再署名が必要です。
- 自己署名証明書での署名は、ローカル利用や身近な環境での配布向けです。
- 広く配布する場合は、Apple Developer ID 署名と notarization を使います。

## 初回の画面収録権限設定

権限をリセットする必要がある場合:

```bash
tccutil reset ScreenCapture jp.co.sra.vu-stereo
killall Finder
```

その後、次の手順を行う。

1. Finder から `build/VuStereo.app` を起動する。
2. 画面収録のダイアログが出たら、システム設定を開く。
3. 画面収録の一覧で `VuStereo` を ON にする。
4. `VuStereo` を完全に終了する。
5. Finder から `build/VuStereo.app` を再度起動する。

この設定後は、同じ bundle identifier と同じローカル証明書でビルドする限り、
通常は再度許可する必要はありません。

## 再度許可が必要になる場合

次の場合は、権限のリセットまたは再許可が必要になることがあります。

- `CFBundleIdentifier` が `jp.co.sra.vu-stereo` から変わった。
- ローカル署名証明書を作り直した、または名前を変えた。
- ad-hoc 署名されたアプリをビルドまたは起動した。
- `tccutil reset ScreenCapture jp.co.sra.vu-stereo` を実行した。
- システム設定で画面収録の許可を手動で OFF にした。

## 他の人がソースコードを受け取った場合

他の人がこのソースコードを受け取ってビルドする場合も、同じ考え方で署名します。
ただし、同じ証明書を共有する必要はありません。

各 Mac 上で、同じ名前の自己署名コード署名証明書を作成してください。

```text
vu-stereo Local Code Signing
```

その後、その Mac で通常通りビルドします。

```bash
cmake -S . -B build
cmake --build build
```

`cmake/sign_app.cmake` は `vu-stereo Local Code Signing` という名前の証明書を探して
署名します。見つからない場合はビルドを失敗させます。これは、画面収録権限と一致し
ない ad-hoc 署名版を誤って作らないためです。

他の人が必要な作業は次の通りです。

1. `vu-stereo Local Code Signing` というコード署名証明書を作る。
2. 証明書を `常に信頼` に設定する。
3. `cmake -S . -B build` を実行する。
4. `cmake --build build` を実行する。
5. Finder から `build/VuStereo.app` を起動する。
6. 画面収録を許可する。
7. `VuStereo` を完全に終了して再起動する。

注意:

- 画面収録権限は Mac ごとに必要です。
- 証明書を作り直すと再許可が必要になることがあります。
- 開発や個人利用では自己署名で十分です。
- 一般配布する場合は、Apple Developer ID 署名と notarization を使うのが適切です。
