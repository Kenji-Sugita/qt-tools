# Qt 生成物検証チェックリスト

icpp で Qt の生成物を含むコードを確認するためのチェックリスト。

検証用ファイルは `verification/qt-generated/` に置く。

2026-05-10 時点の非対話検証では、7 ケースすべてで生成物作成と基本評価が通った。
追加検証として、`.autogen on`、非 C++ ファイル編集時の案内、resource 再生成、壊れた `.ui` の失敗系も確認する。

## 共通確認項目

- [x] `.h` / `.ui` / `.qrc` が不要に自動登録されない
- [x] `.cpp` が自動登録される
- [x] `.i .` で include path が解決する
- [x] `.gen` が必要な生成物を作る
- [x] `.r` 相当の再評価が `.gen` 後に行われる
- [x] `.x auto w = go();` などで実行できる
- [x] `.defs` で関数定義を確認できる
- [x] 非対話入力 `input.icpp` で基本確認できる
- [x] `.e <file>` で自然に作成・編集できる
- [x] 途中エラーが出る場合、許容できる途中状態か、仕様改善が必要か判断する
- [ ] USER_GUIDE に書ける手順になっている

## 検証で分かった安定する書き方

- `Q_OBJECT` を持つ class は、検証ケースではヘッダーに inline で最小実装を書くと安定する。
- `Widget::Widget(...)` のような out-of-class 定義は、cling への逐次入力で崩れやすい。
- `.h` / `.ui` / `.qrc` / `.ts` は `.e` で編集できるが、自動登録されない。まだ `.cpp` が登録されていない場合、編集後は評価せず、`.e <file.cpp>` または `.add <file.cpp>` の案内が出る。
- `moc_widget.cpp` は `widget.cpp` 末尾で `#include "moc_widget.cpp"` する。
- `ui_form.h` は `.add` せず、利用側 `.cpp` から `#include "ui_form.h"` する。
- `qrc_resources.cpp` / `qrc_translations.cpp` は `.add` せず、利用側 `.cpp` から include する方が安定する。
- `.qrc` に `.qm` を含める場合は、先に `lrelease`、その後に `.gen` / `rcc` を実行する。

## 1. Header + Q_OBJECT

想定操作:

```text
icpp[qtcling]> .reset
icpp[qtcling]> .i .
icpp[qtcling]> .e widget.h
icpp[qtcling]> .e widget.cpp
icpp[qtcling]> .gen
icpp[qtcling]> .x auto w = go();
```

確認:

- [x] `widget.h` は自動登録されない
- [x] `widget.cpp` は自動登録される
- [x] `moc_widget.cpp` が生成される
- [x] `widget.cpp` 末尾の `#include "moc_widget.cpp"` が通る
- [x] `go()` を定義して呼べる
- [x] 非対話検証 `01-header-qobject/input.icpp` が通る

メモ:

- 検証ケースでは constructor や `paintEvent()` をヘッダー内 inline 定義にした。
- out-of-class 定義は `Widget::Widget(...)` が対話入力で崩れるため、非対話検証では避ける。

## 2. .ui Only

想定操作:

```text
icpp[qtcling]> .reset
icpp[qtcling]> .i .
icpp[qtcling]> .e form.ui
icpp[qtcling]> .e widget.cpp
icpp[qtcling]> .gen
icpp[qtcling]> .x auto w = go();
```

確認:

- [x] `form.ui` は自動登録されない
- [x] `.gen` で `ui_form.h` が生成される
- [x] `widget.cpp` から `#include "ui_form.h"` できる
- [x] `ui_form.h` は `.add` しない
- [x] `go()` を定義して呼べる
- [x] 非対話検証 `02-ui-only/input.icpp` が通る

## 3. Q_OBJECT + .ui

想定操作:

```text
icpp[qtcling]> .reset
icpp[qtcling]> .i .
icpp[qtcling]> .e widget.h
icpp[qtcling]> .e form.ui
icpp[qtcling]> .e widget.cpp
icpp[qtcling]> .gen
icpp[qtcling]> .x auto w = go();
```

確認:

- [x] `moc_widget.cpp` が生成される
- [x] `ui_form.h` が生成される
- [x] `widget.cpp` で `#include "widget.h"` と `#include "ui_form.h"` が通る
- [x] `widget.cpp` 末尾の `#include "moc_widget.cpp"` が通る
- [x] `go()` を定義して呼べる
- [x] 非対話検証 `03-qobject-ui/input.icpp` が通る

メモ:

- slot はヘッダー内 inline 定義にした。

## 4. .qrc Only

想定操作:

```text
icpp[qtcling]> .reset
icpp[qtcling]> .i .
icpp[qtcling]> .e resources.qrc
icpp[qtcling]> .e widget.cpp
icpp[qtcling]> .gen
icpp[qtcling]> .p resourceText()
```

確認:

- [x] `resources.qrc` は自動登録されない
- [x] `.gen` で `qrc_resources.cpp` が生成される
- [x] `qrc_resources.cpp` を `widget.cpp` から include して resource が使える
- [x] `:/...` パスでテキストを読める
- [x] 非対話検証 `04-qrc-only/input.icpp` が通る

