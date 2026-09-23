# dateorder

`dateorder` は、コマンドラインで指定したファイル列が更新日時順に並んでいるかを判定する小さなユーティリティです。

実体コマンドは `younger` で、`older` はそのシンボリックリンクです。起動名に応じて判定方向が切り替わります。

## 含まれるコマンド

- `younger`
  引数のファイル列が新しい順に並んでいるかを判定します。
- `older`
  引数のファイル列が古い順に並んでいるかを判定します。

比較には各ファイルの更新時刻 (`mtime`) を使います。

## 使い方

```text
younger FILE FILE...
older   FILE FILE...
```

- `younger a b c`
  `a.mtime >= b.mtime >= c.mtime` なら成功です。
- `older a b c`
  `a.mtime <= b.mtime <= c.mtime` なら成功です。

同じ更新時刻は順序違反とみなしません。

## 終了コード

- `0`
  指定順序を満たしています。
- `1`
  指定順序を満たしていません。
- `2`
  使用方法エラーまたは実行時エラーです。

`2` になる例:

- 引数が 2 個未満
- 存在しないファイルを指定した
- `stat` に失敗した

## 出力

成功時は何も出力しません。

順序違反の例:

```text
younger: order violation between 'old.txt' and 'new.txt'
```

存在しないファイルの例:

```text
older: cannot stat 'missing.txt'
```

## 例

```bash
# 新しい順かを判定する
younger newest.log middle.log oldest.log

# 古い順かを判定する
older  20240101.txt 20240201.txt 20240301.txt

# シェルの条件分岐で使う
if younger *.log; then
  echo OK
else
  echo NG
fi
```

## ビルド方法

```bash
cmake -S . -B build
cmake --build build
```

ビルドすると `build/younger` が生成され、あわせて `build/older` のシンボリックリンクも作成されます。

## インストール方法

システムへ配置せずに使う場合は、ビルド後の実行ファイルをそのまま呼び出せます。

```bash
./build/younger FILE FILE...
./build/older   FILE FILE...
```

`PATH` の通ったディレクトリーに配置する場合は、`younger` をコピーし、`older` のシンボリックリンクを作成します。

例:

```bash
install -m 0755 build/younger /usr/local/bin/younger
ln -sf younger /usr/local/bin/older
```

`/usr/local/bin` への書き込みに管理者権限が必要な環境では `sudo` を付けて実行してください。

インストール後の確認例:

```bash
younger --help
older --help
```

ただし、現行実装では `--help` オプションはありません。上のコマンドは usage 表示の確認用で、終了コードは `2` になります。
