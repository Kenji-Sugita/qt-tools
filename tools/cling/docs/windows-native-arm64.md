# Windows 11 ARM64 ネイティブ版

Windows 11 ARM64ネイティブ版はqtcling 1.2.0の正式対象です。
2026-08-16にWindows 11 ARM64、Qt 6.11.1 ARM64、Cling 1.3の
組み合わせで対話動作を確認しました。

## 確認済みの機能

- `bin/qtcling.cmd` から ARM64 ネイティブの Cling REPL を起動できる。
- libedit によるカーソル移動、行編集、コマンド履歴を利用できる。
- `qApp` から `QApplication` を参照できる。
- REPL が入力待ちの間も Qt event が処理される。
- `QPushButton` の表示と `clicked` signal を利用できる。
- `qDebug()` などの Qt message が Git Bash のコンソールへ表示される。
- Qt の関数ポインタ版およびラムダ版 `QObject::connect` を利用できる。

日本語入力ではWindows C runtimeの文字種localeをUTF-8に設定します。これは
`0009-windows-libedit-utf8-locale.patch`によりWindowsのLineEditor初期化時に
自動設定されます。Tab補完はqtclingではなく`icpp`側で扱います。

## 確認環境

```text
OS       Windows 11 ARM64
Qt       C:\Qt\6.11.1\msvc2022_arm64
Cling    1.3 (ARM64 native)
libedit  michaeljclark/libedit f3a98319002d0eca05e2a370a68b28a2afa6a822
SDK      Windows SDK 10.0.26100.0
Build    Release, -j 2
Memory   12 GB
```

ClingとLLVMは次の固定revisionで確認しました。

```text
cling         af630d98
llvm-project  7c49650f1446
```

## 構成

Windows版では、macOS/Linuxと同じREPL操作を維持するため、portable libeditを
ARM64ネイティブでビルドします。インストール先は `libedit-win-arm64/` です。

Qt event callbackは `bin/qtcling-win.dll` に実装しています。このDLLは
ARM64ネイティブコードで `QApplication` を生成し、入力待ち中に
`QCoreApplication::processEvents()` を呼びます。Windows版libeditは
スレッド依存のI/O実装を持つため、`el_gets()` はメインスレッドに残し、
libedit内部の入力待ちからcallbackを実行します。

Windows固有の起動コードは `src/qtguiwin-startup.cpp` です。ここでは
`QApplication` headerをREPLへ公開し、ClingとMSVC STLの組み合わせで発生する
Qt `connect` の `std::is_convertible` 判定も補正します。これらの処理は
Windows専用ファイルまたは `_WIN32` 条件内にあり、macOS/Linuxには影響しません。

## ビルド

12 GBのメモリ環境では並列度を2に固定します。

Clingを構成する前に、PowerShellでportable libeditをARM64 Releaseとして
ビルドし、`libedit-win-arm64/`へインストールします。

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass `
  -File .\build-libedit-win.ps1
Test-Path .\libedit-win-arm64\lib\edit.lib
Test-Path .\libedit-win-arm64\share\libedit\termcap
```

どちらも`True`になることを確認し、Git BashでCling/LLVMをビルドします。

```sh
./build-win.sh
```

`build-win.sh`もClingの構成前に同じlibeditスクリプトを呼ぶため、先行ビルド済みの場合は
増分処理になります。また、macOS/Linux用`build.sh`と同様に、ClingとLLVMを
必要に応じてcloneし、固定revisionを検証してから必須patchを適用します。
Cling/LLVMのビルドとinstallはいずれもRelease構成です。

Windows用Qt runtime callback DLLは次のCMake projectから生成します。

```powershell
cmake -S .\windows -B .\build-qtcling-win-runtime -A ARM64 `
  -DCMAKE_PREFIX_PATH=C:\Qt\6.11.1\msvc2022_arm64
cmake --build .\build-qtcling-win-runtime --config Release -j 2
Copy-Item `
  .\build-qtcling-win-runtime\bin\Release\qtcling-win.dll `
  .\bin\qtcling-win.dll -Force
Test-Path .\bin\qtcling-win.dll
```

build結果をまとめてinstallする場合はPowerShellで実行します。既定のinstall先は
`%LOCALAPPDATA%\qtcling`です。

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass `
  -File .\install-win.ps1
& "$env:LOCALAPPDATA\qtcling\bin\qtcling.cmd"
```

別のinstall先は`-InstallDir`、別のQt ARM64 kitは`-QtRoot`で指定できます。

## 起動

Git Bash、PowerShell、またはWindows Terminalから起動します。

```sh
bin/qtcling.cmd
```

source rootに`VERSION`という名前のfileを置かないでください。Windowsの
case-insensitive filesystemでは標準C++ header`<version>`と衝突します。
qtclingのrelease versionは`QTCLING_VERSION`に保存します。

既定値は次のとおりです。

```text
QTCLING_CLING_ROOT  <source-root>\qtcling-win-arm64
QTCLING_QT_ROOT     C:\Qt\6.11.1\msvc2022_arm64
QTCLING_STARTUP_FILE <source-root>\src\qtguiwin-startup.cpp
LIBEDIT_TERMCAP     <source-root>\libedit-win-arm64\share\libedit\termcap
```

## 動作例

```cpp
qApp
#include <QPushButton>
const auto b = new QPushButton("OK?");
QObject::connect(b, &QPushButton::clicked,
                 []() { qDebug() << "Clicked"; });
b->show();
```

クリックで表示を変更する例です。Clingのトップレベル変数は通常の自動変数では
ないため、init-captureを使います。

```cpp
QObject::connect(b, &QPushButton::clicked, [button = b]() {
    button->setWindowTitle("Clicked!");
    button->setText("Clicked");
});
```

## Windows版libeditの主な修正

- ARM64をx64として扱わないCMake architecture判定。
- Windows SDK versionのhard-code除去。
- ARM64で利用できないx64 `_lzcnt_u64` 依存の除去。
- install対象へのheader、static library、termcap dataの追加。
- termcap pathを `LIBEDIT_TERMCAP` で指定可能にした。
- `getline()` にPOSIX互換のbuffer確保・拡張・EOF処理を実装した。
- 入力待ち中のcallbackをWindows互換I/O層から呼べるようにした。

特に旧 `getline()` は、履歴読込み時に未確保bufferへ書き込み、REPLがlogo表示後に
アクセス違反で終了する原因になっていました。

## macOS/Linuxとの分離

macOSはXcode SDKのlibedit、Linuxはsystem libeditを引き続き使います。
Windows用portable libedit、DLL、cmd launcher、MSVC STL回避処理は読み込まれません。
共通のperiodic callback APIを除き、今回のWindows固有修正によって
macOS/Linuxの実行経路は変わりません。
