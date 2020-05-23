#include "browser.h"
#include "browserwindow.h"
#include "tabwidget.h"
#include "webpage.h"
#include "webpopupwindow.h"
#include "webview.h"
#include <QContextMenuEvent>
#include <QDebug>
#include <QMenu>
#include <QMessageBox>
#include <QTimer>

//コンストラクタ
WebView::WebView(QWidget *parent) :
    QWebEngineView(parent),
    m_loadProgress(100)
{

    //Webページロード開始時のファビコン設定の紐付け
    connect(this, &QWebEngineView::loadStarted, [this]() {
        m_loadProgress = 0;
        emit favIconChanged(favIcon());
    });

    //ロード進捗に応じた紐付け
    connect(this, &QWebEngineView::loadProgress, [this](int progress) {
        m_loadProgress = progress;
    });

    //Webページロード完了時のファビコン設定の紐付け
    connect(this, &QWebEngineView::loadFinished, [this](bool success) {
        m_loadProgress = success ? 100 : -1;
        emit favIconChanged(favIcon());
    });

    //Webページのファビコン変更時（URL遷移など）のファビコン設定の紐付け
    connect(this, &QWebEngineView::iconChanged, [this](const QIcon &) {
        emit favIconChanged(favIcon());
    });

    //Webページロードが停止された時の紐付け
    connect(this, &QWebEngineView::renderProcessTerminated,
        [this](QWebEnginePage::RenderProcessTerminationStatus termStatus, int statusCode) {

            QString status;
            switch (termStatus) {
            case QWebEnginePage::NormalTerminationStatus:
                status = tr("Render process normal exit");
                break;
            case QWebEnginePage::AbnormalTerminationStatus:
                status = tr("Render process abnormal exit");
                break;
            case QWebEnginePage::CrashedTerminationStatus:
                status = tr("Render process crashed");
                break;
            case QWebEnginePage::KilledTerminationStatus:
                status = tr("Render process killed");
                break;
            }

            QMessageBox::StandardButton btn = QMessageBox::question(
                window(),
                status,
                tr("Render process exited with code: %1\n"
                   "Do you want to reload the page ?").arg(statusCode)
            );
            if(btn == QMessageBox::Yes)
                //非同期でWebページをリロードする
                QTimer::singleShot(0, [this] { reload(); });
        }
    );

}


//Webページのセットアップ
void WebView::setPage(WebPage *page)
{
    createWebActionTrigger(page, QWebEnginePage::Forward);
    createWebActionTrigger(page, QWebEnginePage::Back);
    createWebActionTrigger(page, QWebEnginePage::Reload);
    createWebActionTrigger(page, QWebEnginePage::Stop);
    QWebEngineView::setPage(page);
}


//Webページロード進捗状況を返す
int WebView::loadProgress() const
{
    return m_loadProgress;
}


//Webページの基本動作の有効無効化のシグナルを生成する
void WebView::createWebActionTrigger(QWebEnginePage *page, QWebEnginePage::WebAction webAction)
{
    QAction *action = page->action(webAction);
    connect(action, &QAction::changed, [this, action, webAction] {
        emit webActionEnabledChanged(webAction, action->isEnabled());
    });
}


//Webページの基本動作の有効無効の判定
bool WebView::isWebActionEnabled(QWebEnginePage::WebAction webAction) const
{
    return page()->action(webAction)->isEnabled();
}


//デフォルトのファビコンを返す
QIcon WebView::favIcon() const
{
    QIcon favIcon = icon();
    if(!favIcon.isNull())
        return favIcon;

    if(m_loadProgress < 0) {
        static QIcon errorIcon(QStringLiteral(":dialog-error.svg"));
        return errorIcon;
    } else if(m_loadProgress < 100) {
        static QIcon loadingIcon(QStringLiteral(":view-refresh.svg"));
        return loadingIcon;
    } else {
        static QIcon defaultIcon(QStringLiteral(":text-html.svg"));
        return defaultIcon;
    }
}


//状況に応じた新しいウィンドウを生成しそのポインタを返す
QWebEngineView *WebView::createWindow(QWebEnginePage::WebWindowType type)
{

    //このWebViewクラスが所属するウィンドウを取得
    BrowserWindow *mainWindow = qobject_cast<BrowserWindow*>(window());
    if(!mainWindow)
        return nullptr;

    switch (type) {

    //新しいタブで開く
    case QWebEnginePage::WebBrowserTab: {
        return mainWindow->tabWidget()->createTab();
    }
    //新しいバックグラウンドタブで開く
    case QWebEnginePage::WebBrowserBackgroundTab: {
        return mainWindow->tabWidget()->createBackgroundTab();
    }
    //カレントタブで開く
    case QWebEnginePage::WebBrowserWindow: {
        return mainWindow->browser()->createWindow()->currentTab();
    }
    //新しいポップアップウィンドウで開く
    case QWebEnginePage::WebDialog: {
        WebPopupWindow *popup = new WebPopupWindow(page()->profile());

        //DevTools要求時の紐付け
        connect(popup->view(), &WebView::devToolsRequested, this, &WebView::devToolsRequested);
        return popup->view();
    }

    }
    return nullptr;

}


//Webページのコンテキストメニューのセットアップ
void WebView::contextMenuEvent(QContextMenuEvent *event)
{

    QMenu *menu = page()->createStandardContextMenu();
    const QList<QAction *> actions = menu->actions();
    auto inspectElement = std::find(actions.cbegin(), actions.cend(), page()->action(QWebEnginePage::InspectElement));

    if(inspectElement == actions.cend()) {
        auto viewSource = std::find(actions.cbegin(), actions.cend(), page()->action(QWebEnginePage::ViewSource));
        if(viewSource == actions.cend())
            menu->addSeparator();

        QAction *action = new QAction(menu);
        action->setText("Open inspector in new window");
        connect(action, &QAction::triggered, [this]() {
            emit devToolsRequested(page());
        });

        QAction *before(inspectElement == actions.cend() ? nullptr : *inspectElement);
        menu->insertAction(before, action);
    } else {
        (*inspectElement)->setText(tr("Inspect element"));
    }

    menu->popup(event->globalPos());

}










