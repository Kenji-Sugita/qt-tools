# Qt Tools

Qt/C++ の開発支援ツール、対話的プログラミング環境、資料作成ツール、
アプリ開発の手順と文書テンプレート、作業コンテキストを管理するテンプレート、
日常的に使用するユーティリティーなどをまとめたリポジトリーです。

Qt を使ったアプリケーションやライブラリーを中心に、
Python/Bash によるツールや、生成 AI を利用した Qt 開発を支援するための
ファイルも含まれています。

[English](README.en.md)

## 目的から探す

| 系統 | 内容 | 入口 |
|---|---|---|
| Tools | 実行するツール、アプリへ組み込むライブラリー | [用途別ツール一覧](tools/README.md) |
| Workflows | コピー・適用して使う手順、指示、テンプレート | [ワークフロー一覧](workflows/README.md) |
| Docs | 読んで学ぶ・判断する資料と、既存資料への読書案内 | [文書一覧と読む順番](docs/README.md) |

## Featured: Interactive Qt/C++

Qt Clingとicppで対話的に試し、ObjectSelector・PropertyEditorで実行中のオブジェクトを調べます。
[詳しい紹介とツール一覧](tools/README.md#featured-interactive-qtc)を参照してください。

## Featured: App Generation Workflow

要求整理、仕様化前ノート、UC・UI・ビジネスレイヤーの三点セット、実装、レビューを文書でつなぎます。
[詳しい紹介と始め方](workflows/README.md#featured-app-generation-workflow)を参照してください。

## Featured: Work Templates

現在の状態、判断の理由、長期の課題をMarkdownへ残し、人・PC・AIセッションをまたいで作業を引き継ぎます。
[詳しい紹介と運用例](workflows/README.md#featured-work-templates)を参照してください。

## Documentation

各ツールに密接なガイドとサンプルは、そのプロジェクト内に置いています。
横断的な読書案内は [docs/](docs/README.md)、Sites正本と公開先・旧配置の対応は [PUBLICATION_MAP.md](PUBLICATION_MAP.md) を参照してください。

日英文書は同じ場所の `.md` と `.en.md`（ClingのREADMEは `.txt`）を使用します。
ルートの `NEXT.md`、`DECISIONS.md`、`backlog.md` はmaintainerの実運用用記録です。再利用用テンプレートは [work-templates](workflows/work-templates/README.md) にあります。


## Platforms

ツールによって対応環境は異なります。

主に次の環境を対象としています。

- macOS
- Linux
- Windows
- Qt 6

コマンド系は、genpdf など一部を除き、macOS と Linux を対象としています。

一部のツールには個別の制約があります。

- VuStereo は macOS 専用です。
- `tl` は Google Cloud Translation の認証設定を必要とします。
- Qt Cling / icpp は Cling および対応する Qt 環境を必要とします。

詳細は各ツールのドキュメントを参照してください。


## About This Repository

ここにあるツールの多くは、実際の Qt/C++ 開発やセミナー、
資料作成などで必要になったものを作り、使用しながら改良してきたものです。

大きな統合環境を作ることよりも、
それぞれのツールが単独でも利用でき、必要に応じて組み合わせられることを
重視しています。
