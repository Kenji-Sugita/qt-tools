# Executable Demo Results

## 対象

この文書は、実行形式デモ `localizedclockdemo` の結果を要約する。

- executable: `build/localizedclockdemo`
- trace: `samples/generated/localizedclockdemo.qtd`
- source QML: `demos/localizedclock/Main.qml`

詳細なコマンド出力全文は `docs/EXECUTABLE_DEMO_OUTPUTS.md` を参照。

## 確認できたこと

- 実行形式アプリから `.qtd` トレースを生成できる
- 生成トレースを `qmlprofileranalyzer` で読める
- `Compiling`, `Creating`, `Binding`, `Javascript`, `MemoryAllocation`,
  `SceneGraph`, `AnimationFrame`, `DebugMessage` を含むトレースを読める
- `qml` デモで見えていた `default.qml` 系 event は出ていない

## 結果の要点

- 実行時間
  - `localizedclockdemo` は約15秒で自動終了
  - `traceDuration=15499169084`
- `version=1.02`
- `eventTypes=37`
- `events=441`
- `ranges=106`
- `pointEvents=335`
- `notes=0`
- 最長 range
  - `Main.qml:43`
  - `duration=472956334`
- 繰り返し更新
  - `Main.qml:56 count=15`
  - `Main.qml:32 count=16`

## 読み取れること

- 実行形式デモは `qml` デモより実運用に近い
- `Text` と binding / javascript 更新が目立つ
- `Timer` の `onTriggered` 処理に対応する event を15回観測できる
- `default.qml` 層が無い分、`qml` デモより構造が素直

## 現時点の結論

- 実行形式デモは成立している
- `qml` デモと並べて使うことで、runtime launcher 由来の差を比較できる
- 今後は、この executable demo 用にも手順書と評価文を保守していける状態になった
