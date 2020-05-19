#ifndef BROWSER_H
#define BROWSER_H

#include "downloadmanagerwidget.h"

#include <QVector>
#include <QWebEngineProfile>

class BrowserWindow;

class Browser
{
public:

    //コンストラクタ
    Browser();

    //windowオブジェクトのベクター
    QVector<BrowserWindow*> windows() { return m_windows; }

    //アプリケーションウィンドウを生成しポインタ返し
    BrowserWindow *createWindow(bool offTheRecord = false);

    //DevToolsウィンドウを生成しポインタ返し
    BrowserWindow *createDevToolsWindow();

    //DownloadManagerWidgetの参照返し
    DownloadManagerWidget &downloadManagerWidget() { return m_downloadManagerWidget; }

private:

    //windowオブジェクトのベクター
    QVector<BrowserWindow*> m_windows;

    //DownloadManagerWidgetクラスのインスタンス
    DownloadManagerWidget m_downloadManagerWidget;

    //ポインタ登録されたプライベートウィンドウ用QWebEngineProfileクラス
    QScopedPointer<QWebEngineProfile> m_otrProfile;

};
#endif // BROWSER_H
