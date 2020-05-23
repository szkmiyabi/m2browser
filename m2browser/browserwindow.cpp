#include "browser.h"
#include "browserwindow.h"
#include "downloadmanagerwidget.h"
#include "tabwidget.h"
#include "webview.h"
#include "javascriptutil.h"
#include <QApplication>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QEvent>
#include <QFileDialog>
#include <QInputDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QProgressBar>
#include <QScreen>
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
#include <QWebEngineFindTextResult>
#endif
#include <QWebEngineProfile>

//コンストラクタ
BrowserWindow::BrowserWindow(Browser *browser, QWebEngineProfile *profile, bool forDevTools, QUrl homeUrl) :
    m_browser(browser),
    m_profile(profile),
    m_tabWidget(new TabWidget(profile, this)), //TabWidgetクラスのインスタンスへのポインタ
    m_progressBar(nullptr),
    m_historyBackAction(nullptr),
    m_historyForwardAction(nullptr),
    m_stopAction(nullptr),
    m_reloadAction(nullptr),
    m_stopReloadAction(nullptr),
    m_urlLineEdit(nullptr),
    m_favAction(nullptr),
    m_urlArrIndex(0),
    m_openTsvAction(nullptr),
    m_comboForwardAction(nullptr),
    m_comboBackAction(nullptr),
    m_homeUrlAction(nullptr),
    m_comboReloadAction(nullptr),
    m_homeUrl(homeUrl),
    m_jsUtil(new JavascriptUtil())
{

    //closeイベントが届いたら自動でdeleteする設定
    setAttribute(Qt::WA_DeleteOnClose, true);
    //マウスクリックのみでフォーカスを移す設定
    setFocusPolicy(Qt::ClickFocus);

    //通常のウィンドウの場合ツールバーとメニューバー表示
    if(!forDevTools) {
        m_progressBar = new QProgressBar(this);

        QToolBar *toolbar = createToolBar();
        addToolBar(toolbar);

        menuBar()->addMenu(createFileMenu(m_tabWidget));
        menuBar()->addMenu(createEditMenu());
        menuBar()->addMenu(createViewMenu(toolbar));
        menuBar()->addMenu(createWindowMenu(m_tabWidget));
    }

    //レイアウトコンテナの設定
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout;

    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    //通常のウィンドウの場合progressバーを表示
    if(!forDevTools) {
        addToolBarBreak();

        m_progressBar->setMaximumHeight(1);
        m_progressBar->setTextVisible(false);
        m_progressBar->setStyleSheet(QStringLiteral(
            "QProgressBar {border: 0px} QProgressBar::chunk {background-color: #00C000}"
        ));

        layout->addWidget(m_progressBar);
    }

    //CentralWidget（ウィンドウいっぱいの幅高さにする）の設定
    layout->addWidget(m_tabWidget);
    centralWidget->setLayout(layout);

    setCentralWidget(centralWidget);

    //URL参照やページ遷移に応じたアプリウィンドウタイトル変更の紐付け
    connect(m_tabWidget, &TabWidget::titleChanged, this, &BrowserWindow::handleWebViewTitleChanged);

    if(!forDevTools) {

        //リンクポイント時、ロード状況、戻る・進む・更新の無効有効判断、URL変更時
        //ファビコン変更時、DevTools要求時、URL欄Enter時、ページ内検索要求時の紐付け
        connect(m_tabWidget, &TabWidget::linkHovered, [this](const QString& url) {
            statusBar()->showMessage(url);
        });
        connect(m_tabWidget, &TabWidget::loadProgress, this, &BrowserWindow::handleWebViewLoadProgress);
        connect(m_tabWidget, &TabWidget::webActionEnabledChanged, this, &BrowserWindow::handleWebActionEnabledChanged);
        connect(m_tabWidget, &TabWidget::urlChanged, [this](const QUrl &url) {
            m_urlLineEdit->setText(url.toDisplayString());
        });
        connect(m_tabWidget, &TabWidget::favIconChanged, m_favAction, &QAction::setIcon);
        connect(m_tabWidget, &TabWidget::devToolsRequested, this, &BrowserWindow::handleDevToolsRequested);
        connect(m_urlLineEdit, &QLineEdit::returnPressed, [this]() {
            m_tabWidget->setUrl(QUrl::fromUserInput(m_urlLineEdit->text()));
        });

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        connect(m_tabWidget, &TabWidget::findTextFinished, this, &BrowserWindow::handleFindTextFinished);
#endif

        //Ctrl+LでURL欄にフォーカス移動処置
        QAction *focusUrlLineEditAction = new QAction(this);
        addAction(focusUrlLineEditAction);
        focusUrlLineEditAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_L));
        connect(focusUrlLineEditAction, &QAction::triggered, this, [this]() {
            m_urlLineEdit->setFocus(Qt::ShortcutFocusReason);
        });
    }

    //引数があればそれを用いてアプリウィンドウタイトルを設定
    handleWebViewTitleChanged(QString());
    //ポインタ経由でタブを生成
    m_tabWidget->createTab();

}


