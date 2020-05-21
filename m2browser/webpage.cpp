#include "browserwindow.h"
#include "tabwidget.h"
#include "ui_certificateerrordialog.h"
#include "ui_passworddialog.h"
#include "webpage.h"
#include "webview.h"

#include <QAuthenticator>
#include <QMessageBox>
#include <QStyle>
#include <QTimer>
#include <QWebEngineCertificateError>

//コンストラクタ
WebPage::WebPage(QWebEngineProfile *profile, QObject *parent) :
    QWebEnginePage(profile, parent)
{

    //Basic認証要求時の紐付け
    connect(this, &QWebEnginePage::authenticationRequired, this, &WebPage::handleAuthenticationRequired);

    //拡張的機能要求時の紐付け
    connect(this, &QWebEnginePage::featurePermissionRequested, this, &WebPage::handleFeaturePermissionRequested);

    //Proxyサーバ認証要求時の紐付け
    connect(this, &QWebEnginePage::proxyAuthenticationRequired, this, &WebPage::handleProxyAuthenticationRequired);

    //ProtocolHandler登録要求時の紐付け
    connect(this, &QWebEnginePage::registerProtocolHandlerRequested, this, &WebPage::handleRegisterProtocolHandlerRequested);

    //SSLクライアント証明書要求時の紐付け
#if !defined (QT_NO_SSL) || QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    connect(this, &QWebEnginePage::selectClientCertificate, this, &WebPage::handleSelectClientCertificate);
#endif

}


//認証エラーダイアログを表示し認証成功失敗のフラグを返す
bool WebPage::certificateError(const QWebEngineCertificateError &error)
{

    //このWebページが所属するウィンドウのポインタを取得
    QWidget *mainWindow = view()->window();

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QWebEngineCertificateError deferredError = error;
    //認証待ち時間を作る処理
    deferredError.defer();

    //非同期に認証判定する。std:moveは右辺値キャスト（コピーでなくムーブ可能にするため）
    QTimer::singleShot(0, mainWindow, [mainWindow, error = std::move(deferredError)]() mutable {

        //mutable:コピーキャプチャされた引数からconst修飾子を削除
        //認証待ち状態が長すぎてロードが停止された場合
        if(!error.deferred()) {
            QMessageBox::critical(mainWindow, tr("Certificate Error"), error.errorDescription());

        //入力が適切だった場合
        } else {

#else
    if(error.isOverridable()) {
#endif

            //認証ダイアログのセットアアップ
            QDialog dialog(mainWindow);
            dialog.setModal(true);
            dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);

            Ui::CertificateErrorDialog certificateDialog;
            certificateDialog.setupUi(&dialog);
            certificateDialog.m_iconLabel->setText(QString());
            QIcon icon(mainWindow->style()->standardIcon(QStyle::SP_MessageBoxWarning, 0, mainWindow));
            certificateDialog.m_iconLabel->setPixmap(icon.pixmap(32,32));
            certificateDialog.m_errorLabel->setText(error.errorDescription());
            dialog.setWindowTitle(tr("Certificate Error"));

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            if(dialog.exec() == QDialog::Accepted)
                //証明書エラーを無視して続行
                error.ignoreCertificateError();
            else
                //証明書エラーを拒否して続行
                error.rejectCertificate();
        }
    });
    return  true;

#else
        return dialog.exec() == QDialog::Accepted;
    }
    QMessageBox::critical(mainWindow, tr("Certificate Error"), error.errorDescription());
    return false;

#endif

}


//Basic認証要求ダイアログ表示のためのスロット
void WebPage::handleAuthenticationRequired(const QUrl &requestUrl, QAuthenticator *auth)
{

    QWidget *mainWindow = view()->window();
    QDialog dialog(mainWindow);
    dialog.setModal(true);
    dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);

    Ui::PasswordDialog passwordDialog;
    passwordDialog.setupUi(&dialog);

    passwordDialog.m_iconLabel->setText(QString());
    QIcon icon(mainWindow->style()->standardIcon(QStyle::SP_MessageBoxQuestion, 0, mainWindow));
    passwordDialog.m_iconLabel->setPixmap(icon.pixmap(32, 32));

    QString introMessage(tr("Enter username and password for \"%1\" at %2")
                         .arg(auth->realm()).arg(requestUrl.toString().toHtmlEscaped()));
    passwordDialog.m_infoLabel->setText(introMessage);
    passwordDialog.m_infoLabel->setWordWrap(true);

    if(dialog.exec() == QDialog::Accepted) {
        auth->setUser(passwordDialog.m_userNameLineEdit->text());
        auth->setPassword(passwordDialog.m_passwordLineEdit->text());
    } else {
        *auth = QAuthenticator();
    }

}


