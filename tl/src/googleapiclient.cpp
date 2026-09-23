// Google API 呼び出しの詳細を隠蔽するプロバイダ実装。

#include "googleapiclient.h"

#include <QDateTime>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QScopeGuard>
#include <QTemporaryFile>
#include <QUrl>
#include <QUrlQuery>

namespace {

constexpr auto kTranslationScope = "https://www.googleapis.com/auth/cloud-translation";

struct ServiceAccountConfig {
    QString clientEmail;
    QString privateKey;
    QString projectId;
    QString tokenUri;
};

struct NetworkResult {
    int httpStatus = 0;
    QByteArray body;
    QString errorString;
};

TlError makeError(TlErrorCode code, const QString &message)
{
    return {code, message};
}

QByteArray base64Url(const QByteArray &input)
{
    return input.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
}

Result<ServiceAccountConfig> loadServiceAccountConfig()
{
    const QString credentialPath = qEnvironmentVariable("GOOGLE_APPLICATION_CREDENTIALS");
    if (credentialPath.isEmpty()) {
        return Result<ServiceAccountConfig>::failure(
            makeError(TlErrorCode::MissingCredentials,
                      QStringLiteral("GOOGLE_APPLICATION_CREDENTIALS is not set")));
    }

    QFile file(credentialPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return Result<ServiceAccountConfig>::failure(
            makeError(TlErrorCode::MissingCredentials,
                      QStringLiteral("cannot read credentials file '%1'").arg(credentialPath)));
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) {
        return Result<ServiceAccountConfig>::failure(
            makeError(TlErrorCode::InvalidResponse,
                      QStringLiteral("credentials file is not valid JSON")));
    }

    const QJsonObject object = document.object();
    ServiceAccountConfig config;
    config.clientEmail = object.value(QStringLiteral("client_email")).toString();
    config.privateKey = object.value(QStringLiteral("private_key")).toString();
    config.projectId = object.value(QStringLiteral("project_id")).toString();
    config.tokenUri = object.value(QStringLiteral("token_uri")).toString();
    if (config.tokenUri.isEmpty()) {
        config.tokenUri = QStringLiteral("https://oauth2.googleapis.com/token");
    }

    if (config.clientEmail.isEmpty() || config.privateKey.isEmpty() || config.projectId.isEmpty()) {
        return Result<ServiceAccountConfig>::failure(
            makeError(TlErrorCode::MissingCredentials,
                      QStringLiteral("credentials file is missing required fields")));
    }

    return Result<ServiceAccountConfig>::success(std::move(config));
}

NetworkResult performRequest(QNetworkAccessManager &manager,
                             QNetworkRequest request,
                             const QByteArray &payload,
                             const QByteArray &contentType)
{
    request.setHeader(QNetworkRequest::ContentTypeHeader, QString::fromUtf8(contentType));

    QNetworkReply *reply = manager.post(request, payload);
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    const auto cleanup = qScopeGuard([reply]() { reply->deleteLater(); });

    NetworkResult result;
    result.httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    result.body = reply->readAll();
    if (reply->error() != QNetworkReply::NoError) {
        result.errorString = reply->errorString();
    }

    return result;
}

Result<QByteArray> signJwt(const QByteArray &payload, const QString &privateKey)
{
    const QString openssl = QStringLiteral("/opt/local/bin/openssl");
    QFileInfo opensslInfo(openssl);
    if (!opensslInfo.exists()) {
        return Result<QByteArray>::failure(
            makeError(TlErrorCode::AuthFailed,
                      QStringLiteral("openssl executable not found at %1").arg(openssl)));
    }

    QTemporaryFile keyFile;
    keyFile.setAutoRemove(true);
    if (!keyFile.open()) {
        return Result<QByteArray>::failure(
            makeError(TlErrorCode::AuthFailed,
                      QStringLiteral("cannot create temporary key file")));
    }

    if (keyFile.write(privateKey.toUtf8()) < 0 || !keyFile.flush()) {
        return Result<QByteArray>::failure(
            makeError(TlErrorCode::AuthFailed,
                      QStringLiteral("cannot write temporary key file")));
    }

    QProcess process;
    process.start(openssl, {QStringLiteral("dgst"),
                            QStringLiteral("-sha256"),
                            QStringLiteral("-binary"),
                            QStringLiteral("-sign"),
                            keyFile.fileName()});
    if (!process.waitForStarted()) {
        return Result<QByteArray>::failure(
            makeError(TlErrorCode::AuthFailed,
                      QStringLiteral("cannot start openssl")));
    }

    process.write(payload);
    process.closeWriteChannel();

    if (!process.waitForFinished()) {
        return Result<QByteArray>::failure(
            makeError(TlErrorCode::AuthFailed,
                      QStringLiteral("openssl did not finish")));
    }

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        return Result<QByteArray>::failure(
            makeError(TlErrorCode::AuthFailed,
                      QStringLiteral("openssl failed: %1")
                          .arg(QString::fromUtf8(process.readAllStandardError()).trimmed())));
    }