//Widgetの推奨サイズを取得
QSize BrowserWindow::sizeHint() const
{
    QRect desktopRect = QApplication::primaryScreen()->geometry();
    QSize size = desktopRect.size() * qreal(0.9);
    return size;
}


//ファイルメニューを生成
QMenu *BrowserWindow::createFileMenu(TabWidget *tabWidget)
{

    //QMenuインスタンスを親にしてシグナル＋スロットを紐付けたQActionを追加していく
    QMenu *fileMenu = new QMenu(tr("&File"));
    fileMenu->addAction(tr("&New Window"), this, &BrowserWindow::handleNewWindowTriggered, QKeySequence::New);
    fileMenu->addAction(tr("New &Incognito Window"), this, &BrowserWindow::handleNewIncognitoWindowTriggered);

    QAction *newTabAction = new QAction(tr("New &Tab"), this);
    newTabAction->setShortcuts(QKeySequence::AddTab);
    connect(newTabAction, &QAction::triggered, this, [this]() {
        m_tabWidget->createTab();
        m_urlLineEdit->setFocus();
    });
    fileMenu->addAction(newTabAction);

    //ウェブリソースを開くコマンドの定義と登録
    fileMenu->addAction(tr("&Open File..."), this, &BrowserWindow::handleFileOpenTriggered, QKeySequence::Open);



    fileMenu->addSeparator();

    QAction *closeTabAction = new QAction(tr("&Close Tab"), this);
    closeTabAction->setShortcuts(QKeySequence::Close);
    connect(closeTabAction, &QAction::triggered, [tabWidget]() {
        tabWidget->closeTab(tabWidget->currentIndex());
    });
    fileMenu->addAction(closeTabAction);

    QAction *closeAction = new QAction(tr("&Quit"), this);
    closeAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q));
    connect(closeAction, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(closeAction);

    connect(fileMenu, &QMenu::aboutToShow, [this, closeAction]() {
        if(m_browser->windows().count() == 1)
            closeAction->setText(tr("&Quit"));
        else
            closeAction->setText(tr("&Close Window"));
    });

    return fileMenu;

}


//編集メニューを生成
QMenu *BrowserWindow::createEditMenu()
{

    QMenu *editMenu = new QMenu(tr("&Edit"));
    QAction *findAction = editMenu->addAction(tr("&Find"));
    findAction->setShortcuts(QKeySequence::Find);
    connect(findAction, &QAction::triggered, this, &BrowserWindow::handleFindActionTriggered);

    QAction *findNextAction = editMenu->addAction(tr("Find &Next"));
    findNextAction->setShortcut(QKeySequence::FindNext);
    connect(findNextAction, &QAction::triggered, [this]() {
        if(!currentTab() || m_lastSearch.isEmpty())
            return;
        currentTab()->findText(m_lastSearch);
    });

    QAction *findPreviousAction = editMenu->addAction(tr("Find &Previous"));
    findPreviousAction->setShortcut(QKeySequence::FindPrevious);
    connect(findPreviousAction, &QAction::triggered, [this]() {
        if(!currentTab() || m_lastSearch.isEmpty())
            return;
        currentTab()->findText(m_lastSearch, QWebEnginePage::FindBackward);
    });

    return editMenu;

}


