---
genpdf:
  Format: book
  Title: dtree 利用ガイド
  Subtitle: 1.0.1 版
  Author: (株) SRA
  Date: 2026-11-08
---

# 1. 概要

`dtree` は、ディレクトリーの内容をツリー形式で表示する小さなコマンドラインツールです。

指定したディレクトリーを再帰的にたどり、ファイルとサブディレクトリーを見やすい枝線付きで表示します。Unicode の線を使った表示と、ASCII のみを使った表示を切り替えられ、必要に応じて隠しファイルや隠しディレクトリーも表示できます。各階層の項目は名前順または更新時刻順で表示でき、特定の名前の項目を除外したり、ファイル数の多いディレクトリーでは表示するファイル数を制限したりできます。ルートは既定でベース名だけを表示し、必要ならフルパス表示に切り替えられます。必要に応じて各項目の種類、更新時刻、サイズ、権限、所有者を表示できます。最後に件数を表示することもできます。

現在の `dtree` は、次のような用途に向いています。

- ディレクトリー構造をすばやく確認する
- 配布物やサンプルの構成を文章に載せる
- Unicode が使えない環境向けに ASCII 表示へ切り替える

# 2. 基本的な考え方

`dtree` は、指定した起点ディレクトリーを深さ優先でたどって表示します。

- 先頭に起点ディレクトリー自身を表示します
- 先頭の起点ディレクトリーは、既定ではベース名だけを表示します
- `-f`, `--full-root` を付けると、先頭の起点ディレクトリーをフルパスで表示します
- その下に、子要素を上から順に表示します
- 各階層では、通常の項目を既定で名前順に表示します
- `--sort date` を付けると、通常の項目を更新時刻の新しい順で表示します
- `--all` を付けた場合は、通常の項目を表示したあとに、隠し項目を表示します
- `--all --sort date` を付けた場合も、通常の項目と隠し項目は別々に更新時刻順で表示します
- `--exclude NAME` を付けると、各階層で名前が `NAME` と一致する項目を除外します
- `--max-files N` を付けると、各ディレクトリー内で表示するファイル数を最大 `N` 件に制限します
- ディレクトリーは `--max-files` の上限に含めず、常に表示します
- ディレクトリー名の末尾には `/` を付けます
- `-v` を付けると、各項目の表示情報を増やします
- 既定では隠しファイルや隠しディレクトリーは表示しません
- `--all` を付けると隠しファイルや隠しディレクトリーも追加で表示します

引数を省略した場合は、現在のディレクトリー (`.`) を表示します。

# 3. インストール

配布用 zip を展開して使います。

```text
$ unzip dtree-1.0.1-source.zip
$ cd dtree-1.0.1
$ chmod +x dtree
```

そのままカレントディレクトリーから起動できます。

```text
$ ./dtree
```

任意のディレクトリーから使いたい場合は、`PATH` の通った場所へ配置します。

```text
$ cp dtree ~/bin/
$ cp VERSION ~/bin/
$ chmod +x ~/bin/dtree
```

Windows では shebang 実行に依存せず、Git Bash や WSL など Bash が使える環境で起動します。

```text
$ bash ./dtree
```

# 4. 起動

通常は次のように起動します。

```text
$ dtree
dtree/
└─ dtree
```

表示したいディレクトリーを指定することもできます。

```text
$ dtree src
src/
├─ lib/
└─ main.sh
```

ヘルプは次のように表示します。

```text
$ dtree --help
```

バージョンは次のように表示します。

```text
$ dtree --version
dtree 1.0.1
```

スクリプトを直接 Bash で起動しても構いません。

```text
$ bash ./dtree .
```

# 5. 最初に試す例

## 5.1 現在のディレクトリーを表示する

```text
$ dtree
dtree/
├─ docs/
├─ dtree
└─ samples/
```

引数を付けない場合は、現在のディレクトリーを起点に表示します。

## 5.2 指定したディレクトリーを表示する

```text
$ dtree project
project/
├─ bin/
├─ lib/
└─ README.md
```

ディレクトリー名を 1 つ指定すると、そのディレクトリーを起点に表示します。

# 6. 基本オプション

`dtree` では、表示に使う枝線の種類を切り替えられます。

## 6.1 ヘルプ

```text
$ dtree --help
```

- `-h`, `--help` で使い方を表示します

## 6.2 線種の指定

```text
$ dtree --lines unicode
$ dtree --lines ascii
$ dtree --unicode
$ dtree --ascii
```

- `--lines unicode` は Unicode の枝線を使います
- `--lines ascii` は `|`, `+`, `-` を使った ASCII 表示にします
- `--unicode` は `--lines unicode` と同じです
- `--ascii` は `--lines ascii` と同じです

## 6.3 隠しファイルの表示

```text
$ dtree --all
$ dtree -a project
```

- `-a`, `--all` を付けると、`.` で始まる名前のファイルやディレクトリーも表示します
- 通常の項目は既定で名前順に表示します
- `--sort date` を付けると、通常の項目も隠し項目もそれぞれ更新時刻の新しい順で表示します
- 隠し項目は通常の項目のあとに続けて表示します

## 6.4 件数表示

