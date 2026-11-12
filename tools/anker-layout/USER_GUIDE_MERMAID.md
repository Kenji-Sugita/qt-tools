---
genpdf:
  format: book
  title: Anker Layout 利用ガイド Mermaid 図版
  subtitle: Mermaid 図で XmForm 風 attachment レイアウトを説明する
  author: Anker Layout Project
  version: 0.1.0
  date: 2026-06-30
  font_size: 11pt
  page_numbers: true
---

# Anker Layout 利用ガイド

Anker Layout は、Qt Widgets で Motif の `XmForm` に近いフォーム型レイアウトを使うための `QLayout` 実装です。各ウィジェットの `left`、`right`、`top`、`bottom` をフォーム、他のウィジェット、位置割合、または自身の初期位置に attachment して配置します。

## 要件

- Qt 6 Widgets
- C++20
- CMake 3.16 以上

このプロジェクトは C++20 の designated initializer を前提にしています。

```cpp
layout->addWidget(editor, {
    .left = attachWidget(label, 8),
    .right = attachForm(),
    .top = attachOppositeWidget(label),
});
```

## ビルド

プロジェクト全体をビルドします。

```bash
cmake -S . -B build
cmake --build build
```

この環境で Qt License Service のロックエラーが出る場合は、次のように実行します。

```bash
QTFRAMEWORK_BYPASS_LICENSE_CHECK=1 cmake --build build
```

テストを実行します。

```bash
QTFRAMEWORK_BYPASS_LICENSE_CHECK=1 ctest --test-dir build --output-on-failure
```

## 基本的な使い方

`FormLayout` を親ウィジェットに設定し、`addWidget()` の第 2 引数で `FormConstraints` を渡します。

```cpp
#include "AnkerFormLayout.h"

#include <QApplication>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

using namespace Anker;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    auto *layout = new FormLayout(&window);
    layout->setContentsMargins(12, 12, 12, 12);

    auto *nameLabel = new QLabel("Name:");
    auto *nameEdit = new QLineEdit;
    auto *okButton = new QPushButton("OK");

    layout->addWidget(nameLabel, {
        .left = attachForm(),
        .top = attachForm(),
    });

    layout->addWidget(nameEdit, {
        .left = attachWidget(nameLabel, 8),
        .right = attachForm(),
        .top = attachOppositeWidget(nameLabel),
    });

    layout->addWidget(okButton, {
        .right = attachForm(),
        .bottom = attachForm(),
    });

    window.resize(420, 160);
    window.show();
    return app.exec();
}
```

## 制約の考え方

`FormConstraints` は 4 辺の attachment を持つ構造体です。

```cpp
struct FormConstraints
{
    FormAttachment left;
    FormAttachment right;
    FormAttachment top;
    FormAttachment bottom;
};
```

指定しない辺は `attachNone()` と同じ扱いです。片側だけを指定した場合、ウィジェットは `sizeHint()` と `minimumSize()` を基準に配置されます。両側を指定した場合、その 2 辺の間に伸縮します。

基本モデルは次の図のようになります。

!mermaid{align=center scale=0.9}
```mermaid
flowchart LR
  form["FormLayout"]:::form
  child["child widget"]:::primary
  top["top"] -->|"top attachment"| child
  left["left"] -->|"left attachment"| child
  right["right"] -->|"right attachment"| child
  bottom["bottom"] -->|"bottom attachment"| child
  form -. "contained by" .-> child

  classDef form fill:#f8fafc,stroke:#64748b,color:#111827;
  classDef primary fill:#dbeafe,stroke:#2563eb,color:#111827;
```

4 辺のうち、指定した辺だけが attachment で解決されます。

!mermaid{align=center scale=0.9}
```mermaid
flowchart LR
  oneTitle["one side: left = attachForm(12)"]:::note
  oneForm["Form left edge"]:::form
  oneChild["child<br/>sizeHint width"]:::primary
  twoTitle["two sides: left/right = attachForm(12)"]:::note
  twoLeft["left form edge"]:::form
  twoChild["child stretches"]:::success
  twoRight["right form edge"]:::form

  oneTitle --> oneForm -->|"12px"| oneChild --> twoTitle --> twoLeft -->|"12px"| twoChild -->|"12px"| twoRight

  classDef note fill:#ffffff,stroke:#ffffff,color:#111827;
  classDef form fill:#f8fafc,stroke:#64748b,color:#111827;
  classDef primary fill:#dbeafe,stroke:#2563eb,color:#111827;
  classDef success fill:#dcfce7,stroke:#16a34a,color:#111827;
```