//ビューメニューを生成
QMenu *BrowserWindow::createViewMenu(QToolBar *toolbar)
{

    QMenu *viewMenu = new QMenu(tr("&View"));
    m_stopAction = viewMenu->addAction(tr("&Stop"));
    QList<QKeySequence> shortcuts;
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_Period));
    shortcuts.append(Qt::Key_Escape);
    m_stopAction->setShortcuts(shortcuts);
    connect(m_stopAction, &QAction::triggered, [this]() {
        m_tabWidget->triggerWebPageAction(QWebEnginePage::Stop);
    });

    m_reloadAction = viewMenu->addAction(tr("Reload Page"));
    m_reloadAction->setShortcuts(QKeySequence::Refresh);
    connect(m_reloadAction, &QAction::triggered, [this]() {
        m_tabWidget->triggerWebPageAction(QWebEnginePage::Reload);
    });

    QAction *zoomIn = viewMenu->addAction(tr("Zoom &In"));
    zoomIn->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Plus));
    connect(zoomIn, &QAction::triggered, [this]() {
        if(currentTab())
            currentTab()->setZoomFactor(currentTab()->zoomFactor() + 0.1);
    });

    QAction *zoomOut = viewMenu->addAction(tr("Zoom &Out"));
    zoomOut->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus));
    connect(zoomOut, &QAction::triggered, [this]() {
        if(currentTab())
            currentTab()->setZoomFactor(currentTab()->zoomFactor() - 0.1);
    });

    QAction *resetZoom = viewMenu->addAction(tr("Reset &Zoom"));
    resetZoom->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));
    connect(resetZoom, &QAction::triggered, [this]() {
        if(currentTab())
            currentTab()->setZoomFactor(1.0);
    });

    viewMenu->addSeparator();

    QAction *viewToolbarAction = new QAction(tr("Hide Toolbar"), this);
    viewToolbarAction->setShortcut(tr("Ctrl+|"));
    connect(viewToolbarAction, &QAction::triggered, [toolbar, viewToolbarAction]() {
        if(toolbar->isVisible()) {
            viewToolbarAction->setText(tr("Show Toolbar"));
            toolbar->close();
        } else {
            viewToolbarAction->setText(tr("Hide Toolbar"));
            toolbar->show();
        }
    });
    viewMenu->addAction(viewToolbarAction);

    QAction *viewStatusbarAction = new QAction(tr("Hide Status Bar"), this);
    viewStatusbarAction->setShortcut(tr("Ctrl+/"));
    connect(viewStatusbarAction, &QAction::triggered, [this, viewStatusbarAction]() {
        if(statusBar()->isVisible()) {
            viewStatusbarAction->setText(tr("Show Status Bar"));
            statusBar()->close();
        } else {
            viewStatusbarAction->setText(tr("Hide Status Bar"));
            statusBar()->show();
        }
    });
    viewMenu->addAction(viewStatusbarAction);

    return viewMenu;

}


//ウィンドウメニューを生成
QMenu *BrowserWindow::createWindowMenu(TabWidget *tabWidget)
{

    QMenu *menu = new QMenu(tr("&Window"));

    QAction *nextTabAction = new QAction(tr("Show Next Tab"), this);
    QList<QKeySequence> shortcuts;
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_BraceRight));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_PageDown));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_BracketRight));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_Less));
    nextTabAction->setShortcuts(shortcuts);
    connect(nextTabAction, &QAction::triggered, tabWidget, &TabWidget::nextTab);

    QAction *previousTabAction = new QAction(tr("Show Previous Tab"), this);
    shortcuts.clear();
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_BraceLeft));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_PageUp));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_BracketLeft));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_Greater));
    previousTabAction->setShortcuts(shortcuts);
    connect(previousTabAction, &QAction::triggered, tabWidget, &TabWidget::previousTab);

    //ウィンドウメニューが表示される直前に紐付け
    connect(menu, &QMenu::aboutToShow, [this, menu, nextTabAction, previousTabAction]() {
        menu->clear();
        menu->addAction(nextTabAction);
        menu->addAction(previousTabAction);
        menu->addSeparator();

        //単一あるいは複数のウィンドウを操作しカレントウィンドウの場合項目にチェック表示する処理
        QVector<BrowserWindow*> windows = m_browser->windows();
        int index(-1);
        for(auto window : windows) {
            QAction *action = menu->addAction(window->windowTitle(), this, &BrowserWindow::handleShowWindowTriggered);
            action->setData(++index);
            action->setCheckable(true);
            if(window == this)
                action->setChecked(true);
        }
    });

    return menu;

}


