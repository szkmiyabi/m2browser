#ifndef WEBVIEW_H
#define WEBVIEW_H

#include <QIcon>
#include <QWebEngineView>

class WebPage;          //WebPageクラスの参照

class WebView : public QWebEngineView
{
    Q_OBJECT

public:

    //コンストラクタ
    WebView(QWidget *parent = nullptr);

    //Webページのセットアップ
    void setPage(WebPage *page);

    //Webページロード進捗状況を返す
    int loadProgress() const;

    //Webページの基本動作の有効無効の判定
    bool isWebActionEnabled(QWebEnginePage::WebAction webAction) const;

    //デフォルトのファビコンを返す
    QIcon favIcon() const;

protected:

    //Webページのコンテキストメニューのセットアップ
    void contextMenuEvent(QContextMenuEvent *event) override;

    //状況に応じた新しいウィンドウを生成しそのポインタを返す
    QWebEngineView *createWindow(QWebEnginePage::WebWindowType type) override;

signals:

    //WebViewクラス上で発生しうるイベントのシグナル群
    void webActionEnabledChanged(QWebEnginePage::WebAction webAction, bool enabled);
    void favIconChanged(const QIcon &icon);
    void devToolsRequested(QWebEnginePage *source);


private:

    //Webページの基本動作の有効無効化のシグナルを生成する
    void createWebActionTrigger(QWebEnginePage *page, QWebEnginePage::WebAction);

private:
    int m_loadProgress;
};

#endif // WEBVIEW_H