## Attachment 一覧

### attachNone()

辺を固定しません。

```cpp
.left = attachNone()
```

通常は省略すれば同じ意味になります。

### attachForm(offset)

フォームの同じ側の辺へ attachment します。

```cpp
layout->addWidget(child, {
    .left = attachForm(12),
    .right = attachForm(12),
    .top = attachForm(12),
});
```

`left` と `top` の offset は内側へ加算されます。`right` と `bottom` の offset は内側へ減算されます。

!mermaid{align=center scale=0.85}
```mermaid
flowchart LR
  formLeft["left form edge"]:::form -->|"+12px"| child["child"]:::primary -->|"-12px"| formRight["right form edge"]:::form

  classDef form fill:#f8fafc,stroke:#64748b,color:#111827;
  classDef primary fill:#dbeafe,stroke:#2563eb,color:#111827;
```

### attachWidget(target, offset)

対象ウィジェットの反対側の辺へ attachment します。

```cpp
layout->addWidget(edit, {
    .left = attachWidget(label, 8),
    .top = attachOppositeWidget(label),
});
```

`.left = attachWidget(label, 8)` は、`edit` の左辺を `label` の右辺から 8 px 右へ置きます。

!mermaid{align=center scale=0.85}
```mermaid
flowchart LR
  label["label"]:::label -->|"attachWidget(label, 8)<br/>label right edge + 8px"| edit["edit"]:::primary

  classDef label fill:#fef3c7,stroke:#d97706,color:#111827;
  classDef primary fill:#dbeafe,stroke:#2563eb,color:#111827;
```

offset を省略した場合は `FormLayout` の spacing を使います。

```cpp
layout->setHorizontalSpacing(12);

layout->addWidget(rightPanel, {
    .left = attachWidget(leftPanel),
});
```

### attachOppositeWidget(target, offset)

対象ウィジェットの同じ側の辺へ attachment します。

```cpp
layout->addWidget(edit, {
    .left = attachOppositeWidget(reference),
    .right = attachOppositeWidget(otherReference),
    .top = attachWidget(reference),
});
```

ラベルと入力欄の上端を揃える、複数行の左端を揃える、といった用途に使います。

!mermaid{align=center scale=0.85}
```mermaid
flowchart TB
  sharedLeft["shared left edge"]:::plain
  sharedRight["shared right edge"]:::plain
  reference["reference"]:::label
  otherReference["otherReference"]:::success
  edit["edit"]:::primary

  sharedLeft --> reference
  sharedLeft -->|"left = attachOppositeWidget(reference)"| edit
  edit -->|"right = attachOppositeWidget(otherReference)"| sharedRight
  otherReference --> sharedRight

  classDef plain fill:#ffffff,stroke:#ffffff,color:#111827;
  classDef label fill:#fef3c7,stroke:#d97706,color:#111827;
  classDef success fill:#dcfce7,stroke:#16a34a,color:#111827;
  classDef primary fill:#dbeafe,stroke:#2563eb,color:#111827;
```

### attachPosition(position, offset)

フォーム内の割合位置へ attachment します。

```cpp
layout->setFractionBase(100);

layout->addWidget(leftPane, {
    .left = attachForm(),
    .right = attachPosition(50, 6),
});

layout->addWidget(rightPane, {
    .left = attachPosition(50, 6),
    .right = attachForm(),
});
```

`fractionBase` が 100 の場合、`attachPosition(50)` はフォーム幅または高さの 50% 位置です。

!mermaid{align=center scale=0.85}
```mermaid
flowchart LR
  formLeft["form left"]:::form --> leftPane["leftPane"]:::primary --> gapLeft["6px"]:::split --> split["50%"]:::split --> gapRight["6px"]:::split --> rightPane["rightPane"]:::success --> formRight["form right"]:::form
  leftPane -. "left = attachForm()" .-> formLeft
  leftPane -. "right = attachPosition(50, 6)" .-> split
  rightPane -. "left = attachPosition(50, 6)" .-> split
  rightPane -. "right = attachForm()" .-> formRight

  classDef form fill:#f8fafc,stroke:#64748b,color:#111827;
  classDef primary fill:#dbeafe,stroke:#2563eb,color:#111827;
  classDef success fill:#dcfce7,stroke:#16a34a,color:#111827;
  classDef split fill:#fef9c3,stroke:#ca8a04,color:#111827;
```

