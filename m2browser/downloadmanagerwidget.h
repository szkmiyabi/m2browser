#ifndef DOWNLOADMANAGERWIDGET_H
#define DOWNLOADMANAGERWIDGET_H

#include "ui_downloadmanagerwidget.h"

#include <QWidget>

QT_BEGIN_NAMESPACE
class QWebEngineDownloadItem;       //QWebEngineDownloadItemクラスの暗黙参照
QT_END_NAMESPACE

class DownloadWidget;               //DownloadWidgetクラスを参照

class DownloadManagerWidget final : public QWidget, public Ui::DownloadManagerWidget
{
    Q_OBJECT

public:

    //コンストラクタ
    //explicitを宣言し値渡しでコンストラクタ呼び出し禁止
    explicit DownloadManagerWidget(QWidget *parent = nullptr);

    //ダウンロードマネージャーウィンドウを表示する
    void downloadRequested(QWebEngineDownloadItem *webItem);

private:

    //ダウンロードアイテムを追加（ダウンロードの開始）するためのスロット
    void add(DownloadWidget *downloadWidget);

    //ダウンロードアイテムを削除するためのスロット
    void remove(DownloadWidget *downloadWidget);

    int m_numDownloads;
};

#endif // DOWNLOADMANAGERWIDGET_H
