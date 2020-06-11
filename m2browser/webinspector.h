#ifndef WEBINSPECTOR_H
#define WEBINSPECTOR_H

#include <QWebEngineView>

class WebView;

class WebInspector : public QWebEngineView
{
    Q_OBJECT
public:
    explicit WebInspector(QWidget *parent = nullptr);
    ~WebInspector();

    void setView(WebView *view);
    void inspectElement();

    QSize sizeHint() const override;

    static bool isEnabled();
    static void pushView(QWebEngineView *view);
    static void registerView(QWebEngineView *view);
    static void unregisterView(QWebEngineView *view);

private slots:
    void loadFinished();

private:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    static QList<QWebEngineView*> s_views;
    int m_height;
    QSize m_windowSize;
    bool m_inspectElement = false;
    WebView *m_view;
};

#endif // WEBINSPECTOR_H
