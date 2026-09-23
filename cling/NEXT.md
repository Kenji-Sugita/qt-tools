# NEXT

このファイルは、次回の作業再開に必要な近傍情報だけを書く。
近傍の目安は、直近 5 実作業日以内、または現在の作業テーマが完了するまでに必要な情報とする。
実作業日とは、このリポジトリで実際に作業・判断・検証・コミットを行った日を指す。
セッション数や暦日だけでは区切らない。
完了済みの長い履歴、検証ログ、古い判断経緯は残さず、必要なら承認後に別ファイルへ退避する。
長い残作業、問題点、保留事項は `backlog.md` にまとめる。
NEXT.md には次回再開に必要な要点と、必要なら `backlog.md` への参照だけを書く。
上記のルールおよびこの行は変更しないこと。


## 次にやること

- Windows 11 ARM64 native qtcling の残りの対話動作を確認する。
  - `libedit/` は `michaeljclark/libedit` の固定revision `f3a98319002d0eca05e2a370a68b28a2afa6a822`。
  - `build-win.sh`によるCling/LLVM/libeditのARM64 Release buildと`qtcling-win-arm64/`へのinstallは成功済み。12GB RAMのため並列度は`-j 2`を維持する。
  - `build-win.sh`は`build.sh`と同じsource自動取得、固定revision検証、dirty tree保護、必須patch検証の構成へ統一した。
  - `install-win.ps1`はQt callback DLLをbuildし、Windows版一式を既定で`%LOCALAPPDATA%\qtcling`へinstallする。通常のPowerShellでQt付属CMakeを自動検出し、install後の起動まで実機確認済み。
  - `bin/qtcling.cmd`でGit Bash/Windows Terminalにnative REPLを起動できる。
  - REPL編集、履歴、`qApp`、Qt button click、periodic callback、`qDebug()`出力、ラムダ版`QObject::connect`は実機確認済み。
  - `VERSION`と標準C++ header`<version>`のWindows上の衝突を避けるため`QTCLING_VERSION`へ改名し、`bin/qtcling.cmd`の正常起動を再確認済み。
  - Tab補完はqtclingではなく`icpp`側で扱う。
  - macOSの日本語入力は確認済み。WindowsではC localeによりUTF-8の中間byteが欠落することを確認し、`0009-windows-libedit-utf8-locale.patch`でLineEditor初期化時の`LC_CTYPE=.UTF-8`を追加した。修正版でのIME入力を最終確認する。

- macOS interactive の次の改善対象を決める。
  - Linux実機確認は1.0.0の時点で完了済み。
  - 次の候補は `icpp` 連携整理。
  - `qtcling-project` の改善は進めない。実用 workflow は `icpp` 側に寄せる。

## 未完了

- 次回の作業判断に必要な未完了だけを書く
- 詳細な残作業、問題点、保留事項は `backlog.md` に移す

- Windows ARM64 native qtcling:
  - install先は `libedit-win-arm64/`。`edit.lib`, `compat.lib`, `pdcurses.lib`, `regex.lib`, `terminfo.lib` と `histedit.h` を生成済み。
  - `dumpbin /headers` でdemoと`edit.lib`が`AA64 machine (ARM64)`であることを確認済み。
  - upstreamのx86/x64二択、Windows SDK `10.0.22621.0` hard-code、x64 `_lzcnt_u64`依存を `patch/libedit-windows-arm64.patch` で修正した。
  - `patch/qtcling-interactive/0007-enable-libedit-on-windows.patch` はLLVMのWindows libedit無効化を解除し、portable版の依存libraryもlinkする。
  - `patch/qtcling-interactive/0008-cling-windows-native.patch` は`_WIN32`判定を修正し、不要なJupyter libraryを無効化可能にする。
  - `patch/qtcling-interactive/0009-windows-libedit-utf8-locale.patch` はWindowsのlibeditが出力するUTF-8を`mbrtowc()`で正しくdecodeするため、LineEditor初期化時にWindows限定で`LC_CTYPE=.UTF-8`を設定する。
  - Cling/LLVMは固定revisionでbuild済み。`cling.exe`は`AA64 machine (ARM64)`、CMake cacheは`LLVM_ENABLE_LIBEDIT=FORCE_ON`、生成`config.h`は`HAVE_LIBEDIT TRUE`。
  - periodic callback C APIは`cling.exe`のPE export tableに存在する。
  - Qt kitは`C:\Qt\6.11.1\msvc2022_arm64`。`bin/qtcling.cmd --version`は`1.3`を返す。
  - build treeを指すClang resource探索と、current directory相対だったtermcap探索は修正済み。termcapは`LIBEDIT_TERMCAP`で指定する。
  - Git Bashで`bin/qtcling.cmd`を起動し、行編集、履歴、`qApp`、Qt event、button click、Qt message出力を確認済み。
  - Windowsの日本語入力は手動で`LC_CTYPE`を`.UTF-8`にすると`const auto p = "日本語"`を正常に評価できることを確認済み。自動設定を追加してARM64版を`-j 2`で再build済み。
  - portable libeditの未実装`getline()`をPOSIX互換に修正した。旧実装のNULL buffer書込みがlogo直後のアクセス違反の原因だった。
  - Windowsでは`el_gets()`をメインスレッドに残し、互換I/O層の入力待ちからperiodic callbackを実行する。
  - Qt callbackはARM64 native `bin/qtcling-win.dll`、REPL startupは`src/qtguiwin-startup.cpp`を使う。
  - rootの`VERSION`はWindowsで標準header`<version>`と衝突するため、`QTCLING_VERSION`へ改名した。
  - 詳細は`docs/windows-native-arm64.md`に記録した。


