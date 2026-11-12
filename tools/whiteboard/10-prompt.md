レビュー済み三点セットを上位仕様として、実装プロンプトを作成してください。

実装プロンプト（implementation_prompt.md）を作成してください。
テンプレートは app-generation-workflow/templates/implementation_prompt.md を使ってください。
作成先は対象アプリ用ディレクトリの prompts/implementation_prompt.md です。

実装方式、ビルド方式、テスト方式、UI条件、保存・永続化条件、対象外、実装前に確認すべき未決事項を記載してください。
実装時の作業ディレクトリは対象アプリ用ディレクトリとし、source_memo.md、specs/、implementation_constraints.md をその作業ディレクトリから参照するように記載してください。
対象アプリ用ディレクトリに implementation_constraints.md が存在する場合は、プロジェクト共通の実装制約として実装プロンプトへ参照を記載してください。
実装プロンプトが使える状態になっていない場合は、実装へ進まず警告してください。
