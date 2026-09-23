# Vim Markdown Snippet Guide

このファイルは、`USER_GUIDE.md` にある `genpdf` 用 Markdown 記法を、Vim 上で `neosnippet` により展開できるようにしたときの手順と使い方をまとめたものです。

# 動作した構成

- Vim
- `NeoBundle`
- `Shougo/neosnippet.vim`
- `Shougo/neosnippet-snippets`
- GitHub Copilot

今回の構成では、役割を次のように分けています。

- `Ctrl-k`: `neosnippet` の展開
- `Ctrl-j`: Copilot の提案受け入れ

`Ctrl-j` は Copilot と衝突するため、`neosnippet` 側には使いません。

# .vimrc の設定

`NeoBundle` 定義の中に次を入れます。

```vim
NeoBundle 'Shougo/neosnippet.vim'
NeoBundle 'Shougo/neosnippet-snippets'

let g:neosnippet#snippets_directory = expand('~/.vim/my-snippets')

imap <C-k>     <Plug>(neosnippet_expand_or_jump)
smap <C-k>     <Plug>(neosnippet_expand_or_jump)
xmap <C-k>     <Plug>(neosnippet_expand_target)

if has('conceal')
    set conceallevel=2 concealcursor=niv
endif
```

Copilot 側は `Ctrl-j` をそのまま使います。

```vim
imap <silent><script><expr> <C-J> copilot#Accept("\<CR>")
```

# インストール手順

1. Vim でプラグインをインストールします。

```vim
:NeoBundleInstall
```

2. スニペット置き場を作ります。

```sh
mkdir -p ~/.vim/my-snippets
```

3. このリポジトリの Markdown 用スニペットを配置します。

```sh
cp /Users/sugita/prj/doc/templates/markdown/snippets/vim/markdown.snip ~/.vim/my-snippets/markdown.snip
```

4. Vim で設定を再読込します。

```vim
:source ~/.vimrc
```

5. スニペットキャッシュを作ります。

```vim
:NeoSnippetMakeCache markdown
```

# 動作確認

Vim で次を確認します。

```vim
:echo g:neosnippet#snippets_directory
:verbose imap <C-k>
:set filetype?
```

期待値:

- `g:neosnippet#snippets_directory` が `/Users/sugita/.vim/my-snippets`
- `<C-k>` が `<Plug>(neosnippet_expand_or_jump)` に割り当てられている
- Markdown 編集中は `filetype=markdown`

# 使い方

1. `.md` ファイルを開く
2. スニペットのトリガーを入力する
3. `Ctrl-k` で展開する
4. プレースホルダ間の移動も `Ctrl-k` で行う
5. Copilot の提案を採用したいときは `Ctrl-j` を使う

例:

1. `gcallout` と入力
2. `Ctrl-k` を押す
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

実運用では、先にスニペットで骨格を出して、その後の本文や値だけ Copilot に補完させるのが安定します。

例:

- `gcallout` を展開して本文を Copilot に書かせる
- `gcover` を展開して `caption` や `alt` を Copilot に補完させる
- `gmermaid` を展開して図のノードや矢印を Copilot に続けさせる

# 関連ファイル

- [USER_GUIDE.md](/Users/sugita/prj/doc/templates/markdown/USER_GUIDE.md:1)
- [snippets/vim/markdown.snip](/Users/sugita/prj/doc/templates/markdown/snippets/vim/markdown.snip:1)
- [SNIPPETS.md](/Users/sugita/prj/doc/templates/markdown/SNIPPETS.md:1)
