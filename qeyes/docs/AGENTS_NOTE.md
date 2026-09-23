# AGENTS.md の記述量について

現実のプロジェクトでも、このくらい、あるいはもっと長い `AGENTS.md` や運用文書は普通にあります。

確認できた範囲では、かなり幅があります。

- `agentsmd/agents.md` の公開説明は、`AGENTS.md` の最小例として数セクションの短い例を示しています。つまり「短く始める」運用は一般的です。  
  Source: https://github.com/agentsmd/agents.md
- `openai/codex` の実際の `AGENTS.md` は GitHub 上で `211 lines`、`16.5 KB` と表示されています。かなり具体的な実装・テスト・設計規約まで入っています。  
  Source: https://github.com/openai/codex/blob/main/AGENTS.md
- Codex の issue では、ある利用者が `AGENTS.md` は `around 40kb` と述べており、かなり大きい実運用例もあります。  
  Source: https://github.com/openai/codex/issues/7138
- GitHub 公式も、`CONTRIBUTING.md` のようなガイドライン文書をリポジトリの `root`、`docs`、`.github` に置いて contributors に示す運用を案内しています。つまり、ルールをファイル化して持つこと自体が一般的です。  
  Source: https://docs.github.com/en/communities/setting-up-your-project-for-healthy-contributions/setting-guidelines-for-repository-contributors

## 結論

- 今の `qeyes` の記述量は多すぎるとは言えません
- 小規模プロジェクトとしてはやや丁寧ですが、不自然ではありません
- 実務では「短い最低限」から始めるケースもあれば、`openai/codex` のようにかなり長く具体的に書くケースもあります

## 実用上の基準

長さそのものではなく、次の点が重要です。

- 実際に守ってほしい規則だけが入っているか
- 重複や矛盾がないか
- 変更時に更新し続けられる量か

今の `qeyes` は、その観点では十分現実的な範囲です。
