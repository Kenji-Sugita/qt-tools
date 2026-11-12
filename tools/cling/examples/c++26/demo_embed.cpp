// このコードは、#embedディレクティブを使用してテキストファイルの内容を直接コ
// ンパイルされたバイナリに含める方法を示しています。"message.txt"の内容は、符
// 号なし文字の配列として埋め込まれ、その後コンソールに出力されます。コンパイ
// ルして実行する前に、このソースコードと同じディレクトリに"message.txt"ファイ
// ルを作成し、いくつかのテキスト内容を追加してください。message.txt"の内容が
// コンソールに表示されるのを確認してください。

#include <iostream>

const unsigned char message[] = {
#embed "message.txt"
};

void demo_embed()
{
    for (unsigned char ch : message) {
        std::cout << ch;
    }
    std::cout << std::endl;
}
