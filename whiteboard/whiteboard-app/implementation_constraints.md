# ホワイトボードアプリ 共通実装制約

このファイルは、分割した仕様化ワークフロー全体に共通する実装制約の正本である。
個別の `source_memo.md` へ同じ制約を重複記述せず、実装プロンプト生成時にこの内容を参照または反映する。

## 環境依存の互換性

- OS、コンパイラ、Qt のプラットフォーム実装差によって挙動やビルド結果が変わる箇所は、対象環境、原因、補正範囲、既存仕様への影響を明示すること。
- 環境依存の回避策は、必要な環境にだけ適用し、他の環境の既存挙動を変えないこと。
- 回避策は仕様変更として扱わず、既存仕様を維持するための実装制約として扱うこと。
- 環境依存の回避策は、可能な限り UI 実装や業務ロジックへ直接散らさず、専用の小さな互換性ヘルパーまたはライブラリへ集約すること。
- 既存の互換性ヘルパーまたはライブラリがある場合、同種の環境依存処理はそこを再利用し、再生成時に削除しないこと。
- macOS/Linux で生成・確認したコードでも、Windows/MSVC でそのままビルドまたは同一挙動になるとは仮定しないこと。環境差が疑われる箇所は Windows/MSVC でも確認すること。
- Windows/MSVC でもビルドできること。
- macOS では `CMAKE_OSX_ARCHITECTURES` の既定値を `arm64;x86_64` にして
  universal binary をビルドすること。使用する Qt も同じアーキテクチャを含む必要がある。
- universal binary 設定は macOS の CMake 設定に限定し、Windows/Linux のビルドへ持ち込まないこと。
- `QColor`、`QFont` など Qt 型のクラス静的既定値は、`static const Type name;` の宣言だけにしない。
- 既定値をクラス定数として公開する場合は、C++17 の `inline static const` としてヘッダ内で初期化する。

## Qt Widgets のレイアウト項目矩形

- macOS の Qt Widgets では、ウィジェット矩形とレイアウト項目矩形が異なる場合があることを
  前提にする。
- 実ウィジェット矩形を基準に独自計算するツールバーでは、macOS に限り、管理対象の操作部品へ
  `Qt::WA_LayoutUsesWidgetRect` を共通登録処理で設定し、レイアウト項目矩形を正規化する。
- 独自計算した項目幅は行分割の判断だけに使い、パネルの最終幅は項目配置後の外側
  `QVBoxLayout::sizeHint()` とフレーム幅から決める。必要幅を超える固定幅をレイアウトへ渡して
  `qGeomCalc()` の暗黙の余剰空間配分を発生させない。
- 折り返し行はスペーサーを持たない `QHBoxLayout` とし、短い行は左寄せにする。複数行で列幅を共有する
  `QGridLayout` は拡張パネルに使用しない。
- オフスクリーンテストだけではネイティブスタイルのレイアウト項目余白を再現できない場合がある。
  共通ロジックのテストに加え、macOS の Cocoa 環境でも実ウィジェット座標基準の左右余白を確認する。

## MCP Server

- MCP 対応は WhiteboardApp 専用の最小実装で提供し、外部 qtmcpserver ソースツリーには依存しない。
- `WhiteboardMcpController` は `initialize`、`tools/list`、`tools/call`、
  `notifications/initialized` の必要範囲だけを扱う JSON-RPC dispatcher とする。
- 通常起動時の MCP トランスポートは Qt HttpServer による Streamable HTTP とし、
  `127.0.0.1:8765/mcp` だけで待ち受ける。localhost 系以外の Origin は拒否する。
- `--mcp` 起動時は互換用の標準入出力トランスポートだけを開始し、stdout へ MCP JSON 以外の
  ログを出力しない。
- 読み取りツールは承認不要、文書変更ツールは Qt GUI のユーザー承認を必須とする。
- MCP 対応のために既存保存 JSON の schema version やキーを変更しない。
- バージョンは `whiteboard-app/VERSION` を正本とし、semantic version `major.minor.patch` 形式だけを
  受け付ける。リリース内容は `whiteboard-app/CHANGELOG.md` に記録する。
- リポジトリ直下の `whiteboard-app.zip` は Git 管理外のローカル確認用生成物とする。
  正式な source release ZIP は `whiteboard-app/release/whiteboard-app-<version>-source.zip` に
  作成し、ZIP 内トップ階層は `whiteboard-app-<version>/` とする。`DECISIONS.md`、`NEXT.md`、
  `backlog.md`、`docs/`、`release/` は source release ZIP に含めない。

## Windows の QColorDialog と塗り潰し色

- Windows では、`QColorDialog::getColor()` に alpha 0 の初期色を渡し、`QColorDialog::ShowAlphaChannel` を指定すると、ユーザーが色を選んでも alpha 0 のまま返る場合がある。
- 塗り潰し色選択では、Windows で初期塗り潰し色が透明の場合、ダイアログへ渡す初期色は視認可能な alpha に補正すること。
- 補正は `QColorDialog::getColor()` に渡す初期色にのみ行い、ダイアログから返った色は alpha 0 を含めてそのまま保存すること。
- 返却色の alpha 0 を一律に 255 へ変更してはならない。ユーザーが透明を選んだ可能性を保持すること。
- Windows 以外ではこの補正を行わないこと。
- 透明を選ぶ挙動は維持すること。透明を指定した結果として alpha 0 を保存するのは許可する。
- この制約は Windows 固有の `QColorDialog` 挙動差への対処であり、塗り潰し透明を「塗り潰しなし」と扱う既存仕様は変更しない。
- 塗り潰し色ダイアログの Windows 補正は `PlatformColorDialog` に集約し、UI 側に同等のプラットフォーム分岐を重複実装しないこと。