    const QByteArray signature = process.readAllStandardOutput();
    if (signature.isEmpty()) {
        return Result<QByteArray>::failure(
            makeError(TlErrorCode::AuthFailed,
                      QStringLiteral("openssl returned an empty signature")));
    }

    return Result<QByteArray>::success(signature);
}

Result<QString> createJwtAssertion(const ServiceAccountConfig &config)
{
    const QDateTime now = QDateTime::currentDateTimeUtc();
    const qint64 issuedAt = now.toSecsSinceEpoch();
    const qint64 expiresAt = issuedAt + 3600;

    const QJsonObject header = {
        {QStringLiteral("alg"), QStringLiteral("RS256")},
        {QStringLiteral("typ"), QStringLiteral("JWT")}
    };
    const QJsonObject payload = {
        {QStringLiteral("iss"), config.clientEmail},
        {QStringLiteral("scope"), QString::fromLatin1(kTranslationScope)},
        {QStringLiteral("aud"), config.tokenUri},
        {QStringLiteral("iat"), issuedAt},
        {QStringLiteral("exp"), expiresAt}
    };

    const QByteArray encodedHeader = base64Url(QJsonDocument(header).toJson(QJsonDocument::Compact));
    const QByteArray encodedPayload = base64Url(QJsonDocument(payload).toJson(QJsonDocument::Compact));
    const QByteArray signingInput = encodedHeader + '.' + encodedPayload;

    auto signatureResult = signJwt(signingInput, config.privateKey);
    if (!signatureResult.ok()) {
        return Result<QString>::failure(signatureResult.error());
    }

    const QString assertion = QString::fromUtf8(
        signingInput + '.' + base64Url(signatureResult.value()));
    return Result<QString>::success(assertion);
}

Result<QJsonObject> callJsonApi(QNetworkAccessManager &manager,
                                const QUrl &url,
                                const QString &accessToken,
                                const QJsonObject &payload)
{
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", "Bearer " + accessToken.toUtf8());

    const QByteArray payloadBytes = QJsonDocument(payload).toJson(QJsonDocument::Compact);
    const NetworkResult result = performRequest(manager, request, payloadBytes, QByteArrayLiteral("application/json"));

    if (!result.errorString.isEmpty() || result.httpStatus < 200 || result.httpStatus >= 300) {
        return Result<QJsonObject>::failure(
            makeError(TlErrorCode::NetworkFailed,
                      QStringLiteral("API request failed")));
    }

    const QJsonDocument document = QJsonDocument::fromJson(result.body);
    if (!document.isObject()) {
        return Result<QJsonObject>::failure(
            makeError(TlErrorCode::InvalidResponse,
                      QStringLiteral("API response is not valid JSON")));
    }

    return Result<QJsonObject>::success(document.object());
}

} // namespace

GoogleTranslationProvider::GoogleTranslationProvider() = default;

