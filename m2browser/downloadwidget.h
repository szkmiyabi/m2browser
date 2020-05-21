#ifndef DOWNLOADWIDGET_H
#define DOWNLOADWIDGET_H

#include "ui_downloadwidget.h"

#include <QFrame>
#include <QElapsedTimer>

QT_BEGIN_NAMESPACE
class QWebEngineDownloadItem;       //QWebEngineDownloadItemクラスの暗黙参照
QT_END_NAMESPACE


class DownloadWidget final : public QFrame, public Ui::DownloadWidget
{
    Q_OBJECT

public:

    //コンストラクタ
    explicit DownloadWidget(QWebEngineDownloadItem *download, QWidget *parent = nullptr);

signals:

    //ダウンロードアイテム削除要求シグナル
    void removeClicked(DownloadWidget *self);

private slots:

    //ダウンロードアイテムの表示更新をするためのスロット
    void updateWidget();

private:

    //ビット数を単位変換して返す
    QString withUnit(qreal bytes);

    QWebEngineDownloadItem *m_download;     //ダウンロードアイテムのポインタ
    QElapsedTimer m_timeAdded;              //処理時間計測用
};

#endif // DOWNLOADWIDGET_H
