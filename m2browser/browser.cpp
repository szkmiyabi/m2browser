#include "browser.h"
#include "browserwindow.h"


//コンストラクタ
Browser::Browser(QUrl homeUrl) :
    m_homeUrl(homeUrl)
{

    //DownloadManagerWidgetだけが残った場合アプリケーションを終了
    m_downloadManagerWidget.setAttribute(Qt::WA_QuitOnClose, false);

    //QWebEngineProfile
    //A web engine profile contains settings, scripts, persistent cookie policy,
    //and the list of visited links shared by all web engine pages that belong to the profile

    //QWebEngineProfileからdownloadRequestedシグナル送信
    //DownloadManagerWidgetのスロットが受信
    QObject::connect(
                QWebEngineProfile::defaultProfile(), &QWebEngineProfile::downloadRequested,
                &m_downloadManagerWidget, &DownloadManagerWidget::downloadRequested
    );

}


//アプリケーションウィンドウを生成しポインタを返す
BrowserWindow *Browser::createWindow(bool offTheRecord)
{
    //プライベートウィンドウの場合のSignal/Slot処理の付け直し
    if(offTheRecord && !m_otrProfile) {
        m_otrProfile.reset(new QWebEngineProfile);
        QObject::connect(
                    m_otrProfile.get(), &QWebEngineProfile::downloadRequested,
                    &m_downloadManagerWidget, &DownloadManagerWidget::downloadRequested
        );
    }

    //BrowserWindowクラスのインスタンス（ウィンドウの生成）
    auto profile = offTheRecord ? m_otrProfile.get() : QWebEngineProfile::defaultProfile();
    auto mainWindow = new BrowserWindow(this, profile, false, m_homeUrl);
    m_windows.append(mainWindow);
    QObject::connect(mainWindow, &QObject::destroyed, [this, mainWindow]() {
        m_windows.removeOne(mainWindow);
    });
    mainWindow->show();
    return mainWindow;
}


//DevToolsウィンドウを生成しポインタを返す
BrowserWindow *Browser::createDevToolsWindow()
{
    auto profile = QWebEngineProfile::defaultProfile();
    auto mainWindow = new BrowserWindow(this, profile, true, m_homeUrl);
    m_windows.append(mainWindow);
    QObject::connect(mainWindow, &QObject::destroyed, [this, mainWindow]() {
        m_windows.removeOne(mainWindow);
    });
    mainWindow->show();
    return mainWindow;
}
