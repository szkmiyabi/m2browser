#include "webpage.h"
#include "webpopupwindow.h"
#include "webview.h"

#include <QAction>
#include <QIcon>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QWindow>

//コンストラクタ
WebPopupWindow::WebPopupWindow(QWebEngineProfile *profile) :
    m_urlLineEdit(new QLineEdit(this)),
    m_favAction(new QAction(this)),
    m_view(new WebView(this))
{

    //このクラスはポップアップ表示のアプリウィンドウセットアアップの役割
    setAttribute(Qt::WA_DeleteOnClose);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    layout->addWidget(m_urlLineEdit);
    layout->addWidget(m_view);

    m_view->setPage(new WebPage(profile, m_view));
    m_view->setFocus();

    m_urlLineEdit->setReadOnly(true);
    m_urlLineEdit->addAction(m_favAction, QLineEdit::LeadingPosition);

    //通常のアプリウィンドウと同じようにイベントと紐付け
    connect(m_view, &WebView::titleChanged, this, &QWidget::setWindowTitle);
    connect(m_view, &WebView::urlChanged, [this](const QUrl &url){
        m_urlLineEdit->setText(url.toDisplayString());
    });
    connect(m_view, &WebView::favIconChanged, m_favAction, &QAction::setIcon);
    connect(m_view->page(), &WebPage::windowCloseRequested, this, &QWidget::close);
    //Webページのジオメトリ変更要求時に紐付け
    connect(m_view->page(), &WebPage::geometryChangeRequested, this, &WebPopupWindow::handleGeometryChangeRequested);
}


//このクラス配下のWebViewクラスインスタンスポインタを返す
WebView *WebPopupWindow::view() const
{
    return m_view;
}


//Webページのジオメトリ変更要求時のためのスロット
void WebPopupWindow::handleGeometryChangeRequested(const QRect &newGeometry)
{

    if(QWindow *window = windowHandle())
        setGeometry(newGeometry.marginsRemoved(window->frameMargins()));
    show();
    m_view->setFocus();

}
