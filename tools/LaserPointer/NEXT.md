# NEXT

このファイルを読む、または更新する前に、`~/AGENTS.next.md` を読み、その指示に従うこと。
`~/AGENTS.next.md` が存在しない場合は、このファイルを整理・削除せず、利用者に確認すること。
このルールは、ユーザーから明示的な指示がない限り変更しない。

## 次にやること

- `build/LaserPointer.app` を実行し、実際のセミナー環境で最前面表示、ドラッグ移動、右クリックメニュー、macOS メニューバーアイコンの表示とメニュー、ポインター表示/非表示、Display/Motion/Trail/Preset/Reset/Help のサブメニュー、点滅、サイズ変更、色変更、虹色、追従、軌跡、波紋、リセット、Help 表示、`USER_GUIDE.pdf` の内容を確認する。
- Linux/Wayland では、レーザー本体への左ドラッグ、右クリック、ホイール操作を実機で確認する。ほかのアプリケーション上を含むカーソル追従は Wayland のグローバルポインター取得制約により通常の Qt Widgets 実装だけでは実現できないため、必要性がある場合はポータルまたはデスクトップ環境固有の連携を別途検討する（詳細は `backlog.md` と `DECISIONS.md`）。
- 必要なら見た目や操作名だけを微調整する。仕様追加は事前に提案して許可を得る。

## 未完了

- 実機での GUI 動作確認は未実施。ビルド確認は `cmake -S . -B build` と `cmake --build build` で成功済み。macOS Dock 非表示用 `LSUIElement` 追加後も `cmake --build build` は成功済み。Windows のタスクトレイ表示修正と exe アイコン埋め込み後は `cmake --build build --config Release` で成功済み。
- Linux の Qt 6.11.1 / Wayland セッションで `./build/LaserPointer` の起動を確認した。`qt.qpa.wayland.textinput` の `leave` 警告はテキスト入力フォーカスの診断であり、レーザー操作エラーではない。X11 フォールバックは `QT_QPA_PLATFORM=xcb ./build/LaserPointer` で試せるが、現環境では `xcb-cursor0`（または `libxcb-cursor0`）不足により xcb プラットフォームプラグインを初期化できなかった。
- macOS ビルドは `CMAKE_OSX_ARCHITECTURES` を `arm64;x86_64` に設定し、`build/LaserPointer.app/Contents/MacOS/LaserPointer` が universal binary になることを `lipo -info` で確認済み。
- 同種アプリ調査では Mouseposé、Presentify、PowerToys Mouse Utilities、ZoomIt、Epic Pen が近い。今回のアプリは「カーソル追従」ではなく「掴んで任意位置に置けるレーザー光点オーバーレイ」が差別化点。
- `genpdf --format book --output USER_GUIDE.pdf USER_GUIDE.md` で利用ガイド PDF 生成済み。
- DMG 作成用スクリプトは `../utils/create-dmg.sh` に移動済み。`path/to/App.app [path/to/App.dmg] [volume-name]` を受け取り、`macdeployqt`、Applications シンボリックリンク、Finder 背景、矢印、アイコン位置設定、UDZO 変換まで行う。矢印胴体が矢印頭から突き出ないように、背景描画では胴体線を矢印頭の付け根 `x=306` までで止めるよう修正済み。
- ソース配布用 `LaserPointer.zip` は作成済み。ZIP 内トップは `LaserPointer/`。含めるものは `CMakeLists.txt`, `USER_GUIDE.md`, `USER_GUIDE.pdf`, `assets/`, `include/`, `src/`。除外するものは `build/`, `.DS_Store`, `.dmg`, `.zip`, `NEXT.md`, `extension.md`, `.gitignore`。

## 触るファイル

- `src/LaserPointerWidget.cpp`: 表示、操作、リセット、レーザー本体描画、軌跡生成。
- `include/LaserPointerWidget.h`: Widget のメソッド宣言。
- `src/LaserPointerMenuBuilder.cpp`, `include/LaserPointerMenuBuilder.h`: 右クリックメニュー、macOS メニューバー / Windows タスクトレイメニュー、Help サブメニューの生成。
- `src/LaserPointerSettings.cpp`, `include/LaserPointerSettings.h`: `QSettings` のキー管理、保存、復元。
- `src/TrailSpotWidget.cpp`, `include/TrailSpotWidget.h`: 移動軌跡のフェードアウト表示。透明・入力透過の単一オーバーレイ QWidget に複数スポットを描画する。
- `CMakeLists.txt`: Qt 6 Widgets 構成、Qt リソース、macOS バンドルアイコン設定、Windows exe アイコンリソース、macOS universal binary 設定。
- `assets/laser-pointer.svg`, `assets/laser-pointer.png`, `assets/LaserPointer.icns`, `assets/LaserPointer.ico`, `assets/LaserPointer.rc`: アプリケーションアイコン。
- `assets/MacOSXBundleInfo.plist.in`: macOS バンドル用 Info.plist。`LSUIElement` を true にして Dock アイコンを非表示にする。
- `USER_GUIDE.md`, `USER_GUIDE.pdf`: 利用ガイド。PDF は `genpdf --format book --output USER_GUIDE.pdf USER_GUIDE.md` で生成。
- `laserpointer-cheatsheet.svg`, `laserpointer-cheatsheet.png`: 操作チートシート。`../whiteboard/whiteboard-app/whiteboard-cheatsheet.*` を参考にした 1920x1240 の下敷き風レイアウト。PNG は `rsvg-convert -w 1920 -h 1240 laserpointer-cheatsheet.svg -o laserpointer-cheatsheet.png` で生成。
- `extension.md`: 追加改善項目への回答メモ。
- `../utils/create-dmg.sh`: 汎用 DMG 作成スクリプト。LaserPointer 配下からは外に移動済みなので、編集には権限確認が必要になる場合がある。

