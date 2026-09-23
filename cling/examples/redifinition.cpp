#include <print>

// REPL でのみ設定できる。ファイルで設定しても再定義エラーになる。
gClingOpts->AllowRedefinition = true;

int f() { return 0; };
int f() { return 1; };

void redifinition()
{
    std::println("{}", f());
}
