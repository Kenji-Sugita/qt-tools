# warikan User Guide

[日本語](USER_GUIDE.md) / [Project](README.en.md)

`warikan` is a static web application that calculates a shared bill using payment categories and tracks collection from the resulting participants. This distribution includes the working app, use-case/UI/business specifications, implementation instructions and reference material.

## Distribution File

- `warikan-release-20261108.zip`: the app, specifications, user guide and reference material.

Documentation/distribution version 1.0.1 aligns the distribution with the guide; the application and specifications are unchanged. The guide incorporates the November 13, 2026 clarification of storage and historical trial claims.

## Contents

- `src/index.html`: application entry point, opened in a browser.
- `src/style.css`: presentation styles.
- `src/app.js`: calculation, collection tracking and screen interactions.
- `docs/01_usecase_spec.md`: use-case definitions/specification.
- `docs/02_ui_spec.md`: UI specification.
- `docs/03_business_spec.md`: business-layer specification.
- `codex_prompt_without_zip.md`: implementation instructions for applying the three specifications in Codex.
- `USER_GUIDE.md` and `USER_GUIDE.pdf`: Japanese user guide.
- `USER_GUIDE.en.md`: this English guide, added for the public repository.
- `warikan.txt`: original explanatory memo.
- `warikan.png`: reference screen image.

## Prerequisites

A web browser is sufficient to run the app. No server, database or external library is required. Developers need Node.js for the JavaScript syntax check.

## Verification Flow

1. Extract `warikan-release-20261108.zip` and open its `warikan/` directory, or use this checkout.
2. Read the user guide.
3. Read `docs/01_usecase_spec.md`, `docs/02_ui_spec.md` and `docs/03_business_spec.md`.
4. Open `src/index.html` in a browser.
5. Check the representative calculation below.
6. Press `集金管理へ進む` (proceed to collection management) and check collection-state changes.

## Run

Open the following local file in a browser; no development server is needed:

```text
src/index.html
```

## Screens

The Japanese UI has two screens:

- `ワリカン計算` (bill calculation): enter the total, rounding unit and categories; calculate each category's per-person amount and subtotal.
- `集金管理` (collection management): create individual collection entries from the calculation and switch between collected and uncollected.

## Operation

### Bill Calculation

1. Open `ワリカン計算`.
2. Enter `会計総額` (total bill).
3. Enter `丸め単位` (rounding unit).
4. Enter `区分名` (name), `比率` (ratio) and `人数` (people) for each category.
5. Use `区分を追加` (add) or `削除` (delete) as needed.
6. Press `計算する` (calculate).
7. Review category amounts and total validation in `計算結果` (results).

### Collection Management

1. After calculating, press `集金管理へ進む`.
2. Review the participants in `集金管理`.
3. Press `集金済みにする` (mark collected) or `未集金に戻す` (mark uncollected) for an individual.
4. Check the planned total, collected amount and outstanding amount.

## Representative Example

The initial values are:

- Total: JPY 50,000; rounding unit: JPY 500.
- `多め` (higher share): ratio 1.5, two people.
- `普通` (normal share): ratio 1.0, six people.
- `少なめ` (lower share): ratio 0.5, two people.

Expected result:

- Higher: JPY 7,500 × 2 = JPY 15,000.
- Normal: JPY 5,000 × 6 = JPY 30,000.
- Lower: JPY 2,500 × 2 = JPY 5,000.
- Total: JPY 50,000; validation: OK.

## Storage and Changes to Inputs

Calculation results and collection records exist only in page memory. They are not saved/restored after reloading or closing the browser. Keep any records needed for real work separately.

Changing the total, rounding unit, adding/deleting categories, category names, ratios or people after calculation resets existing calculation and collection records.

## Developer Checks

Run the following from the `warikan` project directory:

```sh
node --check src/app.js
```

For logic checks, `src/app.js` exports `calculateWarikan`, `createCollectionMembers` and `summarizeCollection` through CommonJS `module.exports`. They can be called from Node.js.

## Regeneration from the Three Specifications

`warikan.txt` records an individual historical trial. Its duration and evaluation are not guarantees for current environments or regenerated results.

To regenerate or update the app, provide `codex_prompt_without_zip.md` to Codex. Those instructions limit implementation to `src/`, prohibit changes to the three specifications under `docs/`, and prohibit creation of a distribution ZIP.