## 注意

- `AGENTS.md` 方針: 指示にない機能追加や仕様変更は実装前に提案して許可を得る。リファクタは必要最小限。
- `CMakeLists.txt` を触る前に `~/AGENTS.cmake.md` を読む。Qt 6 の現代的 CMake を使い、`find_package(Qt6 ...)` 直後に `qt_standard_project_setup()`。
- 現在は Qt/C++ のみ。非アクティブ時の表示維持は `Qt::WA_MacAlwaysShowToolWindow` と `Qt::WA_ShowWithoutActivating`、最前面は `Qt::Tool | Qt::WindowStaysOnTopHint`。
- 操作: 左ドラッグで移動、右クリックメニューと macOS メニューバー / Windows タスクトレイのメニューは上部に状態サマリを表示し、Display/Motion/Trail/Preset/Reset/Help/Quit に整理。Display 内で Blink/Blink Interval/Larger/Smaller/Opacity/Shape/Rainbow/Color/Auto Fade/Hold H to Show、Motion 内で Follow Cursor/Smooth Follow/Keep On Screen、Trail 内で Trail/Trail Style/Trail Duration を選択する。Reset は Reset Display/Reset Motion/Reset Trail/Reset All。Blink Interval、Opacity、Shape、Trail Style、Trail Duration、Preset はサブメニューで直接選択できる。ホイールでサイズ変更、`B` 点滅、`A` 自動フェード、`E` 画面端制限、`F` マウスポインター追従、`G` 滑らか追従、`M` Hold H to Show、`H` ホールド表示、`O`/`P` 透明度、`S` 形状、`Space` 一時拡大、`V` 虹色、`C` 色、`T` 軌跡、`Y` 軌跡スタイル、`R` リセット、`+`/`-` サイズ、`,`/`.` ブリンク間隔、`]`/`[` 軌跡表示時間、`Esc`/`Q` 終了。
- macOS メニューバー / Windows タスクトレイ常駐は `QSystemTrayIcon` を使う。Linux ではトレイ機能を使わず従来どおり右クリックメニューで操作する。Windows では起動直後の `QSystemTrayIcon::isSystemTrayAvailable()` が false になる場合があるため、この判定で早期 return せずトレイアイコンを生成して `show()` する。`LaserPointerMenuBuilder::populate()` で右クリックメニューと常駐メニューを共通生成し、常駐メニュー側だけ `Show Pointer` / `Hide Pointer` を追加する。表示/非表示状態は保存せず、起動時は必ず表示する。常駐メニュー側は `aboutToShow` で `rebuildStatusMenu()` して現在状態を反映する。`QApplication::setQuitOnLastWindowClosed(false)` は macOS/Windows のみ適用する。
- macOS は `assets/MacOSXBundleInfo.plist.in` の `LSUIElement=true` で Dock に表示しない。Dock に出ないため、終了操作はメニューバー側の `Quit/終了` を残す。
- デフォルトは赤 `QColor(255, 24, 24)`、サイズ `96`、透明度 `100%`、形状 Glow、マウスポインター追従オフ、滑らか追従オフ、画面端制限オフ、自動フェードオフ、Hold H to Show オフ、点滅なし、ブリンク間隔 `900ms`、軌跡オフ、軌跡スタイル Glow、軌跡表示時間 `2500ms`。左クリック時は中心から白いリング状の波紋を短く表示する。`Space` 押下中は一時拡大する。ブリンクは透明度アニメーションで滑らかに点滅し、ドラッグ移動中は一時停止して不透明表示する。
- 右クリックメニューと Help サブメニューは `QLocale::system()` が日本語なら日本語表示、それ以外なら英語表示にする。Help は別ダイアログではなくサブメニュー内に操作一覧を表示する。
- 初回起動時はメニュー上部におすすめプリセット案内を表示する。プリセットは Standard/Noticeable/Follow/Subtle/Large Seminar の用途名で表示する。
- 色・追従状態・滑らか追従・画面端制限・自動フェード・Hold H to Show・透明度・形状・虹色状態・サイズ・点滅状態・ブリンク間隔・軌跡状態・軌跡スタイル・軌跡表示時間は `LaserPointerSettings` 経由で `QSettings` に保存し、起動時に復元する。位置は保存対象外で、起動時は従来どおり中央表示。
- 虹色のときは `Color...` は無効で、`C` キーでも色変更ダイアログを開かない。
- 軌跡表示時間は `500ms` から `6000ms`、`500ms` 刻み。ドラッグ中は前回の軌跡位置から現在位置までを補間し、単一の透明オーバーレイ `TrailSpotWidget` に小さめ・薄めのスポットとして追加する。負荷を抑えるため、1 回の移動イベントで最大 10 個まで補間する。軌跡オンで大サイズ、または虹色+滑らか追従の重い組み合わせでは軌跡表示時間を最大 `3000ms` に制限する。
- `build/` と `.DS_Store` は `.gitignore` 済み。
- ルート直下の `LaserPointer.dmg` と `LaserPointer.zip` は未追跡で存在する。コミット対象に含めるかは確認してから判断する。
