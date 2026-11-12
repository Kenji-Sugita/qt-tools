# 他 GUI ライブラリーへの適用候補メモ

## 前提

現在の `Qt GUI REPL` の基本機能は Qt 固有ではない。
少なくとも現状の実装を見る限り、本質は次の 3 点にある。

- GUI アプリ側の event loop を少しずつ進められること
- その処理を Cling 側の periodic callback から呼べること
- GUI ライブラリー初期化を必要に応じて補えること

参考:

- [src/qtgui.cpp](/usr/local/src/cling/src/qtgui.cpp)
- [src/callback.cpp](/usr/local/src/cling/src/callback.cpp)

この観点で、Qt 以外の GUI / windowing / event loop 系ライブラリーへの展開候補を整理する。

## 候補一覧

### GTK / GLib

有力候補。

- `g_main_context_iteration()` で main loop を単発実行できる
- GTK 3 系では `gtk_main_iteration_do(FALSE)` もある
- Qt に近いイベントループ構造を持つ

評価:

- 移植しやすさ: 高
- 学習用途との相性: 高
- 最小 PoC 難易度: 中

コメント:

Qt の次の本命候補。今の構造を比較的そのまま当てやすい。

### FLTK

PoC 向きの有力候補。

- `Fl::check()` が `Fl::wait(0)` 相当
- non-blocking にイベントを進めやすい
- `Fl::add_check()` など補助 API もある

評価:

- 移植しやすさ: 高
- 学習用途との相性: 中
- 最小 PoC 難易度: 低

コメント:

最短で実証しやすい。Qt 非依存の基本仕組みが成立するかを見る最初の候補としてよい。

### wxWidgets

C++ GUI 学習用途も考えると有力候補。

- `wxEventLoopBase::Pending()`
- `Dispatch()`
- `DispatchTimeout()`
- `Yield()`

といった API がある。

評価:

- 移植しやすさ: 中
- 学習用途との相性: 高
- 最小 PoC 難易度: 中

コメント:

C++ GUI の教育用途にはかなり相性がよい。
ただし event loop の抽象度が少し高く、nested loop や再入性の扱いは慎重に見る必要がある。

### JUCE

音響 / GUI 系では有力。

- `MessageManager::runDispatchLoopUntil()` がある
- `callAsync()` など補助 API もある

評価:

- 移植しやすさ: 中
- 学習用途との相性: 中
- 最小 PoC 難易度: 中

コメント:

実装可能性はあるが、対象分野が少し特殊。

### GLFW

windowing / rendering 系としては簡単。

- `glfwPollEvents()`
- `glfwWaitEventsTimeout()`

でイベントを処理できる。

評価:

- 移植しやすさ: 高
- 学習用途との相性: 低
- 最小 PoC 難易度: 低

コメント:

event loop の統合自体は簡単。
ただし widget toolkit ではないため、Qt の代替 GUI 学習用途としては弱い。

### SDL

GLFW に近い位置づけ。

- `SDL_PollEvent()`
- `SDL_PumpEvents()`

でイベント処理ができる。

評価:

- 移植しやすさ: 高
- 学習用途との相性: 低
- 最小 PoC 難易度: 低

コメント:

ゲーム / マルチメディア寄り。
一般 GUI フレームワークへの展開候補としては優先度が少し下がる。

### Win32

ネイティブ基盤としては実現可能。

- `PeekMessage()`
- `DispatchMessage()`

で同様の構造を作れる。

評価:

- 移植しやすさ: 中
- 学習用途との相性: 低
- 最小 PoC 難易度: 中

コメント:

技術的には素直だが、汎用的なライブラリー展開先としては優先度が下がる。

### Cocoa / CFRunLoop

macOS 向けの下層候補。

- `CFRunLoopRunInMode(..., 0, ...)` で run loop を 1 パス進められる

評価:

- 移植しやすさ: 中
- 学習用途との相性: 低
- 最小 PoC 難易度: 中

コメント:

macOS 専用の下層基盤としてはよいが、汎用展開先としては狭い。

## 比較表

| 候補 | 移植しやすさ | 学習用途との相性 | 最小 PoC 難易度 | コメント |
|---|---|---:|---:|---|
| GTK / GLib | 高 | 高 | 中 | Qt の次の本命候補 |
| FLTK | 高 | 中 | 低 | 最短で PoC を作りやすい |
| wxWidgets | 中 | 高 | 中 | C++ GUI 学習用途と相性がよい |
| JUCE | 中 | 中 | 中 | 実装可能だが対象分野がやや特殊 |
| GLFW | 高 | 低 | 低 | event polling は簡単だが widget GUI 学習向きではない |
| SDL | 高 | 低 | 低 | ゲーム / マルチメディア寄り |
| Win32 | 中 | 低 | 中 | ネイティブ基盤としては可能 |
| Cocoa / CFRunLoop | 中 | 低 | 中 | macOS 専用の下層候補 |

## 結論

### 本命

- `GTK / GLib`

理由:

- event loop を単発実行する API が明確
- 今の Qt 版の構造を比較的そのまま適用しやすい
- GUI 学習用途にもつなげやすい

### 最速 PoC

- `FLTK`

理由:

- 実装が軽く済みそう
- non-blocking にイベントを進める API が分かりやすい
- Qt 非依存の基本設計が成立するかを最短で確認できる

### 学習用途重視の次点

- `wxWidgets`

理由:

- C++ GUI 学習との相性がよい
- Qt 初学者向けの延長で考えやすい
- ただし event loop の扱いは Qt / FLTK より慎重に見る必要がある

## 段階的な検証案

1. `FLTK` で最小 PoC を作る
2. `GTK / GLib` で本格候補を検証する
3. `wxWidgets` を教育用途の観点で評価する

この順にすると、

- 仕組み自体の一般性確認
- 実用的な展開先の検討
- 学習用途との接続

を段階的に見られる。

## 製品メッセージ上の含意

もし将来、製品メッセージを Qt 固有ではなく「特定 GUI ライブラリーに閉じない対話実行基盤」
として広げるなら、`GUI REPL runtime adapters` のような考え方が合う。

その場合の初期アダプター候補は次の順が現実的。

1. Qt
2. FLTK
3. GTK

## 参照元

- Qt 側の現状
  - [src/qtgui.cpp](/usr/local/src/cling/src/qtgui.cpp)
  - [src/callback.cpp](/usr/local/src/cling/src/callback.cpp)
- GLib main loop
  - https://docs.gtk.org/glib/main-loop.html
- GTK iteration
  - https://docs.gtk.org/gtk3/func.main_iteration_do.html
- wxWidgets event loop
  - https://docs.wxwidgets.org/3.2/classwx_event_loop_base.html
- FLTK `Fl`
  - https://www.fltk.org/doc-1.4/classFl.html
- JUCE `MessageManager`
  - https://docs.juce.com/master/classMessageManager.html
- GLFW input / event processing
  - https://www.glfw.org/docs/3.2/input_guide.html
- SDL `SDL_PollEvent`
  - https://wiki.libsdl.org/SDL3/SDL_PollEvent
- SDL `SDL_PumpEvents`
  - https://wiki.libsdl.org/SDL3/SDL_PumpEvents
- Win32 `PeekMessage`
  - https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-peekmessagea
- Win32 `DispatchMessage`
  - https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-dispatchmessage
- Apple `CFRunLoopRunInMode`
  - https://developer.apple.com/documentation/corefoundation/cfrunloopruninmode%28_%3A_%3A_%3A%29
