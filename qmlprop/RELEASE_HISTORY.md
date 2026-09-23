# Release History

## 2026-04-14

### 更新

- Qt 5 向けコードを削除し、Qt 6 向けの構成へ整理した
- qmake ベースの `.pro` から CMake ベースのビルドへ移行した
- private header 依存を削除し、公開 API ベースの実装へ整理した
- 巨大な固定 QML テンプレートを廃止し、候補モジュールを順番に試す方式へ変更した
- `main.cpp` に集中していた責務を分割し、型調査・モジュール探索・表示処理を整理した

### 表示機能

- 型レポートに、所属モジュール / 型フラグ / デフォルトプロパティー / 宣言元クラスの表示を追加した
- プロパティー属性として `readonly` / `final` / `constant` / `required` / `bindable` / `default` / `deferred` / `list` / `object` / `override` / `inherited` を表示できるようにした
- `--show-attached` で Attached properties を表示できるようにした
- `--show-declared-in` で各メンバーの宣言元 C++ クラスを表示できるようにした
- `--all-matches` で同名型をモジュールごとに順番に確認できるようにした

### 一覧と操作性

- `--list-types` に、生成可否や singleton / attached provider などのフラグ表示を追加した
- `--list-modules` で実行環境から見えている QML モジュール URI を一覧表示できるようにした
- `--module-index <module>` でモジュール単位の型索引を 1 行 1 型で表示できるようにした
- 標準出力がターミナル接続時は既定でページャー経由にし、`--no-pager` で無効化できるようにした

### 文書

- `README.md` を現在の構成と主要機能に合わせて更新した
- `USER_GUIDE.md` を追加し、使い方と出力の見方を整理した
- `ReleaseNote.md` と `ReleaseNote.txt` を追加した
