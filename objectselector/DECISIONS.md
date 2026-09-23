# DECISIONS

## 採用済み

### 1. バージョンと配布物の管理

- root の `VERSION` を唯一の版数源とし、CMake と配布物はこの値を使用する。
- `release.sh` は一時 staging directory から `release/objectselector-<VERSION>-source.zip` を作成し、ZIP の整合性を検証する。
- source code release ZIP と書籍形式の `USER_GUIDE.pdf` は Git 管理し、配布先 `/Users/sugita/Sites/tools/objectselector` にも配置する。
- ビルド成果物、Git 管理情報、作業管理ファイルは source code release ZIP に含めない。

## 不採用

### 1. 案の名前

- 検討した案を書く
- 採用しなかった理由を書く
- 再検討する条件を書く

## まだ決めていないこと

### 1. 論点名

- 実装作業ではなく、判断が必要な未決事項を書く
- どこを見て決めるかを書く
- 着手時期だけを保留している作業は `backlog.md` に書く

## 判断基準

- 何を優先して判断するかを書く
- 例: 既存 URL を壊さない
- 例: ホームを肥大化させない

## 継続する仕様・運用制約

- 今後の変更でも維持する仕様を書く
- 作業時に守る運用制約と、その理由を書く
