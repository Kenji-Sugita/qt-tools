# qtcling: Qt 設定の自動化と実験版リリース課題

## 背景

qtcling は、Cling REPL 上で Qt GUI を表示しながら C++/Qt コードを対話的に試すための実験的環境である。

現状でも Qt GUI と REPL の併用は成立しているが、Qt の利用に必要な `-I`, `-L`, `-F`, preload 指定が複雑で、ユーザーが手で設定するには負担が大きい。

特に Qt のインストール場所、platform ごとの framework / shared library の違い、Qt module の include path / library path の違いが鬼門になっている。

## 現状の位置づけ

現時点の qtcling は experimental release として扱う。

目的は、安定した一般利用環境を提供することではなく、以下を検証することにある。

- Cling REPL から Qt object / widget を対話的に生成できること
- GUI を表示したまま REPL 入力を継続できること
- Qt event processing を REPL 入力待ち中に回せること
- Qt を対話的に試す workflow の有用性を確認すること

## 既知の課題

### 1. `-I`, `-L`, `-F` の指定がユーザーに見えている

Qt を使うために必要な include path / library path / framework path の指定が複雑である。

ユーザーが直接扱うべきではないもの:

- `-I QtCore`
- `-I QtGui`
- `-I QtWidgets`
- `-L`
- `-F`
- `#pragma cling load(...)`
- platform ごとの Qt library 名
- Qt installation root の違い

### 2. 起動スクリプトが絶対パスに依存している

現状の script は開発環境のパスに強く依存している。

例:

- `/usr/local/src/cling`
- `/usr/local/qt/Qt/6.11.1/...`
- `/usr/local/qtcling`

実験環境では許容できるが、配布物としては修正が必要。

### 3. Qt event loop 統合はまだ実験的

REPL 入力待ち中に Qt event を処理する方式は成立しているが、入力処理・libedit・TTY・threading まわりには注意が必要。

この段階では stable API として扱わない。

## 改善方針

### 1. Qt 設定を自動検出する

qtcling 起動時に Qt installation root を探す。

優先順の案:

1. `QTCLING_QT_ROOT`
2. CLI option: `qtcling --qt <path>`
3. project config: `./qtcling.toml`
4. user config: `~/.qtcling/config.toml`
5. `Qt_DIR` / `CMAKE_PREFIX_PATH`
6. `qmake -query QT_INSTALL_PREFIX`
7. `qtpaths --install-prefix`
8. platform ごとの既知パス

### 2. ユーザー向け option を高水準にする

ユーザーには低レベルな `-I`, `-L`, `-F` を触らせない。

候補:

```sh
qtcling
qtcling --qt /path/to/Qt/6.11.1/macos
qtcling --profile qt6-macos
qtcling --modules Core,Gui,Widgets
qtcling --show-config
qtcling --check
```

### 3. 設定ファイルを導入する

例:

```toml
qt_root = "/usr/local/qt/Qt/6.11.1/macos"
std = "c++2c"
modules = ["Core", "Gui", "Widgets"]

extra_include_dirs = []
extra_library_dirs = []
extra_framework_dirs = []
```

優先順:

```text
CLI option
> project qtcling.toml
> user ~/.qtcling/config.toml
> auto-detect
```

### 4. `qtcling-config` のような設定生成ツールを作る

shell script に複雑な検出ロジックを詰め込まず、小さな設定ツールに分離する。

想定 command:

```sh
qtcling-config --check
qtcling-config --show
qtcling-config --print-cling-args
qtcling-config --print-preload-header
```

`bin/qtcling` はこの出力を使って Cling を起動する。

## 実装済み

### 1. `QTCLING_QT_ROOT` による Qt root 指定

`QTCLING_QT_ROOT` で Qt installation root を指定できるようにした。

例:

```sh
QTCLING_QT_ROOT=/usr/local/qt/Qt/6.11.1/macos qtcling
```

未指定時は従来の固定 path を fallback として使う。

対象:

- `bin/iqtcling.macos`
- `bin/cqtcling.macos`
- `bin/iqtcling.linux`
- `bin/cqtcling.linux`
- `bin/iqtcling.win`
- `bin/cqtcling.win`

interactive 版の macOS / Linux では、指定された Qt root を反映した一時 preload header を起動時に生成する。

### 2. `QTCLING_STARTUP_FILE` による Qt GUI startup 自動読み込み

手動で `.L qtgui.cpp` を入力しなくても済むように、Cling interactive UI 側に startup file hook を追加した。

