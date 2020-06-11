#include "webinspector.h"
#include "webview.h"
#include "webpage.h"

QList<QWebEngineView*> WebInspector::s_views;

WebInspector::WebInspector(QWidget *parent) :
    QWebEngineView(parent),
    m_view(nullptr)
{
    setAttribute(Qt::WA_DeleteOnClose);
    //setObjectName(QSL("web-inspector"));
    setMinimumHeight(80);

    registerView(this);

    connect(page(), &QWebEnginePage::windowCloseRequested, this, &WebInspector::deleteLater);
    connect(page(), &QWebEnginePage::loadFinished, this, &WebInspector::loadFinished);
}


WebInspector::~WebInspector()
{
    if(m_view && hasFocus()) {
        m_view->setFocus();
    }

    unregisterView(this);
}


void WebInspector::setView(WebView *view)
{
    m_view = view;
    Q_ASSERT(isEnabled());

    page()->setInspectedPage(m_view->page());
    //connect(m_view, &WebView::pageChanged, this, &WebInspector::deleteLater);
    connect(m_view, &WebView::urlChanged, this, &WebInspector::deleteLater);
}


void WebInspector::inspectElement()
{
    m_inspectElement = true;
}


bool WebInspector::isEnabled()
{
    if(!qEnvironmentVariableIsSet("QTWEBENGINE_REMOTE_DEBUGGING")) {
        return false;
    }
    return true;
}


void WebInspector::pushView(QWebEngineView *view)
{
    s_views.removeOne(view);
    s_views.prepend(view);
}


void WebInspector::registerView(QWebEngineView *view)
{
    s_views.prepend(view);
}


void WebInspector::unregisterView(QWebEngineView *view)
{
    s_views.removeOne(view);
}


void WebInspector::loadFinished()
{
    if(!isWindow()) {
        page()->runJavaScript(QLatin1String(
                                  "var button = Components.dockController._closeButton;"
                                  "button.setVisible(true);"
                                  "button.element.onmouseup = function() {"
                                  "    window.close();"
                                  "};"
        ));
    }

    if(m_inspectElement) {
        m_view->triggerPageAction(QWebEnginePage::InspectElement);
        m_inspectElement = false;
    }
}


QSize WebInspector::sizeHint() const
{
    if(isWindow()) {
        return m_windowSize;
    }
    QSize s = QWebEngineView::sizeHint();
    s.setHeight(m_height);
    return s;
}


void WebInspector::keyPressEvent(QKeyEvent *event)
{
    Q_UNUSED(event);
}


void WebInspector::keyReleaseEvent(QKeyEvent *event)
{
    Q_UNUSED(event);
}























