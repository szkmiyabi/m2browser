#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QTabWidget>
#include <QWebEnginePage>

QT_BEGIN_NAMESPACE
class QUrl;         //QUrlクラスの暗黙参照
QT_END_NAMESPACE

class WebView;      //WebViewクラスの参照

class TabWidget : public QTabWidget
{
    Q_OBJECT

public:

    //コンストラクタ
    TabWidget(QWebEngineProfile *profile, QWidget *parent = nullptr);

    //アクティブなWebViewクラスのインスタンスポインタを返す
    WebView *currentWebView() const;

signals:

    //TabWidget上で発生しうるイベントのシグナル群
    void linkHovered(const QString &link);
    void loadProgress(int progress);
    void titleChanged(const QString &title);
    void urlChanged(const QUrl &url);
    void favIconChanged(const QIcon &icon);
    void webActionEnabledChanged(QWebEnginePage::WebAction action, bool enabled);
    void devToolsRequested(QWebEnginePage *source);
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    void findTextFinished(const QWebEngineFindTextResult &result);
#endif

public slots:

    //指定したタブにURLを設定するためのスロット
    void setUrl(const QUrl &url);

    //Webページの基本動作の無効有効を検知して処理するためのスロット
    void triggerWebPageAction(QWebEnginePage::WebAction action);

    //タブを追加してそのポインタを返す
    WebView *createTab();

    //バックグラウンドでタブを追加しそのポインタを返す
    WebView *createBackgroundTab();

    //指定したタブを閉じるためのスロット
    void closeTab(int index);

    //次のタブを選択するためのスロット
    void nextTab();

    //前のタブを選択するためのスロット
    void previousTab();

private slots:

    //タブを切り替えたときの処理のためのスロット
    void handleCurrentChanged(int index);

    //タブのコンテキストメニューのためのスロット
    void handleContextMenuRequested(const QPoint &pos);

    //指定したタブを複製するためのスロット
    void cloneTab(int index);

    //カレントタブ以外のタブを全て閉じるためのスロット
    void closeOtherTabs(int index);

    //全てのタブをリロードするためのスロット
    void reloadAllTabs();

    //指定したタブをリロードするためのスロット
    void reloadTab(int index);

private:

    //指定したWebViewクラスのインスタンスポインタを返す
    WebView *webView(int index) const;

    //WebViewのセットアアップ
    void setupView(WebView *webView);

    QWebEngineProfile *m_profile;

};

#endif // TABWIDGET_H
