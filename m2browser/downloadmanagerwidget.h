#ifndef DOWNLOADMANAGERWIDGET_H
#define DOWNLOADMANAGERWIDGET_H

#include "ui_downloadmanagerwidget.h"

#include <QWidget>

QT_BEGIN_NAMESPACE
class QWebEngineDownloadItem;
QT_END_NAMESPACE

class DownloadWidget;

class DownloadManagerWidget final : public QWidget, public Ui::DownloadManagerWidget
{
    //The Q_OBJECT macro must appear in the private section of a class definition that
    //declares its own signals and slots or that uses other services provided by Qt's
    //meta-object system.
    Q_OBJECT

public:

    //コンストラクタ
    //explicitを宣言し値渡しでコンストラクタ呼び出し禁止
    explicit DownloadManagerWidget(QWidget *parent = nullptr);

    void downloadRequested(QWebEngineDownloadItem *webItem);

private:    
    void add(DownloadWidget *downloadWidget);
    void remove(DownloadWidget *downloadWidget);

    int m_numDownloads;
};

#endif // DOWNLOADMANAGERWIDGET_H
