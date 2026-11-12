---
genpdf:
  Format: slides
  Title: |-
    Qt for Python
    入門
  Subtitle: |-
    1.0 版
    基本操作
  Author: (株) SRA
  Date: 2024-06-01
  Page_Numbers: true
  Background:
    Image: company-footer-logo-band.svg
    Target: all
    Size: 100%
    Position: bottom center
    Repeat: no-repeat
    Opacity: 1
---

<!-- slide -->
<h1 align="center">導入と全体像</h1>
<p class="slide-subtitle">目的と使いどころ</p>

このサンプルでは、PDF スライド向けの基本機能をまとめて確認できます。

## このテンプレートでできること

- タイトル付きのスライド PDF を生成する
- 箇条書きの段階表示風スライドを作る
- カバー画像、コールアウト、発表者メモを併用する

### 段階表示の例

!incremental-list
- 目的と全体像を先に伝える
- 要点を 1 つずつ追加しながら説明する
- 最後に詳細ページへつなげる

<div class="page-break"></div>

<!-- slide -->
<h1 align="center">デモの見どころ</h1>
<p class="slide-subtitle">確認する機能の流れ</p>

このページでは、サンプル全体の見どころを先に整理します。

!callout{type=note title=このあと確認するもの}
  カバー画像、段階表示、コード表示、フッター、発表者メモを順番に確認します。

<!-- slide -->
<div class="page-break"></div>
<h1 align="center">インストール画面の紹介</h1>
<p class="slide-subtitle">画面を使った手順説明</p>

!cover-image{path=../images/qt-installation-01.png alt=Qt-installation caption=インストール画面 width=82%}
!notes{
    インストール手順の全体像を最初に説明する
    画面を見せながら、必要なツールを確認する
    画面の説明は次のライブデモで補足する
}

<!-- slide -->
<div class="page-break"></div>
### 実装コードの抜粋

!fit-code
```python
def build_pdf(source: str) -> None:
    print(f"render: {source}")
    print("apply template")
    print("write pdf 01")
    print("write pdf 02")
    print("write pdf 03")
    print("write pdf 04")
    print("write pdf 05")
    print("write pdf 06")
    print("write pdf 07")
    print("write pdf 08")
    print("write pdf 09")
    print("write pdf 10")
    print("write pdf 11")
    print("write pdf 12")
    print("write pdf 13")
    print("write pdf 14")
    print("write pdf 15")
    print("write pdf 16")
    print("write pdf 17")
    print("write pdf 18")
    print("write pdf 19")
    print("write pdf 20")
```

!callout{type=note title=補足}
  長めのコードを 1 ページに収めたいときは `!fit-code` を使います。
