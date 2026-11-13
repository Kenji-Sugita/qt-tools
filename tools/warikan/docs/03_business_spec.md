# ワリカンアプリ ビジネスレイヤー仕様書

## 1. 目的

本書は、ワリカンアプリの計算ロジック、状態管理、データモデルを定義する。
UI 表示や操作部品の詳細は UI仕様書に従う。

## 2. 用語

| 用語 | 説明 |
| --- | --- |
| 会計総額 | ワリカン対象となる支払い総額。 |
| 丸め単位 | 1 人あたり金額を丸める単位。 |
| 負担区分 | 比率と人数を持つ参加者グループ。 |
| 比率 | 負担の重み。普通を 1.0 とした相対値。 |
| 重み付き人数 | 比率 × 人数 の合計。 |
| 基準単価 | 会計総額を重み付き人数で割った値。 |
| 1 人あたり金額 | 基準単価 × 比率 を丸め単位で丸めた金額。 |
| 小計 | 1 人あたり金額 × 人数。 |

## 3. データモデル

### 3.1 WarikanSession

ワリカン計算全体の状態を表す。

| フィールド | 型 | 必須 | 説明 |
| --- | --- | --- | --- |
| totalAmount | number | 必須 | 会計総額。1 以上の整数。 |
| roundingUnit | number | 必須 | 丸め単位。1 以上の整数。 |
| groups | PaymentGroup[] | 必須 | 負担区分の配列。 |
| calculationResult | CalculationResult \| null | 任意 | 計算結果。未計算時は null。 |
| collectionMembers | CollectionMember[] | 必須 | 集金対象者の配列。未生成時は空配列。 |

### 3.2 PaymentGroup

負担区分を表す。

| フィールド | 型 | 必須 | 説明 |
| --- | --- | --- | --- |
| id | string | 必須 | 画面上で区分を識別する ID。 |
| name | string | 必須 | 区分名。 |
| ratio | number | 必須 | 負担比率。0 より大きい数値。 |
| count | number | 必須 | 人数。1 以上の整数。 |

### 3.3 CalculationResult

計算結果全体を表す。

| フィールド | 型 | 必須 | 説明 |
| --- | --- | --- | --- |
| rows | CalculationRow[] | 必須 | 区分ごとの計算結果。 |
| totalCalculated | number | 必須 | 区分小計の合計。 |
| difference | number | 必須 | totalAmount - totalCalculated。 |
| isBalanced | boolean | 必須 | difference が 0 の場合 true。 |

### 3.4 CalculationRow

区分ごとの計算結果を表す。

| フィールド | 型 | 必須 | 説明 |
| --- | --- | --- | --- |
| groupId | string | 必須 | 対応する PaymentGroup の ID。 |
| groupName | string | 必須 | 区分名。 |
| ratio | number | 必須 | 負担比率。 |
| count | number | 必須 | 人数。 |
| amountPerPerson | number | 必須 | 1 人あたり金額。 |
| subtotal | number | 必須 | 区分小計。 |

### 3.5 CollectionMember

集金対象者を表す。

| フィールド | 型 | 必須 | 説明 |
| --- | --- | --- | --- |
| id | string | 必須 | 対象者 ID。 |
| label | string | 必須 | 表示名。例: `多め 1`。 |
| groupId | string | 必須 | 対応する PaymentGroup の ID。 |
| groupName | string | 必須 | 区分名。 |
| amount | number | 必須 | 集金額。 |
| collected | boolean | 必須 | 集金済みの場合 true。 |

### 3.6 CollectionSummary

集金状況の集計結果を表す。

| フィールド | 型 | 必須 | 説明 |
| --- | --- | --- | --- |
| plannedTotal | number | 必須 | 集金予定額合計。 |
| collectedTotal | number | 必須 | 集金済み合計。 |
| uncollectedTotal | number | 必須 | 未集金合計。 |
| collectedCount | number | 必須 | 集金済み人数。 |
| uncollectedCount | number | 必須 | 未集金人数。 |

## 4. 初期値

初期状態は次の通り。

```json
{
  "totalAmount": 50000,
  "roundingUnit": 500,
  "groups": [
    { "name": "多め", "ratio": 1.5, "count": 2 },
    { "name": "普通", "ratio": 1.0, "count": 6 },
    { "name": "少なめ", "ratio": 0.5, "count": 2 }
  ],
  "calculationResult": null,
  "collectionMembers": []
}
```

## 5. ワリカン計算ロジック

### 5.1 入力

計算ロジックは次を入力とする。

- totalAmount
- roundingUnit
- groups

### 5.2 入力検証

計算前に次を検証する。

