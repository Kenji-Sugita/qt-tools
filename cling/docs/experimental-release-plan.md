# Experimental Release Plan

対象は「今の macOS 版を試してもらうための実験的バイナリ配布」です。
この前提なら、`.zip` は「そのまま再ビルドするための完全ソース一式」ではなく、
「実行に必要な最小ランタイム + 起動スクリプト + 最低限の説明」に絞るのが妥当です。

## 1. 入れるべきもの

### 必須

- `README-experimental.md`
  - これは新規に用意する
  - 対応 OS
  - 前提条件
  - 起動方法
  - 既知の制限
  - 問題報告方法

- `LICENSE.TXT`
  - `cling/LICENSE.TXT`

- `CREDITS.txt`
  - `cling/CREDITS.txt`

- `bin/`
  - `bin/qtcling`
  - `bin/iqtcling.macos`
  - `bin/cqtcling.macos`
  - `bin/run_moc`
  - `bin/run_rcc`
  - `bin/run_uic`
  - `bin/run_moc.py`
  - `bin/run_rcc.py`
  - `bin/run_uic.py`

- `include/`
  - `include/preload_qt_macos.h`
  - macOS 実験版なら `preload_qt_linux.h` は不要

- `examples/`
  - `examples/hello.cpp`
  - `examples/button.cpp`
  - `examples/object.cpp`
  - `examples/slider.cpp`
  - `examples/string.cpp`
  - 実験版では、まず小さな例だけで十分

- `docs/`
  - `docs/samples-repl.md`
  - 必要なら `docs/announce` を簡略版 README に転記

- `runtime/bin/`
  - `/usr/local/qtcling/bin/cling`

- `runtime/lib/`
  - `/usr/local/qtcling/lib/libcling.dylib`
  - `/usr/local/qtcling/lib/libclang-cpp.dylib`
  - `/usr/local/qtcling/lib/libclang.dylib`
  - `/usr/local/qtcling/lib/libLTO.dylib`
  - `/usr/local/qtcling/lib/libRemarks.dylib`

- `runtime/lib/clang/20/include/`
  - Clang resource headers
  - サイズは約 7.6MB で、実行時に必要

### あると良い

- `patch/`
  - この配布物が upstream の Cling そのままではないことを示すため
  - 実際に適用した差分だけを入れる
  - 候補:
    - `patch/cling-fix-L-command-silent-failure.patch`
    - `patch/cling-missing-dotl-warning.diff`
    - `patch/readline-patch-japanese-input/...`

- `QTCLING_VERSION`
  - qtcling の release version
  - Windows の case-insensitive filesystem では `VERSION` が標準 C++ header `<version>` と衝突するため使わない

- `BUILD-INFO.txt`
  - ビルド日
  - 使用した Qt の版
  - 使用した Cling / LLVM ベース
  - 実験版であること

## 2. 入れない方がよいもの

- `build/`
  - ビルド中間物と LLVM ツールが大量に入っている
  - このままだと配布物が不要に巨大になる

- `cling/`
  - フルソース一式は実験版 zip には過剰
  - もしソース公開もしたいなら別 zip に分ける

- `llvm-project/`
  - これは絶対に分けるべき
  - サイズが大きすぎる

- `/usr/local/qtcling/bak`
  - バックアップであり配布不要

- `include/clang`, `include/llvm`, `include/llvm-c`, `include/clang-c`
  - 通常の qtcling 利用者には不要
  - 開発 SDK を配るのでなければ外す

- `lib/*.a`
  - 静的ライブラリ群
  - 実行配布には不要

- `lib/cmake/`
  - SDK 用
  - 実験的バイナリ利用には不要

- `share/scan-build`, `share/scan-view`, `share/man`, `share/clang`
  - 実行には不要

- `tools/propertyeditor/`
  - 別テーマの検証物が多い

- `sessions/`, `memo.md`, `misc/`, `.DS_Store`
  - 開発メモと作業ログであり、配布不要

## 3. 現状の重要なリリース注意点

現状の起動スクリプトは絶対パスに強く依存している。
このまま zip すると、展開先によっては起動しない。

特に注意が必要なもの:

- `bin/iqtcling.macos`
  - `CLING_ROOT=/usr/local/src/cling/build` になっている

- `bin/cqtcling.macos`
  - `cling_path=/usr/local/cling/bin/cling` になっている

- `bin/iqtcling.macos`
  - `DEV_ROOT=/usr/local/src/cling`
  - `preload_qt_macos.h` を配布物内ではなく開発ツリーから読んでいる

つまり、配布前に最低でも次を行う必要がある:

- スクリプトを zip 内相対パス基準に直す
- `runtime/` を見るようにする
- `include/preload_qt_macos.h` を zip 内から読むようにする

## 4. 実験版として妥当な zip 構成

以下のような構成が扱いやすい:

```text
qtcling-experimental-macos/
  README-experimental.md
  LICENSE.TXT
  CREDITS.txt
  BUILD-INFO.txt
  QTCLING_VERSION
  bin/
  include/
  docs/
  examples/
  runtime/
    bin/
      cling
    lib/
      libcling.dylib
      libclang-cpp.dylib
      libclang.dylib
      libLTO.dylib
      libRemarks.dylib
      clang/
        20/
          include/
  patch/
```

## 5. いまの結論

実験版なら、配布物は次の 3 層で十分です。

1. `qtcling` 用の薄い起動スクリプト群
2. `cling` 実行ランタイムの最小集合
3. 最低限の README / LICENSE / 例題

逆に、`build/`, `cling/`, `llvm-project/` を丸ごと入れるのは避けるべきです。
それは「実験版バイナリ」ではなく「開発環境の丸ごとコピー」に近くなります。
