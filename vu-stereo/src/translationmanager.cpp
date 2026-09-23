// TranslationManager の実装です。アプリケーションに現在入っている翻訳を外してから、
// 必要な場合だけ日本語翻訳をインストールします。Qt は翻訳変更時に LanguageChange
// イベントを各ウィジェットへ送るため、各 UI クラスはそのイベントで表示文字列を
// 再設定します。

#include "translationmanager.h"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QLocale>

TranslationManager::TranslationManager(QObject *parent)
    : QObject(parent)
{
}

AppLanguage TranslationManager::language() const
{
    return m_language;
}

void TranslationManager::setLanguage(AppLanguage language)
{
    const AppLanguage previousLanguage = m_language;
    m_language = language;
    const AppLanguage resolved = resolvedLanguage();
    if (m_initialized && previousLanguage == m_language && m_installedLanguage == resolved) {
        return;
    }

    if (m_installedLanguage == AppLanguage::Japanese) {
        qApp->removeTranslator(&m_translator);
    }

    m_installedLanguage = AppLanguage::English;
    if (resolved == AppLanguage::Japanese && m_translator.load(QStringLiteral(":/i18n/vu-stereo_ja.qm"))) {
        qApp->installTranslator(&m_translator);
        m_installedLanguage = AppLanguage::Japanese;
    }

    QGuiApplication::setApplicationDisplayName(applicationName());
    m_initialized = true;
    emit languageChanged();
}

QString TranslationManager::applicationName() const
{
    return QCoreApplication::translate("Application", "VuStereo");
}

AppLanguage TranslationManager::resolvedLanguage() const
{
    if (m_language != AppLanguage::System) {
        return m_language;
    }

    if (QLocale::system().language() == QLocale::Japanese) {
        return AppLanguage::Japanese;
    }

    return AppLanguage::English;
}
