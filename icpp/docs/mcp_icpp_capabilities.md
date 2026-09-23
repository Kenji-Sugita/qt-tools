# icpp MCP Server でできるようになったこと

icpp は、従来の対話型 REPL に加えて、ローカルの Streamable HTTP MCP Server として起動できるようになりました。
MCP client は HTTP 経由で cling または qtcling を操作し、C++ / Qt code の評価、session 状態の確認、interpreter の reset を実行できます。

通常の icpp 起動方法と対話 REPL の挙動は変わりません。MCP Server は `--mcp-http` を明示した場合だけ起動します。

## 起動

32 bytes 以上の認証 token を環境変数へ設定します。

```sh
export ICPP_MCP_TOKEN='replace-with-a-long-random-token'
```

状態確認だけを許可して起動する場合:

```sh
./build/icpp --mcp-http --mcp-port 8766
```

C++ / Qt code の評価も許可する場合:

```sh
./build/icpp \
    --mcp-http \
    --mcp-port 8766 \
    --mcp-allow-execution
```

cling を明示する例:

```sh
./build/icpp \
    --mcp-http \
    --mcp-port 8766 \
    --mcp-allow-execution \
    --engine cling \
    --cling /usr/local/src/cling/build/bin/cling
```

接続先は次の endpoint です。

```text
http://127.0.0.1:8766/mcp
```

`--mcp-port 0` を指定すると、利用可能な port が自動選択されます。実際の endpoint は起動時に標準エラーへ表示されます。

## Codex から接続する

icpp と Codex の起動環境へ、同じ token を設定します。すでに起動中の Codex process には後から `export` した環境変数が反映されないため、token と MCP Server を登録した後に Codex を起動し直します。

```sh
export ICPP_MCP_TOKEN='server-side-tokenと同じ32bytes以上の値'

codex mcp add icpp \
    --url http://127.0.0.1:8766/mcp \
    --bearer-token-env-var ICPP_MCP_TOKEN

codex mcp get icpp
codex mcp list
codex
```

登録内容は `~/.codex/config.toml` では次の形式になります。token 自体は設定ファイルへ保存せず、環境変数名だけを登録します。

```toml
[mcp_servers.icpp]
url = "http://127.0.0.1:8766/mcp"
bearer_token_env_var = "ICPP_MCP_TOKEN"
```

接続後は、まず Codex に `icpp/session/status` の実行を依頼します。`icpp/code/evaluate` と `icpp/session/reset` を使う場合は、icpp を `--mcp-allow-execution` 付きで起動しておく必要があります。

## 認証

すべての HTTP request に次の header が必要です。

```text
Authorization: Bearer <ICPP_MCP_TOKENの値>
```

token がない場合、32 bytes 未満の場合、または 4096 bytes を超える場合は MCP Server を起動しません。
token が一致しない HTTP request には `401 Unauthorized` を返します。

`ICPP_MCP_TOKEN` は、icpp が起動する cling / qtcling process の環境へ継承しません。

## 使用できる MCP tool

### `icpp/session/status`

現在の MCP session の状態を取得できます。

主な戻り値:

- 使用している engine: `cling` または `qtcling`
- interpreter process が起動中か
- code execution が許可されているか
- icpp の作業 directory
- interpreter の直近の起動 error

この tool は `--mcp-allow-execution` を指定していない場合も使用できます。

### `icpp/code/evaluate`

MCP session が保持する interpreter で C++ / Qt code を評価できます。

入力例:

```json
{
  "source": "#include <iostream>\nstd::cout << 2 + 3 << std::endl;",
  "timeoutMs": 5000
}
```

主な戻り値:

- 評価が完了したか
- timeout したか
- interpreter が引き続き起動しているか
- stdout
- stderr
- 出力が上限で切り詰められたか
- 評価時間
- process 起動や timeout に関する error

同じ MCP session 内では interpreter が維持されます。そのため、先の評価で定義した関数や変数を次の評価から使用できます。

```cpp
// 1回目
int add(int a, int b) { return a + b; }

// 2回目
#include <iostream>
std::cout << add(2, 3) << std::endl;
```

`{}` が対応していない source は interpreter へ送信せずに拒否します。
timeout または interpreter crash が発生した場合は子 process を終了し、次の評価時に新しい interpreter を起動します。

この tool を実行するには、icpp 起動時に `--mcp-allow-execution` が必要です。

### `icpp/session/reset`

現在の MCP session の interpreter を再起動できます。
それまでに評価した関数や変数の定義は消去されます。

この tool を実行するにも `--mcp-allow-execution` が必要です。

## HTTP session

- `POST /mcp` で initialize、notification、tool request を処理します。
- initialize response の `Mcp-Session-Id` で session を識別します。
- initialize 後の request では、同じ `Mcp-Session-Id` と `MCP-Protocol-Version` が必要です。
- `DELETE /mcp` で session と対応する interpreter process を終了できます。
- 無操作の session は 30 分で終了します。
- 同時に保持できる MCP session は 1 つです。
- MCP protocol version `2025-03-26` と `2025-06-18` に対応します。
- server 起点の SSE stream は提供していないため、`GET /mcp` は `405 Method Not Allowed` を返します。

## 制限

- HTTP request body: 最大 1 MiB
- 評価する source: 最大 1 MiB
- stdout: 最大 1 MiB
- stderr: 最大 1 MiB
- `timeoutMs`: 100 ms から 30000 ms
- 既定の評価 timeout: 5000 ms
- listen address: `127.0.0.1` のみ
- 許可する Origin: `localhost`、`127.0.0.1`、`::1`

## 安全上の注意

`icpp/code/evaluate` は sandbox ではありません。
評価した C++ / Qt code は、icpp process と同じ利用者権限で動作します。

認証済み MCP client から、次のような操作も原理的には実行できます。

- file の作成、変更、削除
- process の起動
- network 接続
- token 以外の環境変数の参照
- CPU や memory の大量消費
- interpreter の crash

`--mcp-allow-execution` は、信頼できるローカル MCP client と信頼できる code にだけ使用してください。
未信頼 code を扱う場合は、別利用者、container、VM など OS level の隔離が必要です。

## 検証済みの動作

自動テストでは次を確認しています。

- Bearer token がない request の拒否
- 外部 Origin の拒否
- initialize と session ID の発行
- 同時に2つ目のsessionを作成できないこと
- 3つのtoolの公開
- `--mcp-allow-execution` がない場合の評価拒否
- 同じsession内でC++定義が維持されること
- stdout / stderr の取得
- `ICPP_MCP_TOKEN` が interpreter から見えないこと
- 不完全な source の拒否
- timeout 時の process 終了
- timeout 後の次回評価で interpreter が回復すること
- reset と HTTP DELETE
- 従来の対話 REPL の回帰テスト
