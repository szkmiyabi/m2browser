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

    connect(this, &QWebEnginePage::authenticationRequired, this, &WebPage::handleAuthenticationRequired);
    connect(this, &QWebEnginePage::featurePermissionRequested, this, &WebPage::handleFeaturePermissionRequested);
    connect(this, &QWebEnginePage::proxyAuthenticationRequired, this, &WebPage::handleProxyAuthenticationRequired);
    connect(this, &QWebEnginePage::registerProtocolHandlerRequested, this, &WebPage::handleRegisterProtocolHandlerRequested);
#if !defined (QT_NO_SSL) || QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    connect(this, &QWebEnginePage::selectClientCertificate, this, &WebPage::handleSelectClientCertificate);
#endif

}


bool WebPage::certificateError(const QWebEngineCertificateError &error)
{

    QWidget *mainWindow = view()->window();

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QWebEngineCertificateError deferredError = error;
    deferredError.defer();

    QTimer::singleShot(0, mainWindow, [mainWindow, error = std::move(deferredError)]() mutable {
        if(!error.deferred()) {
            QMessageBox::critical(mainWindow, tr("Certificate Error"), error.errorDescription());
        } else {

#else
    if(error.isOverridable()) {
#endif

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
                error.ignoreCertificateError();
            else
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
