---
genpdf:
  format: book
  title: Cutter 利用ガイド
  subtitle: クロマキーも可能な画像切り抜きツール
  author: (株) SRA
  date: 2026-05-06
  font_size: 12pt
  page_numbers: true
  copyright: Copyright (c) 2024 SRA, Inc. All rights reserved.
  table:
    header_background: "#e8f1ff"
    header_color: "#1f2937"
    border_color: "#cbd5e1"
    border_width: "1px"
    stripe: true
    stripe_background: "#f8fafc"
    cell_padding: "0.45em 0.65em"
    font_size: "0.95em"
    compact: false
    align: left
    header_align: center
    cell_align: left
---

# cutter

Qt 6 で構築された画像切り抜きユーティリティです。

一般的な画像ビューアやエディタのマウス操作による切り抜きは、見た目では十分でも、厳密なピクセル単位での指定が難しいことがあります。`cutter` では座標や幅・高さを数値で指定できるため、必要な範囲を正確に切り抜けます。

## 使い方

```text
cutter [--overwrite]
       [--clip-green | --cut=N | --cut-top=N | --cut-bottom=N | --cut-left=N | --cut-right=N]
       [GEOMETRY | --geometry=GEOMETRY]
       FILE
```

`FILE` を切り抜き、結果を `<base>-<geometry>.<ext>` として書き出します。

入力画像と同じ形式で保存できる場合は、出力ファイルも同じ形式になります。入力形式で保存できない場合は、PNG へ切り替えて保存し、その旨をメッセージ表示します。

## オプション

- `--overwrite`
  確認せずに出力ファイルを上書きします。
- `--over`
  `--overwrite` の短縮形です。
- `--clip-green`
  端からつながっている緑系の背景を透明化し、残ったピクセルをすべて含む最小矩形で切り抜いたあと、2 倍サイズの透明キャンバス中央に配置します。不定形の対象でも使用できます。
- `--cut=N`
  画像の上下左右をそれぞれ `Npx` ずつ削ります。
- `--cut-top=N`
  画像の上側を `Npx` 削ります。
- `--cut-bottom=N`
  画像の下側を `Npx` 削ります。
- `--cut-left=N`
  画像の左側を `Npx` 削ります。
- `--cut-right=N`
  画像の右側を `Npx` 削ります。
- `--geometry=GEOMETRY`
  切り抜き範囲をオプションとして明示的に指定します。
- `--geo=GEOMETRY`
  `--geometry=GEOMETRY` の短縮形です。

## ジオメトリ形式

- `WxH+X+Y`
  幅と高さ、および左上からのオフセットを指定します。
- `X1,Y1:X2,Y2`
  左上と右下の座標を指定します。
- `X1,Y1-X2,Y2`
  上と同じ形式で、区切りに `-` を使います。

`GEOMETRY` を省略した場合は、画像全体が使用されます。

指定した範囲が元画像をはみ出す場合、切り抜き領域は画像の境界内に切り詰められます。

`--clip-green` は、背景を先に緑で塗り潰し、その後対象画像を描画した画像向けです。ChatGPT ブラウザーなどで保存した画像のように、緑が `0x00ff00` から多少揺れていても背景として扱います。背景判定は画像端からつながっている部分だけに適用し、背景部分は透明になります。円、矩形、不定形の対象でも同じように動作し、外周の緑背景から到達できない対象内の色は背景として扱いません。

対象の縁に緑が反射したり、アンチエイリアスで緑が混ざったりしている場合は、縁の緑成分を抑えて透明度に反映します。これにより、透明化後の緑のにじみや縁のかすれを軽減します。ただし、対象そのものに明るい緑色の部分がある場合は、縁補正の影響を受けることがあります。

出力は、切り抜いた画像を 2 倍サイズの透明キャンバス中央に配置した画像になります。

`--clip-green` と `GEOMETRY` / `--cut` 系オプションは同時に指定できません。

`--cut` 系オプションと `GEOMETRY` は同時に指定できません。

`--cut` と `--cut-top` / `--cut-bottom` / `--cut-left` / `--cut-right` も同時に指定できません。

## 例

```bash
# JPG を切り抜くと、保存できる場合は JPG のまま出力される
cutter photo.jpg

# 0x00ff00 の背景を除いて切り抜く
cutter --clip-green input.png

# 付属サンプルでクロマキー切り抜きを確認する
cutter --clip-green testdata/vu.png

# 四辺を均等に削る
cutter --cut=5 input.png

# 上側だけ削る
cutter --cut-top=5 input.png

# 複数の辺を個別に削る
cutter --cut-top=5 --cut-right=10 input.png

# 幅・高さ・オフセットを指定して切り抜く
cutter 100x100+10+20 input.png

# 同じ指定で、既存の出力ファイルを上書きする
cutter --overwrite 100x100+10+20 input.png

# 2 つの角の座標で切り抜く
cutter 10,20:110,120 input.png

# ジオメトリはオプションとしても渡せる
cutter --geometry=10,20-110,120 input.png
```

## 出力

成功すると、`cutter` は出力ファイル名を標準出力に表示します。

出力先ファイルがすでに存在し、`--overwrite` が指定されていない場合は、上書きするかどうかを確認します。

## 付属サンプル

`testdata/vu.png` は `--clip-green` の確認用画像です。実行すると、緑系の背景が透明化され、対象画像が透明キャンバス中央に配置されます。対象の形は矩形に限らず、背景が画像端からつながっていれば不定形でも同じ方法で切り抜かれます。
