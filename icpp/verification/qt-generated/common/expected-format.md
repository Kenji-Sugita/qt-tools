# expected.txt の書式

```text
生成されるファイル:
- moc_widget.cpp

出力に含まれるべき文字列:
- Added include path: .
- class Widget
- function Widget *go()

手動確認:
- `auto w = go(); w->show();` を手動実行すると GUI が表示される

メモ:
- ヘッダー、`.ui`、`.qrc` は `.e` で自動登録されない
```