- macOS/Linux側の`origin/master`はWindows ARM64対応commitへmerge済み。
  - merge commitは`8c66550`。
  - Windows native版のビルド手順は`USER_GUIDE.md`と`docs/windows-native-arm64.md`へ反映済み。

- `build.sh` は upstream branch 名ではなく固定 revision を checkout する。
  - Cling は `af630d98`、LLVM は `7c49650f1446`。
  - 異なる revision の既存 checkout は、tracked changes がない場合だけ required revision へ detach checkout する。
  - これにより `cling-latest` の進行で `0002-llvm-lineeditor-periodic-callback.patch` が適用不能になる問題を防ぐ。
  - Ubuntuのbuild必須パッケージは`build-essential`, `cmake`, `ninja-build`, `git`, `python3`, `libedit-dev`。`libzstd-dev`は任意。

- Qt の既定 installation path は `bin/qtcling-defaults.sh` の `QTCLING_DEFAULT_QT_ROOT` で指定する。
  - `build.sh` と launcher はこのファイルを読む。
  - `QTCLING_QT_ROOT` は個別起動用の上書きで、既定 path より優先する。
  - 現在の既定値は Linux ARM64 kit の `/usr/local/qt/Qt/6.11.1/gcc_arm64`。

- qtclingのrelease versionはrootの`QTCLING_VERSION` fileで管理する。
  - 現在値は`1.2.0`。
  - `bin/qtcling --version`は`QTCLING_VERSION`を読んで`qtcling 1.2.0`を表示する。
  - `install.sh`は`QTCLING_VERSION`をinstall prefix rootに入れる。
  - 配布zipにも`QTCLING_VERSION`を含める。

- `release/qtcling-1.2.0-source.zip` と 1.2.0 版 `USER_GUIDE.pdf` は作成済み。
  - zip top directory は `qtcling-1.2.0/`。
  - `unzip -t release/qtcling-1.2.0-source.zip` は OK。
  - zip size は 4908572 bytes。
  - `unzip -l` の total は 5319265 bytes / 66 files。
  - SHA-256 は `6ba75c904ab19f649f0bcfc9bac0646e2de1b751b0723af85bfb55f18df086bf`。
  - 1.2.0 source archiveにはWindows用build script、install script、patch、launcher、callback DLL sourceを含めた。
  - Windows用IME修正patch `0009-windows-libedit-utf8-locale.patch` を含める。
  - 配布先 `/Users/sugita/Sites/tools/qtcling/` へ `USER_GUIDE.md`, `USER_GUIDE.pdf`, `qtcling-1.2.0-source.zip`, `index.html` を配置済み。
  - zipには`VERSION`ではなく`QTCLING_VERSION`を含める。

- `install.sh` は Cling の CMake install に加え、qtcling launcher、platform wrapper、default path 設定、`src/qtgui.cpp` を install する。
  - build 完了後に repository root で `sudo ./install.sh` を実行する。
  - `/usr/local/qtcling/bin/qtcling` を実機で起動確認する。

- `release/qtcling-1.1.0-source.zip` は作成済み。
  - zip top directory は `qtcling-1.1.0/`。
  - `unzip -t release/qtcling-1.1.0-source.zip` は OK。
  - zip size は 4748754 bytes。
  - `unzip -l` の total は 5069046 bytes / 52 files。
  - SHA-256 は `57e5e830ccea15bbef66d4bcac1324d50f744a94cf73e4b976ac972dcfdee3f5`。
  - 1.1.0 の対象外にする `qtcling-gui`, `qtcling-project`, Windows native wrapper, 構想 docs, `build/`, `cling/`, `llvm-project/`, `.DS_Store` は zip から除外済み。
  - 公開先 `/Users/sugita/Sites/tools/qtcling/qtcling-1.1.0-source.zip` への配置はユーザーが行う。
  - `release/qtcling-experimental/` は正式 release flow から外して削除済み。今後の zip 作成は一時 staging directory から行う。

