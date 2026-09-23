# App Store 販売価格メモ

## 推奨価格

`VuStereo` を Mac App Store で販売する場合、初期価格は次を推奨する。

```text
日本: 500 円前後
米国: US $2.99 相当
```

少し強気に設定する場合は、次の価格帯も候補になる。

```text
日本: 800 円前後
米国: US $4.99 相当
```

現時点では、まず低めに出して反応を見る方針がよい。

## 理由

`VuStereo` は、macOS で再生中のシステム音声を左右 2 チャンネルの VU メーターとして
表示する小さな常駐ツールである。

見た目はよいが、現状では業務用のラウドネスメーターではなく、シンプルなデスクトップ
VU 表示アプリとして位置づけるのが自然である。

そのため、最初から高めの価格にするよりも、買いやすい価格にして導入のハードルを
下げる方がよい。

## 類似アプリの価格感

確認時点での類似アプリの価格感は次の通り。

- シンプルな Mac 用 VU メーター: US $2.99 程度
- VU / Peak メーター系の軽量ツール: US $5.99 程度
- LUFS、True Peak、broadcast 対応などを含む高機能メーター: US $9.99 から US $17.99 以上

`VuStereo` は現状、最初のカテゴリに近い。

## 高めの価格にする場合に欲しい機能

US $4.99 以上を狙う場合は、次のような機能や整備があるとよい。

- ウィンドウ位置の保存
- 常に前面表示の ON/OFF
- 音声入力方式の説明強化
- App Store 用スクリーンショット
- プライバシーポリシー
- 安定した App Store 署名
- 画面収録権限に関するアプリ内説明

## App Store 販売時の注意

有料販売には Apple Developer Program への登録と、App Store Connect での Paid Apps
Agreement の承認が必要である。

価格は App Store Connect の price point から選ぶ。基準国または地域の価格を設定すると、
他の国や地域の価格は Apple が為替や税を考慮して自動設定できる。

## 参考

- Apple App Store Connect Help: Set a price
  https://developer.apple.com/help/app-store-connect/manage-app-pricing/set-a-price
- Apple Developer: In-App Purchase
  https://developer.apple.com/in-app-purchase/
- Audio Meter VU
  https://apps.apple.com/us/app/audio-meter-vu/id6785263314?mt=12
- VU Deck
  https://apps.apple.com/us/app/vu-deck/id6742258277?mt=12