//拡張的機能要求時の確認ダイアログ表示のためのスロットと補助関数
//補助関数
inline QString questionForFeature(QWebEnginePage::Feature feature)
{
    switch (feature) {
    case QWebEnginePage::Geolocation:
        return WebPage::tr("Allow %1 to access your location information?");
    case QWebEnginePage::MediaAudioCapture:
        return WebPage::tr("Allow %1 to access your microphone?");
    case QWebEnginePage::MediaVideoCapture:
        return WebPage::tr("Allow %1 to access your webcam?");
    case QWebEnginePage::MediaAudioVideoCapture:
        return WebPage::tr("Allow %1 to access your microphone and webcam?");
    case QWebEnginePage::MouseLock:
        return WebPage::tr("Allow %1 to lock your mouse cursor?");
    case QWebEnginePage::DesktopVideoCapture:
        return WebPage::tr("Allow %1 to capture video of your desktop?");
    case QWebEnginePage::DesktopAudioVideoCapture:
        return WebPage::tr("Allow %1 to capture audio and video of your desktop?");
    case QWebEnginePage::Notifications:
        return WebPage::tr("Allow %1 to show notification on your desktop?");
    }
    return QString();
}

void WebPage::handleFeaturePermissionRequested(const QUrl &securityOrigin, Feature feature)
{

    QString title = tr("Permission Request");
    QString question = questionForFeature(feature).arg(securityOrigin.host());
    if(!question.isEmpty() && QMessageBox::question(view()->window(), title, question) == QMessageBox::Yes)
        setFeaturePermission(securityOrigin, feature, PermissionGrantedByUser);
    else
        setFeaturePermission(securityOrigin, feature, PermissionDeniedByUser);

}


//Proxyサーバ認証要求ダイアログ表示のためのスロット
void WebPage::handleProxyAuthenticationRequired(const QUrl &, QAuthenticator *auth, const QString &proxyHost)
{

    QWidget *mainWindow = view()->window();
    QDialog dialog(mainWindow);
    dialog.setModal(true);
    dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);

    Ui::PasswordDialog passwordDialog;
    passwordDialog.setupUi(&dialog);

    passwordDialog.m_iconLabel->setText(QString());
    QIcon icon(mainWindow->style()->standardIcon(QStyle::SP_MessageBoxQuestion, 0, mainWindow));
    passwordDialog.m_iconLabel->setPixmap(icon.pixmap(32, 32));

    QString introMessage = tr("Connect to proxy \"%1\" using:");
    introMessage = introMessage.arg(proxyHost.toHtmlEscaped());
    passwordDialog.m_infoLabel->setText(introMessage);
    passwordDialog.m_infoLabel->setWordWrap(true);

    if(dialog.exec() == QDialog::Accepted) {
        auth->setUser(passwordDialog.m_userNameLineEdit->text());
        auth->setPassword(passwordDialog.m_passwordLineEdit->text());
    } else {
        *auth = QAuthenticator();
    }

}


//ProtocolHandler登録要求ダイアログ表示のためのスロット
void WebPage::handleRegisterProtocolHandlerRequested(QWebEngineRegisterProtocolHandlerRequest request)
{

    auto answer = QMessageBox::question(
                view()->window(),
                tr("Permission Request"),
                tr("Allow %1 to open all %2 links?")
                .arg(request.origin().host())
                .arg(request.scheme())
    );

    if(answer == QMessageBox::Yes)
        request.accept();
    else
        request.reject();

}


//SSLクライアント証明書要求の処理のためのスロット
#if !defined (QT_NO_SSL) || QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
void WebPage::handleSelectClientCertificate(QWebEngineClientCertificateSelection selection)
{

    selection.select(selection.certificates().at(0));

}
#endif