//ツールバーを生成
QToolBar *BrowserWindow::createToolBar()
{

    QToolBar *navigationBar = new QToolBar(tr("Navigation"));
    navigationBar->setMovable(false);
    navigationBar->toggleViewAction()->setEnabled(false);

    //TSVファイル開くボタン
    m_openTsvAction = new QAction(this);
    m_openTsvAction->setIconVisibleInMenu(false);
    m_openTsvAction->setIcon(QIcon(QStringLiteral(":open-file.svg")));
    m_openTsvAction->setToolTip(tr("Open tsv file with PID and URL"));
    connect(m_openTsvAction, &QAction::triggered, this, &BrowserWindow::handleTsvFileOpenRequested);
    navigationBar->addAction(m_openTsvAction);

    //combo戻るボタン
    m_comboBackAction = new QAction(this);
    m_comboBackAction->setIconVisibleInMenu(false);
    m_comboBackAction->setIcon(QIcon(QStringLiteral(":cmb-previous.svg")));
    m_comboBackAction->setToolTip(tr("Back combo pid"));
    connect(m_comboBackAction, &QAction::triggered, this, &BrowserWindow::handleComboBackRequested);
    navigationBar->addAction(m_comboBackAction);

    //combo進むボタン
    m_comboForwardAction = new QAction(this);
    m_comboForwardAction->setIconVisibleInMenu(false);
    m_comboForwardAction->setIcon(QIcon(QStringLiteral(":cmb-next.svg")));
    m_comboForwardAction->setToolTip(tr("Forward combo pid"));
    connect(m_comboForwardAction, &QAction::triggered, this, &BrowserWindow::handleComboForwardRequested);
    navigationBar->addAction(m_comboForwardAction);

    //combo再読込ボタン
    m_comboReloadAction = new QAction(this);
    m_comboReloadAction->setIconVisibleInMenu(false);
    m_comboReloadAction->setIcon(QIcon(QStringLiteral(":cmb-refresh.svg")));
    connect(m_comboReloadAction, &QAction::triggered, this, &BrowserWindow::handleComboReloadRequested);
    navigationBar->addAction(m_comboReloadAction);

    //URL選択コンボ
    m_urlComboBox = new QComboBox(this);
    m_urlComboBox->setEditable(true);
    connect(m_urlComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
        handleComboCurrentChanged(index);
    });
    navigationBar->addWidget(m_urlComboBox);

    //スペーサー
    QWidget *spacer = new QWidget(this);
    spacer->setFixedWidth(8);
    navigationBar->addWidget(spacer);

    //URL欄
    m_urlLineEdit = new QLineEdit(this);
    m_favAction = new QAction(this);
    m_urlLineEdit->addAction(m_favAction, QLineEdit::LeadingPosition);
    m_urlLineEdit->setClearButtonEnabled(true);
    navigationBar->addWidget(m_urlLineEdit);

    //Homeボタン
    m_homeUrlAction = new QAction(this);
    m_homeUrlAction->setIconVisibleInMenu(false);
    m_homeUrlAction->setIcon(QIcon(QStringLiteral(":go-home.svg")));
    m_homeUrlAction->setToolTip(tr("Back to the default page."));
    connect(m_homeUrlAction, &QAction::triggered, [this]() {
        currentTab()->setUrl(m_homeUrl);
    });
    navigationBar->addAction(m_homeUrlAction);

    //戻るボタン
    m_historyBackAction = new QAction(this);
    QList<QKeySequence> backShortcuts = QKeySequence::keyBindings(QKeySequence::Back);
    for(auto it = backShortcuts.begin(); it != backShortcuts.end();) {
        // Chromium already handles navigate on backspace when appropriate.
        if((*it)[0] == Qt::Key_Backspace)
            it = backShortcuts.erase(it);
        else
            ++it;
    }
    // For some reason Qt doesn't bind the dedicated Back key to Back.
    backShortcuts.append(QKeySequence(Qt::Key_Back));
    m_historyBackAction->setShortcuts(backShortcuts);
    m_historyBackAction->setIconVisibleInMenu(false);
    m_historyBackAction->setIcon(QIcon(QStringLiteral(":go-previous.svg")));
    m_historyBackAction->setToolTip(tr("Go Back in history"));
    connect(m_historyBackAction, &QAction::triggered, [this]() {
        m_tabWidget->triggerWebPageAction(QWebEnginePage::Back);
    });
    navigationBar->addAction(m_historyBackAction);

    //進むボタン
    m_historyForwardAction = new QAction(this);
    QList<QKeySequence> fwdShortcuts = QKeySequence::keyBindings(QKeySequence::Forward);
    for(auto it = fwdShortcuts.begin(); it != fwdShortcuts.end();) {
        if(((*it)[0] & Qt::Key_unknown) == Qt::Key_Backspace)
            it = fwdShortcuts.erase(it);
        else
            ++it;
    }
    fwdShortcuts.append(QKeySequence(Qt::Key_Forward));
    m_historyForwardAction->setShortcuts(fwdShortcuts);
    m_historyForwardAction->setIconVisibleInMenu(false);
    m_historyForwardAction->setIcon(QIcon(QStringLiteral(":go-next.svg")));
    m_historyForwardAction->setToolTip(tr("Go Forward in history"));
    connect(m_historyForwardAction, &QAction::triggered, [this]() {
        m_tabWidget->triggerWebPageAction(QWebEnginePage::Forward);
    });
    navigationBar->addAction(m_historyForwardAction);

    //更新/更新停止ボタン
    m_stopReloadAction = new QAction(this);
    connect(m_stopReloadAction, &QAction::triggered, [this]() {
        m_tabWidget->triggerWebPageAction(QWebEnginePage::WebAction(m_stopReloadAction->data().toInt()));
    });
    navigationBar->addAction(m_stopReloadAction);

    //ダウンロードマネージャーボタン
    auto downloadsAction = new QAction(this);
    downloadsAction->setIcon(QIcon(QStringLiteral(":go-bottom.svg")));
    downloadsAction->setToolTip(tr("Show downloads"));
    navigationBar->addAction(downloadsAction);
    connect(downloadsAction, &QAction::triggered, [this]() {
        m_browser->downloadManagerWidget().show();
    });


    auto jsAction = new QAction(this);
    jsAction->setIcon(QIcon(QStringLiteral(":ninja.svg")));
    connect(jsAction, &QAction::triggered, [this]() {
        currentTab()->page()->runJavaScript(m_jsUtil->css_cut());
    });
    navigationBar->addAction(jsAction);

    return navigationBar;

}


