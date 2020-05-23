#ifndef BROWSERWINDOW_H
#define BROWSERWINDOW_H

#include <QMainWindow>
#include <QTime>
#include <QWebEnginePage>
#include <QComboBox>

QT_BEGIN_NAMESPACE
class QLineEdit;        //QLineEditクラスを暗黙参照
class QProgressBar;     //QProgressBarクラスを暗黙参照
class QComboBox;
QT_END_NAMESPACE

class Browser;          //Browserクラスを参照
class TabWidget;        //TabWidgetクラスを参照
class WebView;          //WebViewクラスを参照
class JavascriptUtil;

class BrowserWindow : public QMainWindow
{
    Q_OBJECT

public:

    //コンストラクタ
    BrowserWindow(Browser *browser, QWebEngineProfile *profile, bool forDevTools = false, QUrl homeUrl = QUrl());

    //Widgetの推奨サイズを取得
    QSize sizeHint() const override;

    //TabWidgetインスタンスポインタを返す
    TabWidget *tabWidget() const;

    //TabWidgetインスタンス経由でカレントタブポインタを返す
    WebView *currentTab() const;

    //Browserインスタンスポインタを返す
    Browser *browser() { return m_browser; }

protected:

    //アプリウィンドウを閉じるシグナルを関知して処理するためのスロット
    void closeEvent(QCloseEvent *event) override;

private slots:

    //新しいアプリウィンドウを開く（追加）ためのスロット
    void handleNewWindowTriggered();

    //新しいプライベートアプリウィンドウを開く（追加）ためのスロット
    void handleNewIncognitoWindowTriggered();

    //ファイルを開く要求に応じてファイルを開くためのスロット
    void handleFileOpenTriggered();

    //ページ内検索の要求に応じて検索（ダイアログ表示）するためのスロット
    void handleFindActionTriggered();

    //アプリウィンドウ表示をきりかえるためのスロット
    void handleShowWindowTriggered();

    //ページロード進捗に応じた更新/更新停止ボタンに切り換えるためのスロット
    void handleWebViewLoadProgress(int);

    //アプリウィンドウのタイトル変更のためのスロット
    void handleWebViewTitleChanged(const QString &title);

    //基本的なウェブアクションの有効無効化のためのスロット
    void handleWebActionEnabledChanged(QWebEnginePage::WebAction action, bool enabled);

    //DevToolsの表示要求に応じてDevToolsウィンドウを表示するためのスロット
    void handleDevToolsRequested(QWebEnginePage *source);

    //ページ内検索結果が最後に到達した際の処理をするためのスロット
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    void handleFindTextFinished(const QWebEngineFindTextResult &result);
#endif

    //TSVファイル読み込み要求の処理のためのスロット
    void handleTsvFileOpenRequested();

    //前のコンボに進む要求時の処理のためのスロット
    void handleComboForwardRequested();

    //次のコンボに進む要求時の処理のためのスロット
    void handleComboBackRequested();

    //コンボ再読込要求時の処理のためのスロット
    void handleComboReloadRequested();

private:

    //ファイルメニューを生成
    QMenu *createFileMenu(TabWidget *tabWidget);
    //編集メニューを生成
    QMenu *createEditMenu();
    //ビューメニューを生成
    QMenu *createViewMenu(QToolBar *toolBar);
    //ウィンドウメニューを生成
    QMenu *createWindowMenu(TabWidget *tabWidget);
    //ツールバーを生成
    QToolBar *createToolBar();
    //セカンダリツールバーを生成
    QToolBar *createSecondToolBar();

    //コンボで選択したページに切替
    void handleComboCurrentChanged(int index);

private:
    Browser *m_browser;                     //Browserクラスのインスタンスポインタ
    QWebEngineProfile *m_profile;           //QWebEngineProfile（通常/プライベート切替用)のポインタ
    TabWidget *m_tabWidget;                 //TabWidgetクラスのインスタンスポインタ
    QProgressBar *m_progressBar;            //ProgressBarのポインタ
    QAction *m_historyBackAction;           //戻るActionのポインタ
    QAction *m_historyForwardAction;        //進むActionのポインタ
    QAction *m_stopAction;                  //停止Actionのポインタ
    QAction *m_reloadAction;                //更新Actionのポインタ
    QAction *m_stopReloadAction;            //更新停止Actionのポインタ
    QLineEdit *m_urlLineEdit;               //URL欄のポインタ
    QAction *m_favAction;                   //ファビコン表示Actionのポインタ
    QString m_lastSearch;                   //ページ内検索の末尾検索結果文字列

    QComboBox *m_urlComboBox;               //URL選択コンボのポインタ
    QVector<QVector<QString>> m_urlArr;     //URL配列
    int m_urlArrIndex;                       //配列カウンタ
    QAction *m_openTsvAction;               //TSVファイルを開くAction
    QAction *m_comboForwardAction;          //URL選択コンボ進むActionのポインタ
    QAction *m_comboBackAction;             //URL選択コンボ戻るActionのポインタ
    QAction *m_homeUrlAction;               //HomeボタンAction
    QAction *m_comboReloadAction;           //URL選択コンボ再読込Actionのポインタ

    QUrl m_homeUrl;                         //規定のURL

    JavascriptUtil *m_jsUtil;               //JavascriptUtilのインスタンスポインタ

};

#endif // BROWSERWINDOW_H