メモ:

- `qrc_resources.cpp` を `.add` で直接登録すると、生成コードのプリプロセッサ構造が対話入力で崩れる。
- include 方式に寄せるのが安定。

## 5. Q_OBJECT + .qrc

想定操作:

```text
icpp[qtcling]> .reset
icpp[qtcling]> .i .
icpp[qtcling]> .e widget.h
icpp[qtcling]> .e resources.qrc
icpp[qtcling]> .e widget.cpp
icpp[qtcling]> .gen
icpp[qtcling]> .x auto w = go();
icpp[qtcling]> .p resourceText()
```

確認:

- [x] `moc_widget.cpp` が生成される
- [x] `qrc_resources.cpp` が生成される
- [x] `widget.cpp` 末尾の `#include "moc_widget.cpp"` が通る
- [x] `qrc_resources.cpp` を `widget.cpp` から include して `:/...` resource が使える
- [x] 非対話検証 `05-qobject-qrc/input.icpp` が通る

## 6. Q_OBJECT + .ui + .qrc

想定操作:

```text
icpp[qtcling]> .reset
icpp[qtcling]> .i .
icpp[qtcling]> .e widget.h
icpp[qtcling]> .e form.ui
icpp[qtcling]> .e resources.qrc
icpp[qtcling]> .e widget.cpp
icpp[qtcling]> .gen
icpp[qtcling]> .x auto w = go();
icpp[qtcling]> .p resourceText()
```

確認:

- [x] `moc_widget.cpp` が生成される
- [x] `ui_form.h` が生成される
- [x] `qrc_resources.cpp` が生成される
- [x] `widget.cpp` で UI と resource を同時に使える
- [x] `.gen` の順序で破綻しない
- [x] 非対話検証 `06-qobject-ui-qrc/input.icpp` が通る

## 7. Translations

現時点では `lrelease` は `.gen` に統合されていない前提。
このケースは `Q_OBJECT` や `.ui` を使わず、`.ts` / `.qm` / `.qrc` の翻訳 resource だけを確認する。

想定操作:

```text
icpp[qtcling]> .reset
icpp[qtcling]> .i .
icpp[qtcling]> .e translations.qrc
icpp[qtcling]> .e app_ja.ts
icpp[qtcling]> .e widget.cpp
icpp[qtcling]> .! lrelease app_ja.ts
icpp[qtcling]> .gen
icpp[qtcling]> .p translatedText()
```

確認:

- [x] `lrelease app_ja.ts` で `.qm` が生成される
- [x] `.qrc` が `.qm` を含む場合、`lrelease` 後に `.gen` / `rcc` する
- [x] `qrc_translations.cpp` を `widget.cpp` から include すると `.qm` resource が使える
- [x] `QTranslator` で翻訳を読み込める
- [x] 非対話検証 `07-translations/input.icpp` が通る
- [ ] 将来 `qtcling-project prepare` へ寄せるべき点を記録する

## 8. autogen + .qrc

想定操作:

```text
icpp[qtcling]> .reset
icpp[qtcling]> .i .
icpp[qtcling]> .autogen on
icpp[qtcling]> .add widget.cpp
icpp[qtcling]> .r
icpp[qtcling]> .p resourceText()
```

確認:

- [x] `.gen` を明示せず、`.r` の前に `run_all` が走る
- [x] `qrc_resources.cpp` が生成される
- [x] 非対話検証 `08-autogen-qrc/input.icpp` が通る

## 9. .ui 編集時の案内

想定操作:

```text
icpp[qtcling]> .reset
icpp[qtcling]> .e form.ui
```

確認:

- [x] `.ui` は自動登録されない
- [x] Qt 生成物が必要な可能性を示す警告が出る
- [x] 評価対象 C++ がない場合、`.e <file.cpp>` または `.add <file.cpp>` を案内する
- [x] 汎用的な `No files or edit buffer to run.` ではない

## 10. .qrc 再生成

想定操作:

```text
icpp[qtcling]> .reset
icpp[qtcling]> .i .
icpp[qtcling]> .add widget.cpp
icpp[qtcling]> .gen
icpp[qtcling]> .p resourceText()
icpp[qtcling]> .! printf 'second resource\n' > message.txt
icpp[qtcling]> .gen
icpp[qtcling]> .p resourceText()
```

確認:

- [x] 初回 `.gen` で `first resource` が読める
- [x] resource 変更後の `.gen` で `second resource` が読める
- [x] 非対話検証 `10-qrc-regeneration/input.icpp` が通る

## 11. 壊れた .ui の失敗系

想定操作:

```text
icpp[qtcling]> .reset
icpp[qtcling]> .i .
icpp[qtcling]> .add widget.cpp
icpp[qtcling]> .gen
icpp[qtcling]> .errors
```

確認:

- [x] 壊れた `.ui` で `.gen` が失敗する
- [x] 失敗内容を `.errors` で確認できる
- [x] `.gen` 失敗時は登録済み C++ の再評価に進まない
