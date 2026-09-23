# Feature Ideas

## Status

- もともとの候補機能はすべて実装済み
- 現在の主な残課題は `!incremental-list` の描画安定性

## `!pause` (Implemented)

- スライドを手で複製せず、その位置までを見せた複製スライドを作る記法
- `!incremental-list` より汎用で、箇条書き以外にも使える

## `!notes{...}` (Implemented)

- 発表者メモを本文に出さず、別テキストや別 PDF に出せる
- スライド資料として使う場合の実用性が高い

## `!fit-code` (Implemented)

- コードブロックが長いときに自動で文字サイズを少し下げる
- スライド向けで特に有効

## `!columns` (Implemented)

- 2カラムや 3カラムの簡易レイアウト
- 表や画像と箇条書きを並べたいときに便利

## `!cover-image{...}` (Implemented)

- 表紙や最初のスライドに大きいビジュアルを置く専用記法
- 文字中心のテンプレートでも見栄えを上げやすい

## `!toc` (Implemented)

- `book` 用に目次を自動生成する
- 長文資料で効果が大きい

## `!callout{type=warning}` (Implemented)

- 注意、補足、重要ポイントを装飾付きボックスにする記法
- plain Markdown より読みやすくなる

## `!incremental-list` (Implemented)

- スライド内の Markdown リストを 1 項目ずつ増やした複製スライドへ展開する
- PDF 向けの段階表示風の見せ方として利用できる

## Mermaid fenced code block (Implemented)

- ```` ```mermaid ```` の fenced code block を図として描画できる
- フローチャートや簡単な構成図を Markdown 内で管理できる
- `book` / `slides` のどちらでも使える

## Graphviz DOT fenced code block (Implemented)

- ```` ```dot ```` の fenced code block を SVG に変換して埋め込める
- 依存関係図、状態遷移、処理フロー、ネットワーク構成に向いている
- `!dot{align=right scale=0.8}` で寄せ方と大きさを指定できる

## `!svg{...}` (Implemented)

- SVG ファイルを `<figure>` として配置できる
- `alt`、`caption`、`width`、`align` を指定できる
- VU メーターやキーボードのような再利用するベクター部品に向いている

## Known Issues

- `!incremental-list` は、ページによっては最初の展開スライドで項目位置が上下にずれることがある
- 現状のサンプルでは、安定性を優先して該当箇所を `!pause` に置き換えている

## Additional Ideas

## `!section-cover{...}`

- 章や節の区切り用に、見出しだけを大きく見せる中扉スライド / 中扉ページを作る
- `book` と `slides` の両方で使えると、長い資料の視認性が上がる
- `!cover-image` より軽量で、文字中心の資料でも使いやすい

## `!speaker-only{...}`

- `!notes` は別ファイル出力だが、こちらは「発表用 PDF には出すが配布版では消す」のような条件付き本文を扱う
- 1 つの Markdown から「発表者向け版」と「配布版」を分けて作れる
- FAQ、補足、口頭だけで触れる注意点の管理が楽になる

## `!auto-fit`

- ページ全体の内容量を見て、本文やリストのサイズを安全な範囲で少しだけ自動縮小する
- `!fit-code` の対象をコード以外にも広げる発想
- 特に `slides` での「1 行だけあふれる」を減らせる

## `!agenda-progress`

- スライドや章ごとに、全体の進行位置を小さなバーや箇条書きで表示する
- 長い発表や研修資料で、今どこを話しているかが伝わりやすい
- `!toc` が静的一覧なら、こちらは各ページでの進行表示に寄せた機能

## `!quote{...}`

- 引用文と出典を見やすくレイアウトする専用記法
- plain blockquote より見栄えを揃えやすく、`book` でも `slides` でも需要がある
- 出典 URL、著者、年などを属性で持てると再利用しやすい

## `!table-compact`

- 直後の表だけ文字サイズ、セル余白、折り返しを表向けに最適化する
- スライドでは表が最も崩れやすいので、専用の逃げ道があると運用しやすい
- `!fit-code` と同じく「よく崩れる要素を局所的に救う」機能として自然
