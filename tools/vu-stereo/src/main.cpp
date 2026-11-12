// main.cpp は、Qt アプリケーションの起動だけを担当します。設定保存用の内部名や
// アイコンを設定し、最上位ウィンドウである VuApplicationWindow を表示します。
// VU メーターの UI 構築、設定、音声モニター接続は機能別のクラスへ分離しています。

#include "translationmanager.h"
#include "vuapplicationwindow.h"

#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName(QStringLiteral("SRA"));
    app.setOrganizationDomain(QStringLiteral("sra.co.jp"));
    // QSettings と画面収録権限の継続性のため、内部名は旧来の値を維持します。
    // ユーザーに見える表示名は TranslationManager が VuStereo に設定します。
    app.setApplicationName(QStringLiteral("vu-stereo"));
    app.setWindowIcon(QIcon(QStringLiteral(":/images/app-icon.svg")));

    TranslationManager translationManager;
    translationManager.setLanguage(AppLanguage::System);

    VuApplicationWindow window(&translationManager);
    window.show();

    return app.exec();
}
