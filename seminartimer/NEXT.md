# NEXT

このファイルを読む、または更新する前に、`~/AGENTS.next.md` を読み、その指示に従うこと。
`~/AGENTS.next.md` が存在しない場合は、このファイルを整理・削除せず、利用者に確認すること。
このルールは、ユーザーから明示的な指示がない限り変更しない。

## 現在の状態

- 2026-08-30 時点で、タイマー本体の基本 UI は動作している。
- `SET` ダイアログに `Always on top` を追加し、設定の保存と起動時復元まで実装済み。
- プリセット時間は `5 / 10 / 20 / 30` 分に変更済み。
- `USER_GUIDE.md` はプリセット変更と `.zip` がソース配布である点まで反映済み。
- Apple Store での販売可能性、需要、価格帯は会話で検討したが、実装や申請準備にはまだ着手していない。

## 次にやること

- `USER_GUIDE.md` に `Always on top` 設定を追記し、現行実装と説明を一致させる。
- Mac App Store に出す前提で不足している項目を洗い出す。

## 未完了

- App Store に出すか、ストア外配布を優先するかは未決。
- 販売する場合の価格戦略と、価格に見合う追加機能の有無は未決。
- 詳細な検討項目は `backlog.md` を参照。

## 触るファイル

- `USER_GUIDE.md`
- `main.cpp`
- `Info.plist.in`
- `CMakeLists.txt`
- `DECISIONS.md`
- `backlog.md`

## 注意

- 指示にない機能追加や仕様変更は、実装前に提案して許可を得る。
- `CMakeLists.txt` を変更する前に `~/AGENTS.cmake.md` を読む。
- `.zip` 配布物には `.app` は含まれず、ビルドに必要なソース一式のみを含む。
- 作業管理ファイルを更新する前に `~/AGENTS.next.md` を読む。

## 関連ファイル

- `main.cpp`
- `USER_GUIDE.md`
- `DECISIONS.md`
- `backlog.md`
- `AGENTS.md`
