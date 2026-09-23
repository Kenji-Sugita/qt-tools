# Editor Snippets

`USER_GUIDE.md` にある `genpdf` 用の独自記法を、Markdown 編集時にすぐ展開できるようにしたスニペット集です。

- Vim / Neovim: [snippets/vim/markdown.snippets](/Users/sugita/prj/doc/templates/markdown/snippets/vim/markdown.snippets:1)
- Vim / NeoBundle + neosnippet: [snippets/vim/markdown.snip](/Users/sugita/prj/doc/templates/markdown/snippets/vim/markdown.snip:1)
- Emacs yasnippet: `snippets/emacs/markdown-mode/`

# 含めたもの

- front matter
- `version`
- 改ページ
- `!font-size`
- `!toc`
- `!pause`
- `!incremental-list`
- `!cover-image`
- `!svg`
- `!callout`
- `!fit-code`
- `!columns`
- `!mermaid`
- `!dot`
- `!include-code`
- `!notes`
- `!footer`
- `copyright`
- `page_numbers`

# Vim / Neovim

前提:

- `UltiSnips` を使う

配置:

```sh
mkdir -p ~/.vim/UltiSnips
cp snippets/vim/markdown.snippets ~/.vim/UltiSnips/markdown.snippets
```

Neovim の場合:

```sh
mkdir -p ~/.config/nvim/UltiSnips
cp snippets/vim/markdown.snippets ~/.config/nvim/UltiSnips/markdown.snippets
```

使い方:

1. Markdown ファイルを開く
2. たとえば `gcallout` と入力する
3. 展開キーを押す

よくある展開キー:

- `Tab`
- `Ctrl-j`

使っている設定次第で違うので、反応しない場合は `UltiSnipsExpandTrigger` の設定を確認してください。

主なトリガー:

- `gfront`
- `gpagebreak`
- `gfontsize`
- `gtoc`
- `gpause`
- `gincremental`
- `gcover`
- `gsvg`
- `gcallout`
- `gfitcode`
- `gcolumns`
- `gcolumnsdiv`
- `gmermaid`
- `gmermaidopt`
- `gdot`
- `gdotopt`
- `ginclude`
- `gincludelines`
- `gnotes`
- `gnotesblock`
- `gfooter`
- `gcopyright`
- `gpagenums`

# Vim + NeoBundle + neosnippet

前提:

- `NeoBundle` を使う
- `UltiSnips` の代わりに `neosnippet` を使う

`.vimrc` には `UltiSnips` の代わりに次を入れます。

```vim
NeoBundle 'Shougo/neosnippet.vim'
NeoBundle 'Shougo/neosnippet-snippets'

imap <C-k>     <Plug>(neosnippet_expand_or_jump)
smap <C-k>     <Plug>(neosnippet_expand_or_jump)
xmap <C-k>     <Plug>(neosnippet_expand_target)

if has('conceal')
  set conceallevel=2 concealcursor=niv
endif
```

`UltiSnips` 行とその trigger 設定は削除するかコメントアウトしてください。

```vim
" NeoBundle 'SirVer/ultisnips'
" let g:UltiSnipsExpandTrigger = "<Tab>"
" let g:UltiSnipsJumpForwardTrigger = "<Tab>"
" let g:UltiSnipsJumpBackwardTrigger = "<S-Tab>"
```

その後 Vim で次を実行します。

```vim
:NeoBundleInstall
```

ローカルスニペットの配置:

```sh
mkdir -p ~/.vim/snippets
cp snippets/vim/markdown.snip ~/.vim/snippets/markdown.snip
```

使い方:

1. `.md` を開く
2. `gcallout` や `gcover` と入力する
3. `Ctrl-k` で展開する
4. 次の項目へも `Ctrl-k` で移動する

確認:

```vim
:echo exists(':NeoSnippetEdit')
:set filetype?
```

期待値:

- `exists(':NeoSnippetEdit')` は `2`
- Markdown では `filetype=markdown`

`Tab` は他の補完やインデントと衝突しやすいので、`neosnippet` では `Ctrl-k` に寄せるのが安全です。

# Emacs

前提:

- `yasnippet` を使う

設定例:

```elisp
(require 'yasnippet)
(yas-global-mode 1)
(add-to-list 'yas-snippet-dirs "/Users/sugita/prj/doc/templates/markdown/snippets/emacs")
(yas-reload-all)
```

この配置では `markdown-mode` 用スニペットとして読み込まれます。

使い方:

1. `.md` を `markdown-mode` で開く
2. たとえば `gcover` と入力する
3. `M-x yas-expand` を実行するか、割り当て済みキーで展開する

主なトリガーは Vim 版と同じです。

# Copilot との併用

Copilot 自体に `USER_GUIDE.md` を学習させるより、まずスニペットで骨格を出して、その後の文言や引数の補完を Copilot に任せる方が安定します。

特に次の使い方が実用的です。

- `gcallout` を展開して本文だけ Copilot に書かせる
- `gcover` を展開して `caption` や `alt` を Copilot に補完させる
- `gmermaid` を展開して図のノード列を Copilot に続けさせる
- `gdot` を展開して Graphviz DOT のノードやエッジを Copilot に続けさせる
