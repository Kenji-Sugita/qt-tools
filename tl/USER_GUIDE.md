# tl 利用者ガイド

## 概要

`tl` は、日本語と英語を自動判定して相互翻訳するコマンドラインツールです。

- 入力が日本語なら英語へ翻訳します
- 入力が英語なら日本語へ翻訳します
- 現在の翻訳エンジンは Google Cloud Translation です
- 対応言語は `ja` と `en` のみです

## 事前準備

利用前に次を満たしてください。

- Qt 6 の開発環境が使えること
- Google Cloud Translation API を有効化済みであること
- Google Cloud の Billing Account を設定済みであること
- Service Account の JSON キーを取得済みであること
- `GOOGLE_APPLICATION_CREDENTIALS` 環境変数を設定済みであること

Google Cloud 側の準備は GOOGLE_FREE_SETUP.md を参照してください。

環境変数の設定例:

```bash
export GOOGLE_APPLICATION_CREDENTIALS="$HOME/.config/tl/google-translation-service-account.json"
```

## ビルド方法

リポジトリ直下で次を実行します。

```bash
cmake -S . -B build
cmake --build build
```

生成される実行ファイル:

```bash
./build/tl
```

## 基本的な使い方

usage:

```bash
tl [text]
tl --file <path>
tl --help
tl --version
```

### 1. 位置引数で翻訳する

```bash
./build/tl "今日は会議があります。"
```

```bash
./build/tl "Please check this file."
```

### 2. ファイル内容を翻訳する

```bash
./build/tl --file input.txt
```

`--file` はファイル全体を読み込み、1 つのテキストとして翻訳します。

### 3. 標準入力から翻訳する

```bash
echo "この問題はまだ解決していません。" | ./build/tl
```

```bash
cat memo.txt | ./build/tl
```

```bash
./build/tl
今日は会議があります。
資料はあとで送ります。
^D
```

## 入力ルール

入力ソースは次の優先順位で 1 つだけ使われます。

1. 位置引数 `text`
2. `--file <path>`
3. 標準入力

次の使い方はエラーです。

- 位置引数を 2 個以上与える
- 位置引数と `--file` を同時に指定する
- `--file` 指定時に標準入力も与える
- 空入力で実行する

## 出力

- 成功時は翻訳結果のみを標準出力へ出します
- 追加の説明文は出しません
- エラー時は標準エラー出力へ `tl: ...` の形式でメッセージを出します

## よく使うオプション

- `--help`: ヘルプを表示します
- `--version`: バージョンを表示します
- `--file <path>`: ファイルを入力に使います

## 注意事項

- 自動判定結果が `ja` または `en` 以外の場合はエラーになります
- 改行を含む入力は扱えますが、出力時の改行保持は保証しません
- 末尾の改行は翻訳前に取り除かれます
- 認証情報ファイルが読めない場合は実行できません

## 動作確認

ヘルプ表示:

```bash
./build/tl --help
```

バージョン表示:

```bash
./build/tl --version
```

簡易テスト:

```bash
ctest --test-dir build --output-on-failure
```

このテストは主に CLI の引数処理と空入力エラーを確認します。Google Cloud への実通信確認は含みません。

## トラブルシュート

`tl: GOOGLE_APPLICATION_CREDENTIALS is not set`

- 環境変数 `GOOGLE_APPLICATION_CREDENTIALS` を設定してください

`tl: cannot read credentials file '...'`

- JSON キーファイルのパスが正しいか確認してください
- 実行ユーザーに読み取り権限があるか確認してください

`tl: openssl executable not found at /opt/local/bin/openssl`

- `openssl` をインストールし、`/opt/local/bin/openssl` に配置してください

`tl: API request failed`

- Google Cloud Translation API が有効か確認してください
- 認証情報のプロジェクトに対して Translation API を利用できるか確認してください
- ネットワーク接続を確認してください

`tl: detected language '...' is not supported`

- 入力が日本語または英語として判定される内容か確認してください

## 関連資料

- 仕様: SPEC.md
- Google Cloud 設定: GOOGLE_FREE_SETUP.md