```text
$ dtree -s project
project/
├─ bin/
│  └─ run.sh
├─ lib/
│  ├─ core.sh
│  └─ util.sh
└─ README.md

2 directories, 4 files
```

- `-s`, `--summary` を付けると、表示したディレクトリー数とファイル数を最後に表示します
- 件数には起点ディレクトリー自身は含みません

## 6.5 並び順の指定

```text
$ dtree --sort name project
$ dtree --sort date project
```

- 既定では `--sort name` と同じく名前順で表示します
- `--sort date` を付けると、各階層の項目を更新時刻の新しい順で表示します
- 更新時刻が同じ項目は、名前順で表示します
- `--all` と併用した場合も、通常の項目を先に表示し、そのあとに隠し項目を表示します

## 6.6 名前による除外

```text
$ dtree --exclude build --exclude output project
project/
├─ README.md
├─ docs/
│  └─ guide.md
└─ src/
   └─ main.sh
```

- `--exclude NAME` を付けると、各階層で名前が `NAME` と完全一致する項目を除外します
- `--exclude build` は `build` を除外しますが、`build-old` や `my-build` は除外しません
- `--exclude` は繰り返し指定できます
- 除外したディレクトリーの中はたどりません
- `--all` と併用した場合も、除外した名前の項目は表示しません
- 件数表示には、除外された項目を含めません
- `--max-files` は、除外後のファイルに対して適用します

## 6.7 ファイル数の上限

```text
$ dtree --max-files 1 project
project/
├─ bin/
│  ├─ run.sh
│  └─ ...
├─ lib/
│  ├─ core.sh
│  └─ ...
├─ README.md
└─ ...
```

- `--max-files N` を付けると、各ディレクトリー内で表示するファイル数を最大 `N` 件に制限します
- ディレクトリーは上限に含めず、上限に関わらず表示します
- 省略されたファイルがある場合は、`...` を表示します
- `--max-files 0` を指定すると、ファイル名を表示せず、省略を示す `...` とディレクトリーを表示します
- `...` は件数表示のファイル数には含めません
- シンボリックリンクはリンクとして扱い、ファイル数の上限に含めます

## 6.8 ルートのフルパス表示

```text
$ dtree -f /tmp/project
/tmp/project/
├─ bin/
└─ README.md
```

- 既定ではルートをベース名だけで表示します
- `-f`, `--full-root` を付けるとルートをフルパスで表示します

## 6.9 詳細表示

```text
$ dtree -v project
project/ [dir]
├─ bin/ [dir]
├─ current [link -> releases/current]
└─ README.md [file]
```

```text
$ dtree -vv project
project/ [dir, 2026-05-20 10:15]
├─ bin/ [dir, 2026-05-20 10:14]
├─ current [link -> releases/current, 16B, 2026-05-20 10:13]
└─ README.md [file, 842B, 2026-05-19 18:30]
```

```text
$ dtree -vvv project
project/ [dir, drwxr-xr-x, sugita, 2026-05-20 10:15]
├─ bin/ [dir, drwxr-xr-x, sugita, 2026-05-20 10:14]
├─ current [link -> releases/current, lrwxr-xr-x, sugita, 16B, 2026-05-20 10:13]
└─ README.md [file, -rw-r--r--, sugita, 842B, 2026-05-19 18:30]
```

- `-v`, `--verbose` を付けると、各項目の種類を表示します
- `-vv` を付けると、ディレクトリーには更新時刻を、ファイルにはサイズと更新時刻を表示します
- `-vvv` を付けると、権限と所有者も表示します
- シンボリックリンクは `link -> リンク先` と表示します
- `-v` は `-vv`, `-vvv` のように繰り返せます
- `-avv` のように、ほかの短いオプションと組み合わせて指定できます

## 6.10 バージョン表示

```text
$ dtree --version
dtree 1.0.1
```

- `--version` はバージョンを表示します
- ルートの `VERSION` ファイルを `dtree` のバージョンの単一の参照元とします
- `-v` は詳細表示に使うため、バージョン表示には使いません

# 7. 表示形式の違い

Unicode 表示では、見た目の分かりやすい枝線を使います。

```text
$ dtree .
project/
├─ docs/
├─ dtree
└─ samples/
```

ASCII 表示では、文字化けしやすい環境でも扱いやすい表記になります。

```text
$ dtree --ascii .
project/
+- docs/
+- dtree
+- samples/
```

端末やログ保存先で Unicode 罫線が扱いづらい場合は、ASCII 表示を使います。

# 8. 隠しファイルを含める表示

隠しファイルや隠しディレクトリーも確認したい場合は、`--all` を使います。

```text
$ dtree --all project
project/
├─ bin/
├─ README.md
├─ .gitignore
└─ .tools/
```

設定ファイルや隠しディレクトリーを含めて構成を確認したい場合に便利です。

# 9. 利用上の注意

`dtree` の引数は、起点ディレクトリーを 1 つだけ受け付けます。

```text
$ dtree dir1 dir2
```

このように複数指定すると、エラーになります。

また、存在しないパスやディレクトリーではないパスを指定した場合も、使い方を表示して終了します。

```text
$ dtree not-found
```

既定では、隠しファイルや隠しディレクトリーは表示対象に含まれません。必要な場合は `--all` を指定します。
