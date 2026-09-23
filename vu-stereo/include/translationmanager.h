// TranslationManager は、アプリケーション全体の表示言語を切り替えるための
// 小さな管理クラスです。英語はソース文字列をそのまま使い、日本語は Qt の翻訳
// リソースを QTranslator で読み込みます。System は QLocale::system() に従って、
// 実行中でも英語または日本語へ解決します。

#pragma once

#include <QObject>
#include <QTranslator>

enum class AppLanguage {
    System,
    English,
    Japanese,
};

class TranslationManager final : public QObject {
    Q_OBJECT

public:
    explicit TranslationManager(QObject *parent = nullptr);

    AppLanguage language() const;
    void setLanguage(AppLanguage language);
    QString applicationName() const;

signals:
    void languageChanged();

private:
    AppLanguage resolvedLanguage() const;

    QTranslator m_translator;
    AppLanguage m_language = AppLanguage::System;
    AppLanguage m_installedLanguage = AppLanguage::English;
    bool m_initialized = false;
};
