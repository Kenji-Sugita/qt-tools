# backlog

`NEXT.md` から退避した詳細な履歴、検証ログ、保留事項をまとめる。

## 実装済みの主な機能

- `src/main.py` に `!toc` 展開後の見出し崩れ対策を実装済み。
- `src/main.py` に front matter `genpdf.table` による表カスタマイズを実装済み。
- `genpdf.table` で列見出しとセルの文字揃えを別々に指定できるように更新済み。
- `genpdf.table.font_size` が全体の `genpdf.font_size` に上書きされないように修正済み。
- `-T` / `--front-matter-template` で front matter テンプレートを標準出力できるように実装済み。
- 文書メタ情報系 CLI オプションを非推奨化し、front matter 指定へ誘導する警告を出すように実装済み。
- `-h` は短い通常ヘルプ、`--help` は非推奨オプションも含む詳細ヘルプを表示するように分離済み。
- オプション整理方針は「文書固有の設定は front matter、実行時都合は CLI」に決定済み。
- `format: slides` は複数形のまま維持する方針。
- `genpdf.version` を front matter で指定すると、表紙に `Version ...` を表示できるように実装済み。
- `genpdf.version` 未指定時に表紙の日付 HTML が本文へ露出する不具合を修正済み。
- `USER_GUIDE.md` に `genpdf.table` の使い方、`header_align` / `cell_align` の指定例、`align` との優先関係、`-T` / `--front-matter-template` の使い方を追記済み。
- `USER_GUIDE.md` に、GitLab Markdown では ```` ```dot ```` がそのまま図として表示されるとは限らない注意と、Kroki / 事前変換画像 / Mermaid の選択肢を追記済み。
- `VIM_NEOSNIPPET_GUIDE.md` の Emacs 版として `EMACS_YASNIPPET_GUIDE.md` を新規作成済み。
- `CHANGELOG.md` は新規作成したが、ユーザー指示により配布 zip からは除外済み。
- `USER_GUIDE.pdf` を生成済み。
- `genpdf-release-20260507.zip` は配布候補として内容確認済み。

## 実装内容メモ

- `!toc` 修正:
  - `<div class="page-break"></div>` の前後を `normalize_page_break_spacing()` で正規化。
  - `split_pages()`、`extract_notes()`、`expand_incremental_lists()`、`expand_pause_directives()` のページ結合経路でも同じ正規化を使う。
  - `render_markdown()` の最終レンダリング前にも正規化する。
- 表カスタマイズ:
  - front matter の `genpdf.table` を独自パーサで読み取る。
  - 対応キーは `header_background`、`header_color`、`border_color`、`border_width`、`stripe`、`stripe_background`、`cell_padding`、`font_size`、`compact`、`align`、`header_align`、`cell_align`。
  - `align` は列見出しと本文セルの一括指定、`header_align` / `cell_align` は個別指定として扱う。
  - `align` と `header_align` / `cell_align` を同時指定した場合は個別指定を優先する。
  - `table.font_size` は `table th` / `table td` にも `!important` 付きで適用し、全体の本文フォントサイズ指定より優先する。
  - CSS は `build_table_html_prelude()` で `<style>` にして `build_prefix()` 経由で差し込む。
  - 通常レンダリングと `!footer` / `!font-size` によるページ別レンダリングの両方に適用する。
- front matter テンプレート出力:
  - `-T` / `--front-matter-template` を追加。
  - 入力 Markdown なしで実行可能。
  - PDF は生成せず、指定可能な `genpdf` front matter テンプレートを標準出力して終了する。
  - `source` は argparse 上は省略可能にし、`-T` 未指定時だけ従来どおり必須エラーにする。
  - テンプレートに `version: 1.0` を含める。
- 表紙 version 表示:
  - front matter の `genpdf.version` を読み取る。
  - `version` がある場合、book / slides の表紙で author と date の間に `Version {value}` を表示する。
  - `version` 未指定時は `__VERSION_HTML__` 行ごと削除し、空行で Markdown の raw HTML ブロックが切れないようにする。
  - `gfront` スニペットにも `version` を追加済み。
- CLI オプション非推奨化:
  - `--title`、`--subtitle`、`--author`、`--copyright`、`--page-numbers` / `--no-page-numbers`、`--font-size`、`--date` は後方互換性のため動作を残す。
  - 上記を指定した場合は stderr に非推奨警告を出し、対応する `genpdf.*` front matter キーを案内する。
  - `-h` では非推奨オプションを隠し、`--help` では `Deprecated` として表示する。
  - `USER_GUIDE.md` では文書固有のメタ情報は front matter 指定を推奨し、非推奨 CLI オプション一覧を追加。
- オプション設計:
  - front matter 推奨: `title`、`subtitle`、`author`、`copyright`、`page_numbers`、`font_size`、`date`。
  - CLI 継続: `source`、`--format`、`--output`、`--passthrough`、`--keep-temp`、`-T` / `--front-matter-template`。
  - `--format` は front matter にもあるが、一時的な `book` / `slides` 出し分け用途があるため CLI に残す。
  - `slides` は「スライド一式の形式」として自然なので、`slide` への改名やエイリアス追加は行わない。
- Emacs スニペットガイド:
  - `EMACS_YASNIPPET_GUIDE.md` を追加。
  - `markdown-mode`、`yasnippet`、`snippets/emacs/markdown-mode/` の配置手順、確認方法、使い方、トラブルシュートを記載。
  - `TAB` 展開を基本にし、補完系と衝突する場合は `C-c y` へ `yas-expand` を割り当てる例を記載。

## 検証ログ

- `python3 -m py_compile src/main.py`
- `header_align` / `cell_align` 入り front matter から `table th` / `table td` 別々の CSS が生成されることを確認。
- `table.font_size` 入り front matter から `table th` / `table td` にフォントサイズ CSS が生成されることを確認。
- `python3 src/main.py /tmp/codex.md --output /tmp/codex.pdf` で PDF 生成成功。
- `python3 src/main.py -T` で front matter テンプレートが標準出力されることを確認。
- `python3 src/main.py -h` では非推奨オプションが表示されないことを確認。
- `python3 src/main.py --help` では非推奨オプションが `Deprecated` 付きで表示されることを確認。
- `python3 src/main.py` だけの場合は従来どおり `source` 必須エラーになることを確認。
- `python3 src/main.py -T --page-numbers --font-size 11pt --date 2026-04-30` で非推奨警告が出ることを確認。
- 最小再現 Markdown で `!toc` を含む PDF を生成し、`pdftotext` で `#` / `##` が露出しないことを確認。
- `genpdf.table` 入りの表サンプルで PDF 生成が成功することを確認。
- `unzip -l genpdf-release-20260430.zip` で旧配布 zip の内容を確認済み。
- `python3 src/main.py -T` と `python3 -m py_compile src/main.py` を直近でも再確認済み。
- `EMACS_YASNIPPET_GUIDE.md` の関連リンク先ファイル / ディレクトリが存在することを確認。
- `genpdf.version: 2.3-beta` 入りの最小 Markdown から PDF を生成し、`pdftotext` で表紙に `Version 2.3-beta` が出ることを確認。
- `genpdf.version` 未指定かつ `date` 省略の最小 Markdown から PDF を生成し、`pdftotext` で `<div>` / `</div>` / `class=` が露出しないことを確認。
- `python3 bin/genpdf USER_GUIDE.md --output USER_GUIDE.pdf` で `USER_GUIDE.pdf` を生成済み。
- `pdftotext USER_GUIDE.pdf -` で PDF のテキスト抽出ができることを確認。
- `unzip -l genpdf-release-20260507.zip` で配布 zip の内容を確認。
- `genpdf USER_GUIDE.md` で GitLab DOT 注意を追記した `USER_GUIDE.pdf` を再生成済み。
- `unzip -l genpdf-release-20260507.zip` で更新後の配布 zip 内容を確認。
- `zipinfo -1 genpdf-release-20260507.zip | rg '(^|/)CHANGELOG\.md$|__pycache__/|\.pyc$|\.notes\.md$|^examples/.*\.pdf$'` で除外対象が含まれていないことを確認。
- 2026-08-06 に `bsdtar --format zip` で `genpdf-release-20260507.zip` を再作成し、`unzip -l` で 32 ファイル収録を確認。
- 2026-08-06 に `zipinfo -1 genpdf-release-20260507.zip | rg '(^|/)CHANGELOG\.md$|__pycache__/|\.pyc$|\.notes\.md$|^examples/.*\.pdf$'` で除外対象が含まれていないことを再確認。
- 2026-08-06 に `genpdf-release-20260507.zip` を Git 管理から外し、`.gitignore` に `genpdf-release-*.zip` を追加。