| 対象 | 条件 |
| --- | --- |
| totalAmount | 1 以上の整数 |
| roundingUnit | 1 以上の整数 |
| groups | 1 件以上 |
| group.name | 空文字でない |
| group.ratio | 0 より大きい数値 |
| group.count | 1 以上の整数 |

検証に失敗した場合、計算結果を更新しない。

### 5.3 基準単価の算出

重み付き人数を次の式で算出する。

```text
weightedCount = sum(group.ratio * group.count)
```

基準単価を次の式で算出する。

```text
baseAmount = totalAmount / weightedCount
```

### 5.4 1 人あたり金額の算出

各区分の丸め前金額を次の式で算出する。

```text
rawAmountPerPerson = baseAmount * group.ratio
```

丸め単位に従って、1 人あたり金額を次の式で算出する。

```text
amountPerPerson = round(rawAmountPerPerson / roundingUnit) * roundingUnit
```

`round` は四捨五入を意味する。

### 5.5 小計の算出

区分ごとの小計を次の式で算出する。

```text
subtotal = amountPerPerson * group.count
```

### 5.6 合計検証

計算結果合計を次の式で算出する。

```text
totalCalculated = sum(row.subtotal)
```

会計総額との差額を次の式で算出する。

```text
difference = totalAmount - totalCalculated
```

合計検証は次の条件で判定する。

```text
isBalanced = difference === 0
```

### 5.7 差額調整

丸めにより `difference` が 0 にならない場合、差額調整を行う。

調整対象は、比率が最も大きい負担区分とする。
比率が同じ区分が複数ある場合は、表示順が先の区分を対象とする。

調整方法は次の通り。

```text
adjustedSubtotal = originalSubtotal + difference
adjustedAmountPerPerson = adjustedSubtotal / group.count
```

ただし、`adjustedAmountPerPerson` が整数にならない場合は、1 人あたり金額を均等に表示できないため、差額調整を行わず `difference` を表示する。

調整後に `totalCalculated`、`difference`、`isBalanced` を再計算する。

## 6. 計算例

入力:

| 項目 | 値 |
| --- | ---: |
| 会計総額 | 50,000 |
| 丸め単位 | 500 |

負担区分:

| 区分名 | 比率 | 人数 |
| --- | ---: | ---: |
| 多め | 1.5 | 2 |
| 普通 | 1.0 | 6 |
| 少なめ | 0.5 | 2 |

計算:

```text
weightedCount = 1.5 * 2 + 1.0 * 6 + 0.5 * 2
              = 10

baseAmount = 50000 / 10
           = 5000

多め   = round((5000 * 1.5) / 500) * 500 = 7500
普通   = round((5000 * 1.0) / 500) * 500 = 5000
少なめ = round((5000 * 0.5) / 500) * 500 = 2500
```

結果:

| 区分名 | 1人あたり | 人数 | 小計 |
| --- | ---: | ---: | ---: |
| 多め | 7,500 | 2 | 15,000 |
| 普通 | 5,000 | 6 | 30,000 |
| 少なめ | 2,500 | 2 | 5,000 |

```text
totalCalculated = 50000
difference = 0
isBalanced = true
```

## 7. 集金対象者生成ロジック

### 7.1 入力

集金対象者生成は CalculationResult を入力とする。

### 7.2 生成ルール

CalculationRow ごとに `count` 件の CollectionMember を生成する。
表示名は次の形式とする。

```text
{groupName} {連番}
```

例:

```text
多め 1
多め 2
普通 1
普通 2
```

各 CollectionMember の `amount` は対応する CalculationRow の `amountPerPerson` とする。
各 CollectionMember の `collected` 初期値は false とする。

## 8. 集金サマリー算出ロジック

CollectionMember 配列から次を算出する。

```text
plannedTotal = sum(member.amount)
collectedTotal = sum(member.amount where member.collected === true)
uncollectedTotal = plannedTotal - collectedTotal
collectedCount = count(member where member.collected === true)
uncollectedCount = count(member where member.collected === false)
```

## 9. 状態リセット

計算済みまたは集金管理中に次の入力が変更された場合、計算結果と集金対象者をリセットする。

- 会計総額
- 丸め単位
- 負担区分の追加
- 負担区分の削除
- 区分名
- 比率
- 人数

リセット後の状態:

```json
{
  "calculationResult": null,
  "collectionMembers": []
}
```

## 10. 表示用フォーマット

### 10.1 金額

金額は 3 桁区切りの整数とし、末尾に `円` を付ける。

例:

```text
50000 -> 50,000円
7500 -> 7,500円
```

### 10.2 人数

人数は整数の末尾に `人` を付ける。

例:

```text
2 -> 2人
```

