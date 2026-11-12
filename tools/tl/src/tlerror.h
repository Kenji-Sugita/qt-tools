// tl のエラー表現をまとめる共通定義。
#pragma once

#include <QString>

enum class TlErrorCode {
    InvalidArguments,
    EmptyInput,
    FileReadFailed,
    MissingCredentials,
    AuthFailed,
    NetworkFailed,
    UnsupportedLanguage,
    InvalidResponse
};

struct TlError {
    TlErrorCode code = TlErrorCode::InvalidArguments;
    QString message;
};
