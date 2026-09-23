# Google Cloud Translation 継続無料利用手順

最終確認日: 2026-06-24

## 概要

Google Cloud Translation は、2026-06-24 時点で通常の Translation API (NMT) に対して毎月 500,000 文字まで無料枠があります。

このファイルは、期間限定の Free Trial ではなく、継続利用できる Free Tier だけを前提にしています。

ただし、Free Tier を使う場合でも Google Cloud の Billing Account は必要です。

## 事前に理解しておくこと

- Free Tier は月次の無料枠です。未使用分は繰り越されません。
- 無料枠を超えた分は課金されます。
- Google Cloud Free Tier を使うには Billing Account が必要です。
- Billing Account の作成時にはクレジットカードなどの支払い方法登録が必要になることがあります。
- Free Trial は必須ではありません。このファイルでは扱いません。

## 手順

1. Google Cloud のアカウントを用意する
2. Billing Account を作成する
3. Translation API を使うためのプロジェクトを作成する
4. そのプロジェクトで Cloud Translation API を有効化する
5. Service Account を作成する
6. Service Account に Cloud Translation API User 権限を付ける
7. Service Account の JSON キーを保存する
8. `GOOGLE_APPLICATION_CREDENTIALS` を設定する
9. 少量のテキストで疎通確認する

## 詳細手順

### 1. Billing Account を確認する

- Billing が有効な状態になっていることを確認する
- 今後の超過課金を避けたい場合は、予算アラートの設定も行う

### 2. プロジェクトを作成する

例:

- Project name: `tl-translate`

このプロジェクトに対して Translation API を有効化します。

### 3. Cloud Translation API を有効化する

- Google Cloud Console で対象プロジェクトを開く
- `APIs & Services` に移動する
- `Cloud Translation API` を検索する
- `Enable` を実行する

### 4. Service Account を作成する

CLI から使うなら、まずは Service Account が扱いやすいです。

- Service Account を作成する
- Service Account 名は例として `tl-translate-cli` とする

### 5. Service Account に権限を付ける

この `tl` は Cloud Translation Advanced API (v3) の次のエンドポイントを使います。

- `detectLanguage`
- `translateText`

Service Account には、まず `Cloud Translation API User`
(`roles/cloudtranslate.user`) を付けます。

管理者権限の `Cloud Translation API Admin` は、通常の翻訳実行だけなら不要です。

### 6. JSON キーを作成する

ローカル開発時は、この JSON キーを環境変数で指定して使う構成が簡単です。

Google Cloud Console で次の操作を行います。

- `IAM & Admin` -> `Service Accounts` を開く
- 作成した Service Account を開く
- `Keys` タブを開く
- `Add key` -> `Create new key` -> `JSON` を選ぶ
- JSON キーをダウンロードする

JSON キーは秘密鍵です。Git に追加しないでください。

### 7. 認証情報をローカルに配置する

例:

```bash
mkdir -p "$HOME/.config/tl"
mv "$HOME/Downloads/ダウンロードしたキー.json" \
  "$HOME/.config/tl/google-translation-service-account.json"
chmod 600 "$HOME/.config/tl/google-translation-service-account.json"
```

### 8. 環境変数を設定する

この `tl` の実装は `GOOGLE_APPLICATION_CREDENTIALS` を読みます。

```bash
export GOOGLE_APPLICATION_CREDENTIALS="$HOME/.config/tl/google-translation-service-account.json"
```

毎回使う場合は、利用しているシェルの設定ファイルに追記します。

zsh の例:

```bash
echo 'export GOOGLE_APPLICATION_CREDENTIALS="$HOME/.config/tl/google-translation-service-account.json"' >> "$HOME/.zshrc"
```

bash の例:

```bash
echo 'export GOOGLE_APPLICATION_CREDENTIALS="$HOME/.config/tl/google-translation-service-account.json"' >> "$HOME/.bashrc"
```

確認:

```bash
echo "$GOOGLE_APPLICATION_CREDENTIALS"
test -r "$GOOGLE_APPLICATION_CREDENTIALS" && echo OK
```

### 9. ローカル前提を確認する

この `tl` は JWT 署名に `/opt/local/bin/openssl` を使います。

```bash
/opt/local/bin/openssl version
```

このファイルがない場合は、実行時に次のエラーになります。

```text
tl: openssl executable not found at /opt/local/bin/openssl
```

### 10. 疎通確認する

```bash
cd /Users/sugita/src/qt/tools/tl
./build/tl "今日は会議があります。"
./build/tl "Please check this file."
```

認証情報が未設定の場合は、次のエラーになります。

```text
tl: GOOGLE_APPLICATION_CREDENTIALS is not set
```

### 11. 課金暴走を避けるための設定を入れる

- Budget を設定する
- Alert を設定する
- 必要なら上限に近づいたときに通知されるようにする

注意:
Google Cloud の Budget は通知用途が中心です。強制停止ではない点に注意してください。

### 12. 翻訳 CLI 実装で必要になる情報

この `tl` の実装が JSON キーから読む値は次の通りです。

- `client_email`
- `private_key`
- `project_id`
- `token_uri`

`token_uri` が空の場合は、実装側で `https://oauth2.googleapis.com/token` を使います。

ローカル環境として必要なのは次の 2 つです。

- `GOOGLE_APPLICATION_CREDENTIALS`
- `/opt/local/bin/openssl`

## コマンド利用イメージ

現在の `tl` は、日本語と英語を自動判定して相互翻訳します。

```bash
./build/tl "今日は会議があります。"
```

```bash
echo "Please check this file." | ./build/tl
```

## 料金と無料枠の整理

2026-06-24 時点で確認した認識:

- Translation API (NMT): 毎月 500,000 文字まで無料
- 無料枠超過後: 従量課金
- Free Tier は月単位で計算
- 未使用分の無料枠は翌月へ繰り越されない

実際の料金や制度変更は Google 公式ページを再確認すること。

## 公式ページ

- Pricing: https://cloud.google.com/translate/pricing
- Setup: https://docs.cloud.google.com/translate/docs/setup
- IAM roles: https://docs.cloud.google.com/translate/docs/access-control
- Service Account keys: https://docs.cloud.google.com/iam/docs/keys-create-delete
- Product overview: https://cloud.google.com/translate
- Google Cloud Free Tier: https://docs.cloud.google.com/free/docs/free-cloud-features

## 実装前の確認項目

- Billing Account を作成済みか
- Translation API を有効化済みか
- Service Account の JSON キーを取得済みか
- 無料枠超過時に課金されてもよいか
- Budget / Alert を設定したか