//基本的なウェブアクションの有効無効化のためのスロット
void BrowserWindow::handleWebActionEnabledChanged(QWebEnginePage::WebAction action, bool enabled)
{
    switch (action) {
    case QWebEnginePage::Back:
        m_historyBackAction->setEnabled(enabled);
        break;
    case QWebEnginePage::Forward:
        m_historyForwardAction->setEnabled(enabled);
        break;
    case QWebEnginePage::Reload:
        m_reloadAction->setEnabled(enabled);
        break;
    case QWebEnginePage::Stop:
        m_stopAction->setEnabled(enabled);
        break;
    default:
        qWarning("Unhandled webActionChanged signal");
    }
}


//アプリウィンドウのタイトル変更のためのスロット
void BrowserWindow::handleWebViewTitleChanged(const QString &title)
{
    QString preffix = m_profile->isOffTheRecord() ?
                tr("m2browser (Incognite)") : tr("m2browser");

    if(title.isEmpty())
        setWindowTitle(preffix);
    else
        setWindowTitle(preffix + " - " + title);
}


//新しいアプリウィンドウを開く（追加）ためのスロット
void BrowserWindow::handleNewWindowTriggered()
{
    BrowserWindow *window = m_browser->createWindow();
    window->tabWidget()->setUrl(m_homeUrl);
    window->m_urlLineEdit->setFocus();
}