## 配布 zip

- 作成済みファイル: `genpdf-release-20260507.zip`
- 状態: 配布用 zip として更新済み・内容確認済み
- 収録内容:
  - `bin/`
  - `images/`
  - `background-images/`
  - `USER_GUIDE.md`
  - `USER_GUIDE.pdf`
  - `examples/book.md`
  - `examples/slides.md`
  - `examples/s.md`
  - `src/resources/`
  - `src/main.py`
- 除外済み:
  - `CHANGELOG.md`
  - `__pycache__/`
  - `*.pyc`
  - `*.notes.md`
  - `examples/*.pdf`

## 未完了 / 確認待ち

- `CHANGELOG.md` をリポジトリに残すか削除するかは未確定。配布 zip には入れない方針。
- `manuscript/qtsmartpointer.md` はこの workspace では見つからず未確認。

## 改善候補: PDF 生成の重さと失敗対策

- 現状は `md-to-pdf` が Chromium とローカルサーバを使うため、図やページが増えると重くなりやすい。
- Mermaid はブラウザ上で JS 描画するため、図が多い資料では失敗や待ち時間の原因になりやすい。
- DOT は `dot -Tsvg` を毎回呼ぶため、図が増えると変換回数が増える。
- 複雑な SVG は Chromium 側の描画負荷が高くなる。
- `!footer` や `!font-size` がある文書はページごとの PDF 生成と結合になるため特に重い。
- 正方形に近い SVG はスライド本文領域の高さを超えると、`break-inside: avoid` により次ページへ送られることがある。
- `examples/slides.md` の VU メーターは `images/vu-background.svg` を使い、`width=38%` まで下げて、タイトル・キャプション・補足と同じページに収める方向で調整済み。

