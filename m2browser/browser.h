#ifndef BROWSER_H
#define BROWSER_H

#include "downloadmanagerwidget.h"

#include <QVector>
#include <QWebEngineProfile>
#include <QUrl>

class BrowserWindow;        //BrowserWindowクラスを参照

class Browser
{
public:

    //コンストラクタ
    Browser(QUrl homeUrl = QUrl());

    //windowオブジェクトのベクター
    QVector<BrowserWindow*> windows() { return m_windows; }

    //アプリケーションウィンドウを生成しポインタを返す
    BrowserWindow *createWindow(bool offTheRecord = false);

    //DevToolsウィンドウを生成しポインタを返す
    BrowserWindow *createDevToolsWindow();

    //DownloadManagerWidgetの参照を返す
    DownloadManagerWidget &downloadManagerWidget() { return m_downloadManagerWidget; }


private:

    //アプリウィンドウのポインタのベクター
    QVector<BrowserWindow*> m_windows;

    //DownloadManagerWidgetクラスのインスタンス
    DownloadManagerWidget m_downloadManagerWidget;

    //ポインタ登録されたプライベートウィンドウ用QWebEngineProfileクラス（通常/プライベートの切替用）
    QScopedPointer<QWebEngineProfile> m_otrProfile;

    QUrl m_homeUrl;         //規定のURL
};
#endif // BROWSER_H
