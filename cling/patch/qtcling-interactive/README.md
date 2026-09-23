# qtcling interactive patches

`cling/` と `llvm-project/` の両方を含むディレクトリから適用する。

```sh
git apply patch/qtcling-interactive/0001-cling-add-periodic-callback-api.patch
git apply patch/qtcling-interactive/0002-llvm-lineeditor-periodic-callback.patch
git apply patch/qtcling-interactive/0003-cling-load-qtcling-startup-file.patch
git apply patch/qtcling-interactive/0006-warn-when-libedit-is-disabled.patch
git apply patch/qtcling-interactive/0007-enable-libedit-on-windows.patch
git apply patch/qtcling-interactive/0008-cling-windows-native.patch
```

統合パッチを 1 回で適用する場合:

```sh
git apply patch/qtcling-interactive.patch
```

`patch/qtcling-interactive.patch` は上の 4 つの必須パッチだけを連結したもの。top-level wrapper 用の参考パッチは含めない。

これらのパッチは次の base revision で確認した。

```text
cling        af630d98 Prepare for releasing cling v1.3
llvm-project 7c49650f1446 Implement FileManager uncaching.
```

別ホストで同じ patched tree を再現する場合は、`cling/` と
`llvm-project/` を同じ revision に checkout し、必要なら未追跡ファイルを
clean してから、統合パッチを適用して Cling を rebuild する。

```sh
git apply patch/qtcling-interactive.patch
cmake --build build --target cling
```

この workflow で `git clean -dxf` を使う場合は、必要な未追跡ファイルが
残っていないことを先に確認する。top-level の patch files は clean 前に
commit しておく必要がある。そうしておけば、clean checkout 後に
`cling/` と `llvm-project/` の変更を再生成できる。

パッチ内容:

- `0001`: interactive prompt が入力待ちしている間に periodic callback を登録するための Cling C API を追加する。WindowsではJITから解決できるようAPIをPE exportする。
- `0002`: LLVM `LineEditor` に periodic callback 実装を追加する。callback support は libedit build で有効になり、fallback line editor では no-op になる。
- `0003`: line editor が有効になった後、interactive startup 時に `QTCLING_STARTUP_FILE` を読み込む。
- `0004`: macOS/Linux wrapper の temporary preload header 作成を修正し、macOS wrapper が利用可能な場合に local development `build/bin/cling` を優先するようにする。
  - この変更は top-level repo には commit 済みなので、最新 checkout では通常適用しない。
  - 古い top-level checkout に wrapper 修正だけを backport する場合の参考パッチとして残している。
- `0005`: Linux wrapper に `--show-config` / `--check` と `QTCLING_MODULES` 連動の include / preload / link 引数生成を追加する。
  - この変更は top-level repo 用の補助パッチ。
  - `patch/qtcling-interactive.patch` には含めない。
  - Linux 側で先に試す場合は、top-level repo で `git apply patch/qtcling-interactive/0005-linux-wrapper-diagnostics.patch` を実行する。
- `0006`: libedit が無効な build で、REPL 入力待ち中に Qt event が処理されない可能性を CMake configure 時に警告する。
- `0007`: portable libedit とその依存 static library を Windows でも検出し、LLVM `LineEditor` の libedit backend を有効にする。
- `0008`: Clingのnative Windows compileとinstalled Clang resource探索を修正し、不要なJupyter libraryをWindows ARM64 buildから除外可能にする。
- `0009`: Windows libeditのUTF-8入力を正しくdecodeするため、LineEditor初期化時にWindows限定で文字種localeをUTF-8に設定する。
  - Windows専用のため `patch/qtcling-interactive.patch` には含めない。

## Windows 11 ARM64 開発版

Windows ARM64ではportable libeditのI/Oがthread-affineであるため、`el_gets()`を
worker threadへ移さない。libedit互換I/O層の入力待ちからperiodic callbackを呼び、
Qt callback本体はARM64 native `qtcling-win.dll`に置く。

portable libeditにはarchitecture/SDK/termcap対応に加えて、履歴読込みでNULL bufferへ
書き込んでいた`getline()`の修正が必要である。Cling側ではWindows分岐を含む
LineEditor callback実装も必要になる。現在の実機確認済み構成と適用範囲は
[`docs/windows-native-arm64.md`](../../docs/windows-native-arm64.md)を参照する。