- 2026-05-21 に他の macOS ホストで clean build と実行を確認済み。
  - `build.sh` で clean build できることを確認。
  - `./bin/qtcling` の起動を確認。
  - 1.1.0 は clean build patch detection 修正と install workflow 修正版。

- `build.sh` の patch 適用処理は 2026-05-20 から 2026-05-21 にかけて修正済み。
  - 必須 patch は `0001-cling-add-periodic-callback-api.patch`, `0002-llvm-lineeditor-periodic-callback.patch`, `0003-cling-load-qtcling-startup-file.patch`, `0006-warn-when-libedit-is-disabled.patch`。
  - `patch/qtcling-interactive.patch` はこの 4 本だけを連結した統合パッチ。
  - `0006` は clean `cling v1.3` の `CMakeLists.txt` に存在する位置へ差し替え済み。
  - 個別パッチ再実行時の `--reverse --check` は、後続パッチが同じファイルを触ると失敗するため廃止。
  - 適用済み判定は追加済み symbol/string で行う。

- 1.1.0 の release 案内メールは不要。
  - 旧 release 案内文は `/usr/local/src/cling/qtcling-release-announcement.txt` に保存済み。
  - 本文のみ、`wc -m` は 358 文字。
  - `.git/info/exclude` で Git 対象外。
  - 1.1.0 では送信しない。

## 触るファイル

- `NEXT.md`
- `build.sh`
- `install.sh`
- `install-win.ps1`
- `QTCLING_VERSION`
- `bin/qtcling-defaults.sh`
- `bin/qtcling`
- `bin/iqtcling.linux`
- `bin/iqtcling.macos`
- `bin/cqtcling.linux`
- `bin/cqtcling.macos`
- `patch/qtcling-interactive.patch`
- `patch/qtcling-interactive/0006-warn-when-libedit-is-disabled.patch`
- `USER_GUIDE.md`
- `USER_GUIDE.pdf`
- `README.txt`
- `release/qtcling-1.2.0-source.zip`
- `qtcling-release-announcement.txt`
- `/Users/sugita/Sites/tools/qtcling/index.html`

## 注意

- 作業ツリーには build / source checkout / release zip などの untracked がある。
  - `.DS_Store`, `all/`, `ap298`, `build/`, `cling/`, `llvm-project/`, `examples/cpp-advanced/checklist`, example build directories, `fix.sh`, `libedit-20251016-3.1/`, local patch diffs, `qtcling-release-announcement.txt`, `release/qtcling-experimental.zip`。
  - これらは不用意に削除しない。

- 1.0.0 / 1.0.1 / 1.1.0 / 1.2.0はsource code releaseとして扱う。
  - ユーザー向け対象は terminal `qtcling` interactive。
  - 1.1.0の対象platformはmacOSとLinux。1.2.0でWindows 11 ARM64ネイティブ版を追加する。
  - `qtcling-gui`, `qtcling-project`, `qtcling.toml`, Qt Creator plugin構想は1.2.0のユーザー向け対象外。

- 配布 zip には `build/`, `cling/`, `llvm-project/`, `build-qtcling-gui/`, `icpp`, `.DS_Store` を含めない。

- `cling/` と `llvm-project/` はトップレベル commit 対象外。
  - base revision は `cling` = `af630d98`, `llvm-project` = `7c49650f1446`。
  - patch で再現する方針。

- macOS build は OS 付属 library のみを使う方針。
  - `libedit` は Xcode SDK の `libedit.tbd` を明示し、実行時は `/usr/lib/libedit.3.dylib` 相当を使う。
  - `libzstd` は任意。
  - CMake configure 結果が変わるため、必要なら clean build する。

- macOS terminal focus 問題は環境差があるため、qtcling 側では自動回避しない。
  - `QT_MAC_DISABLE_FOREGROUND_APPLICATION_TRANSFORM` はユーザーが必要に応じて環境変数として設定する。

- Windows 11 ARM64ネイティブ版を1.2.0の正式対象にする。
  - `build-win.sh`, `build-libedit-win.ps1`, `bin/qtcling.cmd`, `windows/`, Windows用patchをsource archiveへ含める。
  - Linux実機確認は1.0.0の時点で完了済み。

- QML / Qt Quick migration支援は1.2.0後の候補。
  - `qtcling` 側は低レベル実行基盤に留める。
  - 履歴、クラッシュ復帰、file 編集、migration workflow は `icpp` 側で扱う。
