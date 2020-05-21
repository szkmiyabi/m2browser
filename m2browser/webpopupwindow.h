#ifndef WEBPOPUPWINDOW_H
#define WEBPOPUPWINDOW_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QLineEdit;            //QLineEditクラスの暗黙参照
class QWebEngineProfile;    //QWebEngineProfileクラスの暗黙参照
class QWebEngineView;       //QWebEngineViewクラスの暗黙参照
QT_END_NAMESPACE

class WebView;              //WebViewクラスの参照

class WebPopupWindow : public QWidget
{
    Q_OBJECT

public:

    //コンストラクタ
    WebPopupWindow(QWebEngineProfile *profile);

    //このクラス配下のWebViewクラスインスタンスポインタを返す
    WebView *view() const;

private slots:

    //Webページのジオメトリ変更要求時のためのスロット
    void handleGeometryChangeRequested(const QRect &newGeometry);

private:

    //URL欄＋ファビコンのポインタ
    QLineEdit *m_urlLineEdit;
    QAction *m_favAction;

    //WebViewクラスインスタンスポインタ
    WebView *m_view;
};

#endif // WEBPOPUPWINDOW_H
