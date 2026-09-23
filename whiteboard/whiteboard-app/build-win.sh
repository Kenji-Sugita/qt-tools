# Windows build note

WhiteboardApp now includes its MCP server implementation in the application
source tree. An external qtmcpserver checkout is not required.

Configure and build with the Qt 6 environment selected:

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

The MCP targets still require Qt HttpServer and Qt Network. If configure fails,
check that the selected Qt installation provides those modules for the chosen
compiler kit.
