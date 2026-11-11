# Interactive Examples

Start icpp with Qt Cling from the `anker-layout/` directory.
The `iccp/` directory name is retained from the original distribution.

```bash
icpp --engine qtcling
```

This example places two buttons diagonally:

```text
icpp[qtcling]> .i include
icpp[qtcling]> .add src/AnkerFormAttachment.cpp
icpp[qtcling]> .add src/AnkerFormLayout.cpp
icpp[qtcling]> .add iccp/diagonal.cpp
icpp[qtcling]> .files
icpp[qtcling]> .r
icpp[qtcling]> .x static auto w = go();
```

Use `.files` to check the three registered files, evaluate them with `.r`, then call `go()`.
To try the basic form, start a new session and register `iccp/anker_icpp.cpp` instead of `iccp/diagonal.cpp`.
Both files define `go()`, so do not register them together.

[日本語](README.md) / [Anker Layout](../README.en.md)