### attachOppositeForm(offset)

フォームの反対側の辺へ attachment します。

```cpp
layout->addWidget(panel, {
    .left = attachOppositeForm(-320),
    .right = attachForm(),
});
```

`left` に指定するとフォーム右端側を基準にします。`right` に指定するとフォーム左端側を基準にします。フォーム外側を基準にした配置や、反対側エッジから戻す配置を表現できます。

!mermaid{align=center scale=0.85}
```mermaid
flowchart LR
  formLeft["form left"]:::form
  empty["form interior"]:::form
  panel["panel"]:::danger
  formRight["form right<br/>opposite edge for left"]:::form

  formLeft --> empty --> panel --> formRight
  panel -. "left = attachOppositeForm(-320)" .-> formRight
  panel -. "right = attachForm()" .-> formRight

  classDef form fill:#f8fafc,stroke:#64748b,color:#111827;
  classDef danger fill:#fee2e2,stroke:#dc2626,color:#111827;
```

### attachSelf(offset)

レイアウト管理に入る前の自身の初期ジオメトリを基準にします。

```cpp
selfPanel->setGeometry(420, 300, 220, 72);

layout->addWidget(selfPanel, {
    .left = attachSelf(),
    .top = attachSelf(),
});
```

`setRubberPositioning(true)` と組み合わせると、フォームサイズ変更時に初期位置の比率を保つように移動します。

```cpp
layout->setRubberPositioning(true);
```

!mermaid{align=center scale=0.85}
```mermaid
flowchart LR
  form["form"]:::form --> x["x = 420"]:::plain --> y["y = 300"]:::plain --> selfPanel["selfPanel<br/>220 x 72<br/>left/top = attachSelf()"]:::self

  classDef form fill:#f8fafc,stroke:#64748b,color:#111827;
  classDef plain fill:#ffffff,stroke:#ffffff,color:#111827;
  classDef self fill:#ede9fe,stroke:#7c3aed,color:#111827;
```

## FormLayout の設定

### fractionBase

`attachPosition()` の基準値です。デフォルトは `100` です。

```cpp
layout->setFractionBase(100);
```

### rubberPositioning

`attachSelf()` の基準位置をフォームサイズに合わせて比率移動させます。デフォルトは `false` です。

```cpp
layout->setRubberPositioning(true);
```

### spacing

`attachWidget(target)` の offset を省略した場合に使われます。

```cpp
layout->setHorizontalSpacing(12);
layout->setVerticalSpacing(10);
```

明示的に offset を指定した場合は、その値が優先されます。

```cpp
.left = attachWidget(label, 8)
```

## 制約の変更

追加済みウィジェットの制約は `setConstraints()` で変更できます。

```cpp
layout->setConstraints(panel, {
    .left = attachForm(12),
    .right = attachForm(12),
    .top = attachPosition(50),
});
```

現在の制約は `constraintsFor()` で取得できます。

```cpp
FormConstraints constraints = layout->constraintsFor(panel);
```

## サンプル

各サンプルはルートからまとめてビルドできます。

```bash
cmake -S . -B build
QTFRAMEWORK_BYPASS_LICENSE_CHECK=1 cmake --build build
```

サンプルごとの単体ビルドもできます。

```bash
cmake -S examples/basic_form -B build-basic_form
QTFRAMEWORK_BYPASS_LICENSE_CHECK=1 cmake --build build-basic_form
```

### examples/basic_form

ラベル、入力欄、ボタンを使った基本的なフォームです。

!mermaid{align=center scale=0.8}
```mermaid
flowchart TB
  row1["Name: | nameEdit"]:::primary
  spacer[" "]:::plain
  buttons["Cancel | OK"]:::button
  row1 --> spacer --> buttons

  classDef primary fill:#dbeafe,stroke:#2563eb,color:#111827;
  classDef button fill:#f8fafc,stroke:#64748b,color:#111827;
  classDef plain fill:#ffffff,stroke:#ffffff,color:#ffffff;
```

### examples/relative_widgets