//新しいプライベートアプリウィンドウを開く（追加）ためのスロット
void BrowserWindow::handleNewIncognitoWindowTriggered()
{
    BrowserWindow *window = m_browser->createWindow(true); //offTheRecord=true
    window->tabWidget()->setUrl(m_homeUrl);
    window->m_urlLineEdit->setFocus();
}


//ファイルを開く要求に応じてファイルを開くためのスロット
void BrowserWindow::handleFileOpenTriggered()
{
    QUrl url = QFileDialog::getOpenFileUrl(
                this,
                tr("Open Web Resource"),
                QString(),
                tr("Web Resources (*.html *.htm *.svg *.png *.gif *.svgz);;All files (*.*)")
    );

    if(url.isEmpty())
        return;
    currentTab()->setUrl(url);
}


//ページ内検索の要求に応じて検索（ダイアログ表示）するためのスロット
void BrowserWindow::handleFindActionTriggered()
{
    if(!currentTab())
        return;

    bool ok = false;
    QString search = QInputDialog::getText(
                this,
                tr("Find"),
                tr("Find:"),
                QLineEdit::Normal,
                m_lastSearch,
                &ok
    );

    if(ok && !search.isEmpty()) {
        m_lastSearch = search;

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        currentTab()->findText(m_lastSearch);
#else
        currentTab()->findText(m_lastSearch, 0, [this](bool found) {
            if(!found)
                statusBar()->showMessage(tr("\"%1\" not found.").arg(m_lastSearch));
        });
#endif

    }
}


//アプリウィンドウを閉じるシグナルを関知して処理するためのスロット
void BrowserWindow::closeEvent(QCloseEvent *event)
{
    if(m_tabWidget->count() > 1) {
        int ret = QMessageBox::warning(
                    this,
                    tr("Confirm close"),
                    tr("Are you sure you want to close the window ?\n"
                       "There are %1 tabs open.").arg(m_tabWidget->count()),
                    QMessageBox::Yes | QMessageBox::No,
                    QMessageBox::No
        );

        if(ret == QMessageBox::No) {
            event->ignore();
            return;
        }
    }

    event->accept();
    deleteLater();
}


//TabWidgetインスタンスポインタを返す
TabWidget *BrowserWindow::tabWidget() const
{
    return m_tabWidget;
}


//TabWidgetインスタンス経由でカレントタブポインタを返す
WebView *BrowserWindow::currentTab() const
{
    return m_tabWidget->currentWebView();
}


//ページロード進捗に応じた更新/更新停止ボタンに切り換えるためのスロット
void BrowserWindow::handleWebViewLoadProgress(int progress)
{
    static QIcon stopIcon(QStringLiteral(":process-stop.svg"));
    static QIcon reloadIcon(QStringLiteral(":view-refresh.svg"));

    if(0 < progress && progress < 100) {
        m_stopReloadAction->setData(QWebEnginePage::Stop);
        m_stopReloadAction->setIcon(stopIcon);
        m_stopReloadAction->setToolTip(tr("Stop loading the current page"));
        m_progressBar->setValue(progress);
    } else {
        m_stopReloadAction->setData(QWebEnginePage::Reload);
        m_stopReloadAction->setIcon(reloadIcon);
        m_stopReloadAction->setToolTip(tr("Reload the current page"));
        m_progressBar->setValue(0);
    }
}


//アプリウィンドウ表示をきりかえるためのスロット
void BrowserWindow::handleShowWindowTriggered()
{
    //シグナルを送信したオブジェクトへのポインタをQActionにキャストしアプリウィンドウ情報を得る
    if(QAction *action = qobject_cast<QAction*>(sender())) {
        int offset = action->data().toInt();
        QVector<BrowserWindow*> windows = m_browser->windows();
        windows.at(offset)->activateWindow();
        windows.at(offset)->currentTab()->setFocus();
    }
}


