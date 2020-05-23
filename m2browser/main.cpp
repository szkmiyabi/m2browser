#include "browser.h"
#include "browserwindow.h"
#include "tabwidget.h"
#include <QApplication>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QDebug>


//コマンドラインパラメータの取得
QUrl commandLineUrlArgument()
{
    const QStringList args = QCoreApplication::arguments();
    for(const QString &arg : args.mid(1)) {
        if(!arg.startsWith(QLatin1Char('-')))
            return QUrl::fromUserInput(arg);
    }
    return QUrl(QStringLiteral("https://www.google.com/"));
}


//メイン関数
int main(int argc, char **argv)
{

    //アプリの設定
    QCoreApplication::setOrganizationName("JCI-TN");
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    //appのインスタンスを生成しアイコン付与
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(QStringLiteral(":AppLogoColor.png")));

    //QWebEngineの設定
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::DnsPrefetchEnabled, true);
    QWebEngineProfile::defaultProfile()->setUseForGlobalCertificateVerification();
#endif

    //コマンドラインから（あれば）URLを読み取る
    QUrl url = commandLineUrlArgument();

    //browser（ウィンドウマネージャ的クラス）のインスタンス
    Browser browser(url);
    //最低1個のウィンドウは生成しておく
    //browserインスタンスからBrowserWindowインスタンスのポインタを受け取る
    BrowserWindow *window = browser.createWindow();
    //windowポインタからTabWidgetクラスのsetUrlを呼び出し
    window->tabWidget()->setUrl(url);

    //イベントループ開始（要するにアプリの実行）
    return app.exec();

}
