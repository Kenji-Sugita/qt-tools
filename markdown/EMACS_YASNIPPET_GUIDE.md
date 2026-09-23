# Emacs Markdown Snippet Guide

このファイルは、`USER_GUIDE.md` にある `genpdf` 用 Markdown 記法を、Emacs 上で `yasnippet` により展開できるようにしたときの手順と使い方をまとめたものです。

# 想定構成

- Emacs
- `markdown-mode`
- `yasnippet`
- GitHub Copilot または任意の補完系パッケージ

今回の構成では、役割を次のように分けます。

- `TAB`: `yasnippet` の展開とプレースホルダ移動
- Copilot などの補完系: 本文や値の補完

補完系パッケージと `TAB` が衝突する場合は、`yasnippet` の展開キーを別のキーに割り当ててください。

# init.el の設定

`use-package` を使っている場合は、`init.el` に次のように設定します。

```elisp
(use-package markdown-mode
  :mode ("\\.md\\'" . markdown-mode))

(use-package yasnippet
  :config
  (setq yas-snippet-dirs
        '("~/.emacs.d/snippets"))
  (yas-global-mode 1))
```

`use-package` を使わない場合は、次のように設定します。

```elisp
(require 'markdown-mode)
(require 'yasnippet)

(setq yas-snippet-dirs
      '("~/.emacs.d/snippets"))

(yas-global-mode 1)
```

`TAB` 以外のキーで明示的に展開したい場合は、たとえば `C-c y` に割り当てます。

```elisp
(with-eval-after-load 'yasnippet
  (define-key yas-minor-mode-map (kbd "C-c y") #'yas-expand))
```

# インストール手順

1. Emacs で `markdown-mode` と `yasnippet` をインストールします。

`M-x package-install` を使う場合:

```text
M-x package-install RET markdown-mode RET
M-x package-install RET yasnippet RET
```

2. スニペット置き場を作ります。

```sh
mkdir -p ~/.emacs.d/snippets
```

3. このリポジトリの Markdown 用スニペットを配置します。

```sh
cp -R /Users/sugita/prj/doc/templates/markdown/snippets/emacs/markdown-mode \
      ~/.emacs.d/snippets/
```

配置後の例:

```text
~/.emacs.d/snippets/
└── markdown-mode/
    ├── gfront
    ├── gcallout
    ├── gmermaid
    └── ...
```

4. Emacs で設定を再読込します。

```text
M-x eval-buffer
```

または Emacs を再起動します。

5. スニペットを再読込します。

```text
M-x yas-reload-all
```

# 動作確認

Emacs で次を確認します。

```text
C-h v yas-snippet-dirs
M-x yas-describe-tables
M-x markdown-mode
```

期待値:

- `yas-snippet-dirs` に `~/.emacs.d/snippets` が含まれている
- `yas-describe-tables` で `markdown-mode` のスニペットが表示される
- Markdown 編集中は `major-mode` が `markdown-mode`

`major-mode` は次で確認できます。

```text
C-h v major-mode
```

# 使い方

1. `.md` ファイルを開く
2. `markdown-mode` になっていることを確認する
3. スニペットのトリガーを入力する
4. `TAB` で展開する
5. プレースホルダ間も `TAB` で移動する
6. 本文や値を Copilot などで補完したい場合は、展開後に補完を使う

例:

1. `gcallout` と入力
2. `TAB` を押す
3. 次のように展開される

```md
!callout{type=warning title=注意}
  この操作は元に戻せません。
```

# 使える補完

Markdown で次のトリガーが使えます。

- `gfront`: `genpdf` 用 front matter
- `gpagebreak`: 改ページ
- `gfontsize`: `!font-size{...}`
- `gtoc`: `!toc`
- `gpause`: `!pause`
- `gincremental`: `!incremental-list`
- `gcover`: `!cover-image{...}`
- `gcallout`: `!callout{...}`
- `gfitcode`: `!fit-code` と fenced code block
- `gcolumns`: `!columns` レイアウト
- `gcolumnsdiv`: 区切り線付き `!columns`
- `gmermaid`: Mermaid ブロック
- `gmermaidopt`: `!mermaid{align=... scale=...}` 付き Mermaid
- `ginclude`: `!include-code{...}`
- `gincludelines`: `lines=` 付き `!include-code`
- `gnotes`: 1 行 `!notes`
- `gnotesblock`: 複数行 `!notes`
- `gfooter`: `!footer{...}`
- `gcopyright`: `copyright` front matter
- `gpagenums`: `page_numbers` front matter

# Copilot との併用

実運用では、先にスニペットで骨格を出して、その後の本文や値だけ Copilot などの補完に任せるのが安定します。

例:

- `gcallout` を展開して本文を補完させる
- `gcover` を展開して `caption` や `alt` を補完させる
- `gmermaid` を展開して図のノードや矢印を続けさせる
- `gfront` を展開して文書ごとの `title` や `author` を補完させる

# トラブルシュート

## `TAB` で展開されない

次を確認します。

```text
C-h v yas-minor-mode
C-h v yas-snippet-dirs
M-x yas-describe-tables
```

- `yas-minor-mode` が有効か
- `yas-snippet-dirs` が正しいか
- `markdown-mode` のスニペットが読み込まれているか

必要なら再読込します。

```text
M-x yas-reload-all
```

## Markdown ファイルでスニペットが出ない

`major-mode` が `markdown-mode` になっているか確認します。

```text
C-h v major-mode
```

`markdown-mode` でなければ、手動で切り替えます。

```text
M-x markdown-mode
```

## Copilot や補完系とキーが衝突する

`TAB` を補完系が使っている場合は、`yas-expand` を別キーに割り当てます。

```elisp
(with-eval-after-load 'yasnippet
  (define-key yas-minor-mode-map (kbd "C-c y") #'yas-expand))
```

この場合は、トリガーを入力して `C-c y` で展開します。

# 関連ファイル

- [USER_GUIDE.md](/Users/sugita/prj/doc/templates/markdown/USER_GUIDE.md:1)
- [snippets/emacs/markdown-mode](/Users/sugita/prj/doc/templates/markdown/snippets/emacs/markdown-mode)
- [SNIPPETS.md](/Users/sugita/prj/doc/templates/markdown/SNIPPETS.md:1)
- [VIM_NEOSNIPPET_GUIDE.md](/Users/sugita/prj/doc/templates/markdown/VIM_NEOSNIPPET_GUIDE.md:1)
