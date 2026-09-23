# 実装工程設計

## 目的

`qeyes` の詳細設計書をもとに、ビルド可能なアプリ実装へ落とし込む工程を定義する。

## 入力

- `PRODUCT_DESIGN.md`
- `UI_DESIGN.md`
- `RENDERING_DESIGN.md`
- `AGENTS.md`

## 出力

- `CMakeLists.txt`
- `main.cpp`
- `qeyes.rc`
- `assets/`

## 役割

- `PRODUCT_DESIGN.md` の機能責務を実装対象へ変換する。
- `UI_DESIGN.md` の操作仕様とダイアログ仕様を Qt Widgets のコードへ反映する。
- `RENDERING_DESIGN.md` の描画仕様を `QPainter` ベースの描画処理へ反映する。
- `AGENTS.md` の実装方針、リソース方針、README 方針を実ファイルへ反映する。

## 実装対象

- Qt Widgets アプリ本体
- ビルド定義
- Windows 用リソース定義
- アイコンなどの実リソース

## 対象外

- リリースノートや利用ガイドの版更新
- 開発基盤文書そのものの生成
- 配布パッケージの作成手順

## 完了条件

- `CMakeLists.txt` が存在し、アプリのビルド対象が定義されている。
- `main.cpp` が存在し、`qeyes` の基本動作が実装されている。
- 必要なリソース参照がソースとビルド定義へ反映されている。
- `qeyes.rc` が必要なプラットフォーム向け設定を持っている。

## 将来拡張

次のような構成が必要になった場合は、この工程の下で文書を分割する。

- 複数ソースファイルへの分割
- テストコードの追加
- 配布設定や CI 設定の生成
