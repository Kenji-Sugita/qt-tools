// このコードは、C++17のフォールド式を使用して、パラメータパックにインデックス
// を付ける方法を示しています。

#include <iostream>

template<typename... TypePack>
void demo_pack_indexing(TypePack... values)
{
    std::cout << values...[0] << '\n';
    std::cout << values...[1] << '\n';
    std::cout << values...[2] << std::endl;
}