優先改善案:

- DOT 変換結果をキャッシュする。
  - DOT 本文、`align`、`scale`、Graphviz バージョンなどからハッシュを作る。
  - 同じ内容なら `dot -Tsvg` を再実行せず、既存 SVG を再利用する。
- Mermaid も可能なら事前 SVG 化またはキャッシュ対象にする。
  - ブラウザ描画依存を減らせると、PDF 生成の安定性が上がる。
  - 実装コストは DOT より高いので後回しでよい。
- 確認用に小さい Markdown だけをレンダリングする運用を用意する。
  - サンプル全体ではなく、対象ページだけを切り出して検証できると再試行が軽い。
- SVG ファイルは複雑なものと軽量版を分けられるようにする。
  - 例: `images/foo.svg` と `images/foo-lite.svg`。
- `!svg` に高さ上限を指定できるようにする。
  - 例: `!svg{path=... width=38% max_height=48vh}`。
  - 正方形 SVG や縦長 SVG が次ページへ送られる問題の対策になる。
- 生成処理のどこで時間がかかっているかを表示する。
  - DOT 変換、Mermaid 描画、md-to-pdf 実行、PDF 結合の時間を分けて見たい。

## 図作成機能の追加候補

- 実装済み: Graphviz DOT
  - ユーザーは過去に DOT をよく使っていた。
  - 依存関係図、状態遷移、処理フロー、ネットワーク構成に向く。
  - ```` ```dot ```` fenced code block を `dot -Tsvg` で SVG に変換して埋め込む。
  - `!dot{align=left|center|right scale=0.8}` で寄せ方と大きさを指定できる。
  - `examples/book.md` と `examples/slides.md` にサンプルを追加済み。
- 実装済み: SVG ファイル参照
  - Mermaid / DOT で足りない図を補う汎用策。
  - draw.io / diagrams.net などで作った SVG を使える。
  - PDF との相性がよく、見た目を細かく作りたい図に向く。
  - `!svg{path=... alt=... caption=... width=... align=...}` を追加済み。
  - `/Users/sugita/src/images` から VU メーターとキーボード SVG を `images/` にコピーし、サンプルに追加済み。
- 優先度 3: PlantUML
  - シーケンス図、クラス図、状態遷移図など UML 系に強い。
  - Java や PlantUML jar の依存が増えるため、DOT より導入コストは高い。
- 優先度 4: D2
  - 構成図やアーキテクチャ図をきれいに出しやすい。
  - 新しい記法を増やす負担があるため、DOT / PlantUML の後でよい。
- 優先度 5: 画像ファイル運用
  - PNG / JPEG を貼るだけなので確実。
  - 差分管理や再編集性は弱いため、スクリーンショットや最終手段向け。

## 過去に触ったファイル

- `src/main.py`
- `USER_GUIDE.md`
- `USER_GUIDE.pdf`
- `EMACS_YASNIPPET_GUIDE.md`
- `CHANGELOG.md`
- `NEXT.md`
- `genpdf-release-20260507.zip`
- `src/resources/templates/book-prefix.md`
- `src/resources/templates/slides-prefix.md`
- `src/resources/styles/book.css`
- `src/resources/styles/slides.css`
- `templates/book-prefix.md`
- `templates/slides-prefix.md`
- `styles/book.css`
- `styles/slides.css`
- `snippets/emacs/markdown-mode/gfront`
- `snippets/vim/markdown.snip`
- `snippets/vim/markdown.snippets`
- `SNIPPETS.md`
- `FEATURE_IDEAS.md`
- `examples/book.md`
- `examples/slides.md`
- `styles/markdown.css`
- `src/resources/styles/markdown.css`
- `snippets/emacs/markdown-mode/gdot`
- `snippets/emacs/markdown-mode/gdotopt`
- `images/vu-background.svg`
- `images/us-keyboard-white.svg`
- `snippets/emacs/markdown-mode/gsvg`

## 注意事項

- コマンドラインオプションは原則増やさない方針。ただしユーザー指示により `-T` / `--front-matter-template` は追加済み。
- 表カスタマイズは front matter のみで指定する。
- 文書メタ情報は front matter 指定を推奨し、旧 CLI 指定は後方互換性のためだけに残す。
- `format` の値は `book` / `slides` のまま維持する。
- `CHANGELOG.md` は未リリース履歴として作ったが、配布 zip には含めない。
- zip 作成時は不要な生成物を除外する。
