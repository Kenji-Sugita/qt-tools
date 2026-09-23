// アプリ全体の実行順序をまとめる最上位層。
#pragma once

#include "clioptionsparser.h"
#include "googleapiclient.h"
#include "inputresolver.h"
#include "translationservice.h"

#include <QCoreApplication>

class TlApplication
{
public:
    int run(QCoreApplication &app);

private:
    void printError(const QString &message) const;

    CliOptionsParser m_cliOptionsParser;
    InputResolver m_inputResolver;
    GoogleTranslationProvider m_googleTranslationProvider;
    TranslationService m_translationService{m_googleTranslationProvider};
};
