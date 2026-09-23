# Cling のインストール方法

## 成功したブランチ

    2026/3/12 時点

    Cling
        git clone --branch v1.3 https://github.com/root-project/cling.git

    LLVM
        git clone --branch cling-latest https://github.com/root-project/llvm-project.git

	cling-latest が Cling 利用者向けの最新バージョン。

        cling-latest は cling-llvm20-20251201-01 を指している。実際には
        cling-llvm20-* は、日付部分が先のタグがある。

        $ git describe --tags --exact-match
        cling-llvm20-20251201-01
        $

        $ git log
	commit 7c49650f1446898cb437657b36b867764ef93660 (HEAD -> cling-latest, tag: cling-llvm20-20251201-01, origin/cling-l...

## 用途

    * ウィジェット開発

    * QML バックエンド開発
      QML_ELEMENT などビルドに依存する機能は代替が必要。

    * 単体テスト

    * モックアップ

    ...

## 改善項目

    * インストール
      現在は、固定のディレクトリーでのみ実行可能。
      ハードコーディングされたパスを排除し、インストールして利用できる構成にする必要がある。

    * 設定
      インストール後の設定 (モジュールなど)

    * moc ファイルの自動生成
      現在は、手動で作成しなければならない。

    * .qrc からのリソースファイルの自動生成
      現在は、手動で作成しなければならない。

    * .ui からの ui_*.h ファイルの自動生成
      現在は、手動で作成しなければならない。

    * qtcling-project のようなツールが必要
      moc やリソース、.ui への対応のため。

    * プライベートヘッダーパスやrhi/qrhi.h へのパスの考慮

      -I をコマンドラインで指定する。

    * Qt Creator プラグイン
      可能か。

    * run_* コマンドのディレクトリー階層への対応

## TODO

    * 配布
      スクリプトや起動用ファイルを実行形式に埋め込んで見られないようにする。
      preload_qt_*.h、qtgui.cpp、...
      iqtcling.*、cqtcling.* は実行形式にする。

    * Qt 付属サンプルコードを動かして問題点を洗い出す

    * libeedit の扱い

      macOS
        MacPorts がない環境で動くようにする
        libedit を使ってしまっている。

      Linux
        libedit のインストールが必要。

      スタティックリンクで回避は、ラインセスの確認が必要。

    * Linux 版作成

    * Windows 版作成

    * Windows での対話実行対応

    * Windows ネイティブ対応

    * 起動スクリプトと補助ツールのリファクタリング
      qtcling、iqtcling.*、cqtcling.*、bin/run_moc、bin/run_rcc、bin/run_uic は現状は仮の実装を含むため、
      構成と実装を見直す必要がある。

    * 起動スクリプトと補助ツールの自動呼び出し対応
      リファクタリング後の構成を前提に、必要な場面で補助ツールを自動で呼び出せるようにする。

    * 雛形コード作成
      こう使えばよいというコード集

    * QML 向けサンプルコード作成
      現在のサンプルはウィジェット中心のため、QML の基本例や C++ 連携例も必要。

    * テストツールとの連携

    * 開発フロー
      Cling で開発し実際の C++ アプリケーションへの落とし込み

    * qtcling のオプションの整理

      Core と Gui をデフォルトにして、

      qtcling                      // Cling を対話モードで起動して、QtCore と QtGui を使う
      qtcling -c                   // Cling をバッチモードで起動して、QtCore を使う
      qtcling -i                   // Cling を対話モードで起動して、QtCore と QtGui を使う
      qtcling -i --widget          // QtWidgets を使う
      qtcling --no-gui             // QtGui を無効化し、QtCore だけで起動
      qtcling --test               // QtTest を追加
      qtcling --network            // QtNetwork を追加
      qtcling --sql                // QtSql を追加
      qtcling --xml                // QtXml を追加
      qtcling --xmlpatterns        // QtXmlPatterns を追加
      qtcling --multimedia         // QtMultimedia を追加
      qtcling --webengine          // QtWebEngine を追加
      qtcling --websockets         // QtWebSockets を追加
      qtcling --datavisualization  // QtDataVisualization を追加
      qtcling --positioning        // QtPositioning を追加
      qtcling --sensors            // QtSensors を追加
      qtcling --serialport         // QtSerialPort を追加

      QtCore を除き --no-* も用意する。

    * preload_qt_*.h の整理

      モジュール別のインクルードを用意し、qtcling のオプションに合わせて選択する。

    * run_* で日付比較

      元ファイルが更新されているか、生成されたファイルが古いかを比較して、必要なときだけ生成する。

## サービス

    * カスタムビルド

    * Cling 機能拡張

    * 開発支援コンサルティング

    * トレーニング

## デバッグ方法

## 問題点

    * macOS で ^Z をし fg すると終了してしまう。
      修正していない Cling でも発生。
      macOS 13 で発生、macOS 15 では発生しない。

    * 構文エラーで表示されるメッセージが多い
      大抵はメッセージの最初か最後の部分が手がかりになる。

    * Windows REPL で編集できない。

## ライセンス

    * 商用利用可能

      https://github.com/root-project/

    ** 再配布: 可能ですが、Cling/LLVM/第三者コードのライセンス表示を整理して同梱する必要がある

       https://llvm.org/LICENSE.txt

    ** 静的リンク/動的リンク:

       UI/NCSA で扱うなら、LGPL 的なリンク義務を中心に考える必要は小さい
       https://github.com/root-project/cling?utm_source=chatgpt.com

       LGPL で扱うなら、一般に動的リンクのほうが扱いやすい
       https://github.com/root-project/cling?utm_source=chatgpt.com