`QTCLING_STARTUP_FILE` が設定されている場合、`LineEditor` 作成後、つまり periodic callback 登録が有効になるタイミングで `.L <startup file>` 相当を実行する。

対象:

- `cling/lib/UserInterface/UserInterface.cpp`
- `bin/iqtcling.macos`
- `bin/iqtcling.linux`
- `bin/iqtcling.win`

macOS では以下を確認済み。

```sh
QTCLING_QT_ROOT=/usr/local/qt/Qt/6.11.1/macos qtcling
```

起動後、手動 `.L qtgui.cpp` なしで:

```cpp
qApp
```

が有効な `QApplication *` を返す。

### 3. macOS interactive の `--show-config` / `--check`

`bin/iqtcling.macos` に設定確認用 option を追加した。

```sh
qtcling --show-config
qtcling --check
```

`--show-config` は、実際に使われる Qt root、Qt framework path、QtCore / QtGui / QtWidgets の library path、Cling executable、dev root、startup file、主要な Cling args を表示する。

`--check` は、Qt root、QtCore / QtGui / QtWidgets、Cling executable、startup file の存在を確認し、成功時は exit 0、失敗時は exit 1 を返す。

macOS では以下の環境で確認済み。

```sh
QTCLING_QT_ROOT=/usr/local/qt/Qt/6.11.1/macos
QTCLING_STARTUP_FILE=/usr/local/src/cling/src/qtgui.cpp
```

`qtcling --check` が `qtcling check: OK` を返す。

現時点では macOS interactive 版のみの実装であり、Linux / Windows / compiled mode へは未展開。

### 4. macOS interactive の module list 生成

`bin/iqtcling.macos` で、固定の QtCore / QtGui / QtWidgets 列挙をやめ、module list から include path / framework library path / preload header を生成するようにした。

既定値:

```sh
QTCLING_MODULES="Core Gui Widgets"
```

追加 module を使う例:

```sh
QTCLING_MODULES="Core Gui Widgets Network" qtcling
```

`--show-config` では module list と、各 module の framework library path / include path を表示する。

確認済み:

```sh
QTCLING_MODULES="Core Gui Widgets Network" qtcling --show-config
QTCLING_MODULES="Core Gui Widgets Network" qtcling
```

REPL 上で以下が動作した。

```cpp
#include <QTcpSocket>
auto s = new QTcpSocket;
s
```

これにより、macOS interactive については Qt root と module list から `-I`, `-F`, preload を自動生成する流れが実用確認済みになった。

### 5. macOS interactive の REPL Cling root 指定

`bin/iqtcling.macos` で、REPL に使う Cling root を `QTCLING_CLING_ROOT` で指定できるようにした。

既定値:

```sh
QTCLING_CLING_ROOT=/usr/local/qtcling
```

開発中の build を使う例:

```sh
QTCLING_CLING_ROOT=/usr/local/src/cling/build qtcling
```

確認済み:

```sh
QTCLING_CLING_ROOT=/usr/local/src/cling/build qtcling
```

REPL 起動後:

```cpp
qApp
```

が有効な `QApplication *` を返す。

注意:

`/usr/local/qtcling/bin/cling` はまだ今回の `QTCLING_STARTUP_FILE` hook 入り build を install していないため、現時点では既定値のままだと startup file が読まれず、`qApp` は未定義になる。

## 目標とするユーザー体験

最終的には、ユーザーが以下だけで Qt を対話的に触れる状態を目指す。

```sh
qtcling
```

REPL では例えば次が動く。

```cpp
auto b = new QPushButton("Hello");
b->show();
```

ユーザーは `-I`, `-L`, `-F`, Qt framework path, preload header を意識しない。

## 優先度

1. macOS interactive を `qtcling-config` 相当へ分離するか判断
2. `qtcling.toml` 対応
3. 起動スクリプトの残りの絶対パス依存解消
4. Linux への展開
5. event loop 統合方式の再検討

## 当面の結論

qtcling は experimental release としては妥当。

ただし、次の段階に進めるには、Qt の path / module / preload 設定をユーザーから隠すことが重要である。

`-I`, `-L`, `-F` を直接扱わせる限り、qtcling は「Qt を簡単に対話実験できる環境」ではなく、「Cling に Qt の複雑な起動 option を足したもの」に見えてしまう。

そのため、設定自動化は qtcling の中核課題として扱う。
