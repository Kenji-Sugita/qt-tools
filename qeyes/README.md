# qeyes

`qeyes` は、マウスポインターの方向を追いかける 2 つの目をデスクトップ上に表示する小さな Qt Widgets アプリケーションです。

## 特徴

- 透明背景のフレームレスウィンドウ
- 既定では常に最前面で表示
- 虹彩と瞳孔がマウスポインターの位置を追従
- 左ドラッグで移動
- 左ダブルクリックで終了
- 右クリックで外観設定ダイアログを表示

## ビルド方法

```text
$ cmake -S . -B build
$ cmake --build build
```

## 起動方法

```text
$ ./build/qeyes
```

## 基本操作

- 左ドラッグ: ウィンドウ移動
- 左ダブルクリック: 終了
- 右クリック: 外観設定ダイアログを開く

## 関連文書

- docs の案内: `docs/README.md`
- 利用ガイド 1.0: `docs/guides/USER_GUIDE-1.0.md`
- 利用ガイド 1.1: `docs/guides/USER_GUIDE-1.1.md`
- 利用ガイド 1.2: `docs/guides/USER_GUIDE-1.2.md`
- リリースノート 1.0: `docs/releases/ReleaseNote-1.0.md`
- リリースノート 1.1: `docs/releases/ReleaseNote-1.1.md`
- リリースノート 1.2: `docs/releases/ReleaseNote-1.2.md`
- 変更履歴: `docs/history/RELEASE_HISTORY.md`
