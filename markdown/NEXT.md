# NEXT

このファイルを読む、または更新する前に、`~/AGENTS.next.md` を読み、その指示に従うこと。
`~/AGENTS.next.md` が存在しない場合は、このファイルを整理・削除せず、利用者に確認すること。
このルールは、ユーザーから明示的な指示がない限り変更しない。

## 未完了

- PDF 生成が重い・時々失敗する問題への改善を検討する。詳細候補は `backlog.md` の「改善候補: PDF 生成の重さと失敗対策」を参照。
- `!svg` に `max_height` を追加するか判断する。正方形 SVG や縦長 SVG がスライド本文領域を超えて次ページへ送られる問題の対策候補。
- `CHANGELOG.md` をリポジトリに残すか削除するか判断する。配布 zip には入れない方針。

## 現在の状態

- 図機能は Mermaid、Graphviz DOT、SVG ファイル参照が実装済み。
- DOT は ```` ```dot ```` fenced code block と `!dot{align=... scale=...}` に対応済み。
- SVG は `!svg{path=... alt=... caption=... width=... align=...}` に対応済み。
- `USER_GUIDE.md` / `USER_GUIDE.pdf` に、GitLab Markdown では ```` ```dot ```` がそのまま図として表示されるとは限らない注意を追記済み。
- VU メーターのサンプルは `images/vu-background.svg` を使用し、スライドでは `width=38%` に調整済み。
- キーボードのサンプルは `images/us-keyboard-white.svg` を使用。
- `USER_GUIDE.md`、`examples/book.md`、`examples/slides.md`、Vim / Emacs スニペットに DOT / SVG の説明とサンプルを追加済み。
- front matter の `genpdf.title` / `genpdf.subtitle` は `|-` による複数行指定に対応済み。表紙出力時に改行を `<br>` に変換し、HTML エスケープは維持する。`genpdf -T`、`examples/s.md`、`examples/slides.md`、`USER_GUIDE.md` / `USER_GUIDE.pdf` に反映済み。
- スライド本文の主タイトル直下に `<p class="slide-subtitle">...</p>` を置くサブタイトル表示を追加済み。サブタイトル下には下端バンドと同じ青 `#60a5fa` の細い横区切り線を表示する。CSS は `src/resources/styles/slides.css` と `styles/slides.css` に定義し、`examples/s.md`、`examples/slides.md`、`USER_GUIDE.md` / `USER_GUIDE.pdf` に反映済み。
- 下端背景バンドの左右隙間は `9f707c29a Fix bottom background band bleed` で修正済み。実 PDF ページサイズで合成し、SVG 画像は `preserveAspectRatio="none"` で左右端まで伸ばす。
- ロゴ入り下端バンドは `4a63a8eb2 Add logo footer band` で追加済み。`background-images/company-footer-logo-band.svg` と `images/sra-logo.png` を使う。
- ロゴ入り下端バンドの顧客名フォントは `0f5bd3164 Use Japanese font fallbacks in logo footer band` で日本語向け候補に変更済み。`Hiragino Sans, Yu Gothic, Noto Sans CJK JP, sans-serif` を指定する。
- Qt ロゴ入り下端バンドとして `background-images/qt-footer-logo-band.svg` を追加。`Qt - Primary Logo/For Designers and CMYK Printing/For Screen - RGB/Neon/Qt-logo-neon-transparent.png` を参照する。
- 下端背景 SVG 内のローカル画像参照は PDF 合成時に data URI へ展開する。下端背景の合成は `rsvg-convert` の 300dpi 指定で行う。
- 確認は重い `examples/slides.md` ではなく、短い `examples/s.md` を使う運用。`examples/s.md` はコミット済み。
- front matter 複数行タイトルは `python3 -m py_compile src/main.py`、`genpdf -T`、`examples/s.md` から `/private/tmp/genpdf-frontmatter-title-breaks.pdf` 生成、表紙画像化で確認済み。
- サブタイトル表示と区切り線は `examples/s.md` から `/private/tmp/genpdf-slide-title-rule-blue.pdf` を生成し、2ページ目の画像化で確認済み。サンプル内の手書き `---` は二重線を避けるため削除済み。
- `genpdf-release-20260507.zip` は更新済み。`USER_GUIDE.md` / `USER_GUIDE.pdf`、`examples/s.md`、`background-images/`、`images/`、`src/main.py`、`src/resources/` を含む。`CHANGELOG.md`、`__pycache__/`、`*.pyc`、`*.notes.md`、`examples/*.pdf` は含めない。
- 2026-08-06 に `genpdf-release-20260507.zip` を同じ収録方針で再作成し、内容と除外対象を再確認済み。
- 以前の実装履歴、検証ログ、配布 zip 情報、図作成候補の詳細は `backlog.md` に退避済み。

## 注意

- コマンドラインオプションは原則増やさない方針。
- 文書固有の設定は front matter、実行時都合は CLI に置く方針。
- 表カスタマイズは front matter のみで指定する。
- `format` の値は `book` / `slides` のまま維持する。
- 配布用 zip は Git 管理しない。`genpdf-release-*.zip` は `.gitignore` で除外する。
- zip 作成時は `CHANGELOG.md`、`__pycache__/`、`*.pyc`、`*.notes.md` を含めない。
- 未追跡の `examples/*.pdf`、`*.notes.md` は生成物なので、明示指示がない限りコミットしない。