//DevToolsの表示要求に応じてDevToolsウィンドウを表示するためのスロット
void BrowserWindow::handleDevToolsRequested(QWebEnginePage *source)
{
    source->setDevToolsPage(m_browser->createDevToolsWindow()->currentTab()->page());
    source->triggerAction(QWebEnginePage::InspectElement);
}


//ページ内検索結果が最後に到達した際の処理をするためのスロット
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
void BrowserWindow::handleFindTextFinished(const QWebEngineFindTextResult &result)
{
    if(result.numberOfMatches() == 0) {
        statusBar()->showMessage(tr("\"%1\" not found.").arg(m_lastSearch));
    } else {
        statusBar()->showMessage(tr("\"%1\" found: %2/%3").arg(
                                     m_lastSearch,
                                     QString::number(result.activeMatch()),
                                     QString::number(result.numberOfMatches()
        )));
    }
}
#endif


//TSVファイル読み込み要求の処理のためのスロット
void BrowserWindow::handleTsvFileOpenRequested()
{
    QString path = QFileDialog::getOpenFileName(
                this,
                tr("Choose a tsv file"),
                QString(),
                tr("tsv file(*.txt)")
    );

    if(path.isEmpty())
        return;

    if(m_urlArr.size() > 0) {
        m_urlArr.clear();
        m_urlComboBox->clear();
    }

    QFile fp(path);
    if(!fp.open(QIODevice::ReadOnly))
        return;

    QTextStream sr(&fp);
    while(!sr.atEnd()) {
        QString line = sr.readLine(0);
        QList<QString> tmp = line.split("\t");
        QVector<QString> row;
        row.append(tmp.at(0));
        row.append(tmp.at(1));
        m_urlArr.append(row);
    }

    fp.close();

    m_urlArrIndex = 0;
    foreach(const QVector<QString> &row, m_urlArr) {
        m_urlComboBox->addItem(row.at(0));
    }

    const QVector<QString> row = m_urlArr.at(0);
    currentTab()->setUrl(QUrl::fromUserInput(row.at(1)));
    m_urlComboBox->setCurrentIndex(0);

}


//コンボで選択したページに切替
void BrowserWindow::handleComboCurrentChanged(int index)
{
    if(m_urlArr.size() == 0)
        return;

    m_urlArrIndex = index;
    const QVector<QString> row = m_urlArr.at(index);
    currentTab()->setUrl(QUrl::fromUserInput(row.at(1)));
}


//前のコンボに進む要求時の処理のためのスロット
void BrowserWindow::handleComboBackRequested()
{
    if(m_urlArr.size() == 0)
        return;
    if(m_urlArrIndex == 0) {
        QMessageBox::information(
                    this,
                    tr("Error"),
                    tr("Can't back page.")
        );
        return;
    }
    m_urlArrIndex--;
    const QVector<QString> row = m_urlArr.at(m_urlArrIndex);
    currentTab()->setUrl(QUrl::fromUserInput(row.at(1)));
    m_urlComboBox->setCurrentIndex(m_urlArrIndex);
}


//次のコンボに進む要求時の処理のためのスロット
void BrowserWindow::handleComboForwardRequested()
{
    if(m_urlArr.size() == 0)
        return;
    if(m_urlArrIndex == (m_urlArr.size() - 1)) {
        QMessageBox::information(
                    this,
                    tr("Error"),
                    tr("Can't forward page.")
        );
        return;
    }
    m_urlArrIndex++;
    const QVector<QString> row = m_urlArr.at(m_urlArrIndex);
    currentTab()->setUrl(QUrl::fromUserInput(row.at(1)));
    m_urlComboBox->setCurrentIndex(m_urlArrIndex);
}


//コンボ再読込要求時の処理のためのスロット
void BrowserWindow::handleComboReloadRequested()
{
    if(m_urlArr.size() == 0)
        return;
    const QVector<QString> row = m_urlArr.at(m_urlArrIndex);
    currentTab()->setUrl(QUrl::fromUserInput(row.at(1)));
}

