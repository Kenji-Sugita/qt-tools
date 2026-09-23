// C++17 では、構造化バインディングでプレースホルダ変数（通常は '_' と名付けら
// れる）を使用して、特定の要素を無視することができます。この例では、タプルの2
// 番目の要素を無視しながら、最初と3番目の要素を抽出します。プログラムを実行す
// ると、名前とスコアが出力されますが、年齢（2番目の要素）は無視されます。出力
// は以下の通りです。

#include <iostream>
#include <tuple>

void demo_placeholder_variable()
{
    auto [name, _, score] = std::tuple{"Taro", 20, 95};

    std::cout << "name = " << name << '\n';
    std::cout << "score = " << score << std::endl;
}
