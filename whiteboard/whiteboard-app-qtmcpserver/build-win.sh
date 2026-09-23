原因は QtMcpServer のソースが見つからないことです。

```text
QtMcpServer source was not found at /usr/local/qt/qtmcpserver
```

`CMakeLists.txt` の既定値が macOS 用のままで、Windows 上にも `qtmcpserver` は見つかりませんでした。`pthread` や Vulkan の表示は今回の致命エラーではありません。

QtMcpServer を任意の場所へ配置した後、次のように指定します。

```powershell
cmake -B build `
  -DQTMCPSERVER_SOURCE_DIR=C:\path\to\qtmcpserver
```

cmake -B build -DQTMCPSERVER_SOURCE_DIR=/c/Program\ Files/Git/usr/local/qt/qtmcpserver/

指定先には `src\CMakeLists.txt` が必要です。既存の失敗した `build` は同じ generator/Qt 構成なので、配置後はそのまま再configureできます。

現時点では診断のみ行い、ソースや CMake ファイルは変更していません。
