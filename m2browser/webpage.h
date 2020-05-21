#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <QWebEnginePage>
#include <QWebEngineRegisterProtocolHandlerRequest>


class WebPage : public QWebEnginePage
{
    Q_OBJECT

public:

    //コンストラクタ
    WebPage(QWebEngineProfile *profile, QObject *parent = nullptr);

protected:

    //認証エラーダイアログを表示し認証成功失敗のフラグを返す
    bool certificateError(const QWebEngineCertificateError &error) override;

private slots:

    //Basic認証要求ダイアログ表示のためのスロット
    void handleAuthenticationRequired(const QUrl &requestUrl, QAuthenticator *auth);

    //拡張的機能要求時の確認ダイアログ表示のためのスロットと補助関数
    void handleFeaturePermissionRequested(const QUrl &securityOrigin, Feature feature);

    //Proxyサーバ認証要求ダイアログ表示のためのスロット
    void handleProxyAuthenticationRequired(const QUrl &requestUrl, QAuthenticator *auth, const QString &proxyHost);

    //ProtocolHandler登録要求ダイアログ表示のためのスロット
    void handleRegisterProtocolHandlerRequested(QWebEngineRegisterProtocolHandlerRequest request);

    //SSLクライアント証明書要求の処理のためのスロット
#if !defined(QT_NO_SSL) || QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    void handleSelectClientCertificate(QWebEngineClientCertificateSelection clientCertSelection);
#endif

};

#endif // WEBPAGE_H