複数のラベルと入力欄を、他ウィジェットへの attachment で並べます。

!mermaid{align=center scale=0.8}
```mermaid
flowchart TB
  title["Title: | titleEdit"]:::primary
  gap["16px"]:::plain
  body["Body: | bodyEdit"]:::success
  title --> gap --> body

  classDef primary fill:#dbeafe,stroke:#2563eb,color:#111827;
  classDef success fill:#dcfce7,stroke:#16a34a,color:#111827;
  classDef plain fill:#ffffff,stroke:#ffffff,color:#111827;
```

### examples/position_anchors

`attachPosition()` と `fractionBase` を使って、フォームを比率で分割します。

!mermaid{align=center scale=0.8}
```mermaid
flowchart LR
  left["0-50%"]:::primary
  top["50-100%<br/>top"]:::success
  bottom["50-100%<br/>bottom"]:::label
  left --> top
  left --> bottom

  classDef primary fill:#dbeafe,stroke:#2563eb,color:#111827;
  classDef success fill:#dcfce7,stroke:#16a34a,color:#111827;
  classDef label fill:#fef3c7,stroke:#d97706,color:#111827;
```

### examples/opposite_edges

`attachOppositeWidget()` で複数ウィジェットの辺を揃えます。

!mermaid{align=center scale=0.8}
```mermaid
flowchart TB
  header["form left | 25% | 50% | form right"]:::plain
  host["Host: | hostEdit"]:::primary
  port["Port: | portEdit"]:::success
  protocol["Protocol: | protocolCombo"]:::self
  header --> host --> port --> protocol

  classDef plain fill:#ffffff,stroke:#ffffff,color:#111827;
  classDef primary fill:#dbeafe,stroke:#2563eb,color:#111827;
  classDef success fill:#dcfce7,stroke:#16a34a,color:#111827;
  classDef self fill:#ede9fe,stroke:#7c3aed,color:#111827;
```

### examples/resizable_dialog

リサイズ可能なダイアログで、リスト、詳細欄、ボタンを配置します。

!mermaid{align=center scale=0.8}
```mermaid
flowchart TB
  header["form left | 35% | form right"]:::plain
  labels["Items | Details"]:::label
  content["list | detail"]:::primary
  buttons["OK | Cancel"]:::button
  header --> labels --> content --> buttons

  classDef plain fill:#ffffff,stroke:#ffffff,color:#111827;
  classDef label fill:#fef3c7,stroke:#d97706,color:#111827;
  classDef primary fill:#dbeafe,stroke:#2563eb,color:#111827;
  classDef button fill:#f8fafc,stroke:#64748b,color:#111827;
```

### examples/full_attachments

追加機能をまとめて確認するサンプルです。

!mermaid{align=center scale=0.8}
```mermaid
flowchart TB
  top["FORM<br/>left/top + right=50% &nbsp;&nbsp; | &nbsp;&nbsp; WIDGET<br/>uses default spacing"]:::top
  oppositeWidget["OPPOSITE_WIDGET<br/>aligned to first row"]:::label
  lower["OPPOSITE_FORM<br/>right from opposite left edge &nbsp;&nbsp; | &nbsp;&nbsp; SELF<br/>initial rectangle"]:::lower
  button["Bottom right"]:::button
  top --> oppositeWidget --> lower --> button

  classDef top fill:#d7ecff,stroke:#2563eb,color:#111827;
  classDef label fill:#fff0c9,stroke:#d97706,color:#111827;
  classDef lower fill:#ffd9d6,stroke:#dc2626,color:#111827;
  classDef button fill:#f8fafc,stroke:#64748b,color:#111827;
```

- `attachOppositeForm()`
- `attachSelf()`
- `setRubberPositioning(true)`
- `setHorizontalSpacing()`
- `setVerticalSpacing()`
- offset 省略時の `attachWidget(target)`

## 注意点

- 依存関係が循環する attachment は避けてください。レイアウトは複数パスで解決しますが、循環した制約は意図しない配置になる可能性があります。
- `attachSelf()` は、レイアウト管理に入る前の `geometry()` を基準にします。初期位置を指定したい場合は、`addWidget()` の前に `setGeometry()` を呼んでください。
- `attachOppositeForm()` はフォームの反対側エッジを基準にするため、offset の符号によってフォーム外へ配置されます。