Result<QString> GoogleTranslationProvider::accessToken()
{
    if (!m_accessToken.isEmpty()) {
        return Result<QString>::success(m_accessToken);
    }

    auto configResult = loadServiceAccountConfig();
    if (!configResult.ok()) {
        return Result<QString>::failure(configResult.error());
    }
    const ServiceAccountConfig &config = configResult.value();

    auto assertionResult = createJwtAssertion(config);
    if (!assertionResult.ok()) {
        return Result<QString>::failure(assertionResult.error());
    }

    QUrlQuery form;
    form.addQueryItem(QStringLiteral("grant_type"),
                      QStringLiteral("urn:ietf:params:oauth:grant-type:jwt-bearer"));
    form.addQueryItem(QStringLiteral("assertion"), assertionResult.value());

    const QNetworkRequest request(QUrl(config.tokenUri));
    const NetworkResult result = performRequest(m_networkAccessManager,
                                                request,
                                                form.toString(QUrl::FullyEncoded).toUtf8(),
                                                QByteArrayLiteral("application/x-www-form-urlencoded"));

    if (!result.errorString.isEmpty() || result.httpStatus < 200 || result.httpStatus >= 300) {
        return Result<QString>::failure(
            makeError(TlErrorCode::AuthFailed,
                      QStringLiteral("token request failed")));
    }

    const QJsonDocument document = QJsonDocument::fromJson(result.body);
    if (!document.isObject()) {
        return Result<QString>::failure(
            makeError(TlErrorCode::InvalidResponse,
                      QStringLiteral("token response is not valid JSON")));
    }

    m_accessToken = document.object().value(QStringLiteral("access_token")).toString();
    if (m_accessToken.isEmpty()) {
        return Result<QString>::failure(
            makeError(TlErrorCode::InvalidResponse,
                      QStringLiteral("token response does not contain access_token")));
    }

    return Result<QString>::success(m_accessToken);
}

Result<QString> GoogleTranslationProvider::detectLanguage(const QString &text)
{
    auto configResult = loadServiceAccountConfig();
    if (!configResult.ok()) {
        return Result<QString>::failure(configResult.error());
    }

    auto tokenResult = accessToken();
    if (!tokenResult.ok()) {
        return Result<QString>::failure(tokenResult.error());
    }

    const ServiceAccountConfig &config = configResult.value();
    const QUrl url(QStringLiteral("https://translation.googleapis.com/v3/projects/%1/locations/global:detectLanguage")
                   .arg(config.projectId));
    const QJsonObject payload = {
        {QStringLiteral("content"), text},
        {QStringLiteral("mimeType"), QStringLiteral("text/plain")}
    };

    auto responseResult = callJsonApi(m_networkAccessManager, url, tokenResult.value(), payload);
    if (!responseResult.ok()) {
        return Result<QString>::failure(responseResult.error());
    }

    const QJsonArray languages = responseResult.value().value(QStringLiteral("languages")).toArray();
    if (languages.isEmpty()) {
        return Result<QString>::failure(
            makeError(TlErrorCode::InvalidResponse,
                      QStringLiteral("language detection returned no candidates")));
    }

    const QString detected = languages.first().toObject().value(QStringLiteral("languageCode")).toString();
    if (detected.isEmpty()) {
        return Result<QString>::failure(
            makeError(TlErrorCode::InvalidResponse,
                      QStringLiteral("language detection did not return a language code")));
    }

    return Result<QString>::success(detected);
}

Result<QString> GoogleTranslationProvider::translate(const QString &text,
                                                     const QString &sourceLanguage,
                                                     const QString &targetLanguage)
{
    auto configResult = loadServiceAccountConfig();
    if (!configResult.ok()) {
        return Result<QString>::failure(configResult.error());
    }

    auto tokenResult = accessToken();
    if (!tokenResult.ok()) {
        return Result<QString>::failure(tokenResult.error());
    }

    const ServiceAccountConfig &config = configResult.value();
    const QUrl url(QStringLiteral("https://translation.googleapis.com/v3/projects/%1/locations/global:translateText")
                   .arg(config.projectId));
    const QJsonObject payload = {
        {QStringLiteral("contents"), QJsonArray{text}},
        {QStringLiteral("mimeType"), QStringLiteral("text/plain")},
        {QStringLiteral("sourceLanguageCode"), sourceLanguage},
        {QStringLiteral("targetLanguageCode"), targetLanguage}
    };

    auto responseResult = callJsonApi(m_networkAccessManager, url, tokenResult.value(), payload);
    if (!responseResult.ok()) {
        return Result<QString>::failure(responseResult.error());
    }

    const QJsonArray translations = responseResult.value().value(QStringLiteral("translations")).toArray();
    if (translations.isEmpty()) {
        return Result<QString>::failure(
            makeError(TlErrorCode::InvalidResponse,
                      QStringLiteral("translation response contains no translations")));
    }

    const QString translatedText =
        translations.first().toObject().value(QStringLiteral("translatedText")).toString();
    if (translatedText.isEmpty()) {
        return Result<QString>::failure(
            makeError(TlErrorCode::InvalidResponse,
                      QStringLiteral("translation response contains an empty translation")));
    }

    return Result<QString>::success(translatedText);
}
