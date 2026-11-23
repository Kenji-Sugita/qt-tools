qtcling ソースツリー
=====================

このディレクトリには、Qt を対話的に利用できる Cling の C++ REPL 環境である
qtcling のソース一式を置いています。現在の利用者向けリリース対象は、端末版の
`qtcling` interactive です。導入方法と使い方は `USER_GUIDE.md` を参照してください。

トップレベルのファイル
------------------------

USER_GUIDE.md   qtcling 1.2.0 の利用者ガイドです。
USER_GUIDE.pdf  qtcling 1.2.0 の利用者ガイド PDF です。
build.sh        macOS/LinuxでCling/LLVMの取得、patch適用、設定、ビルドを行います。
install.sh      macOS/Linuxのビルド結果とqtcling launcherをインストールします。
install-win.ps1 Windows 11 ARM64版をユーザーのlocal application dataへインストールします。
build-all.sh    準備済みの Cling/LLVM ソースツリーをビルドするスクリプトです。
build-win.sh    Windows 11 ARM64 ネイティブ版を -j 2 でビルド・インストールします。
build-libedit-win.ps1
                Windows 11 ARM64用portable libeditをビルド・インストールします。
QTCLING_VERSION qtcling の release version を保持します。
NEXT.md         次の開発セッションへ引き継ぐための短い作業メモです。
memo.md         開発経緯とトラブルシューティングの記録です。

トップレベルのディレクトリ
--------------------------

bin/            利用者向けの起動スクリプトと補助ツールを置きます。
docs/           設計メモ、リリース計画、サンプル、開発資料を置きます。
examples/       Cling/qtcling で利用する C++ と Qt のサンプルプログラムです。
misc/           実験用資料、プラットフォーム別のメモ、作業メモを置きます。
patch/          Cling、LLVM、ラッパー用の必須パッチと参照用パッチを置きます。
qtcling-gui/    実験的な GUI フロントエンドです。1.2.0 の利用者向け対象外です。
release/        配布用 source zip を置きます。zip 作成用 staging は一時ディレクトリで行います。
sessions/       日付ごとの開発セッション記録を置きます。
src/            REPL が読み込む qtcling の起動コードとコールバック実装です。
windows/        Windows 11 ARM64用Qt callback DLLのCMake projectです。

ローカル設定・管理用ディレクトリ
------------------------------

.git/           Git リポジトリの管理情報です。
.agents/         ローカルのエージェント作業環境に関する管理情報です。
.codex/          ローカルの Codex 設定と管理情報です。

注意
----

ビルドを実行すると、`build/`、`cling/`、`llvm-project/` のような大きなローカル
ディレクトリが作成される場合があります。これらは存在しない場合は上の一覧に含めて
おらず、必要性を確認せずに削除しないでください。
