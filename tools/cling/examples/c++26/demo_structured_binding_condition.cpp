// C++17で導入された構造化束縛は、変数宣言と同時に複数の値を分解して代入する機
// 能です。条件付き構造化束縛は、if 文や while 文の条件式内で構造化束縛を使用
// することができます。これにより、条件が満たされた場合にのみ変数が宣言され、
// スコープ内で使用できるようになります。例えば、std::mapのfind関数を使用して
// 特定のキーを検索し、その結果を構造化束縛で受け取ることができます。条件付き
// 構造化束縛を使用することで、コードがより簡潔で読みやすくなります。以下は、
// 条件付き構造化束縛を使用してstd::mapから特定のキーを検索し、その値を出力す
// る例です。

#include <iostream>
#include <map>

void demo_structured_binding_condition()
{
    std::map<int, const char*> dictionary{
        {1, "one"},
        {2, "two"},
        {3, "three"}
    };

    if (auto iterator = dictionary.find(2); iterator != dictionary.end()) {
        auto& [key, value] = *iterator;
        std::cout << key << " => " << value << '\n';
    }
    std::cout << std::flush;
}
