#include "downloadmanagerwidget.h"

#include "browser.h"
#include "browserwindow.h"
#include "downloadwidget.h"

#include <QFileDialog>
#include <QDir>
#include <QWebEngineDownloadItem>

//コンストラクタ
DownloadManagerWidget::DownloadManagerWidget(QWidget *parent) :
    QWidget(parent),
    m_numDownloads(0)
{
    setupUi(this);
}


//ダウンロードマネージャーウィンドウを表示する
void DownloadManagerWidget::downloadRequested(QWebEngineDownloadItem *download)
{

    Q_ASSERT(download && download->state() == QWebEngineDownloadItem::DownloadRequested);

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QString path = QFileDialog::getSaveFileName(this, tr("Save as"), QDir(download->downloadDirectory()).filePath(download->downloadFileName()));
#else
    QString path = QFileDialog::getSaveFileName(this, tr("Save as"), download->path());
#endif

    if(path.isEmpty())
        return;

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    download->setDownloadDirectory(QFileInfo(path).path());
    download->setDownloadFileName(QFileInfo(path).fileName());
#else
    download->setPath(path);
#endif

    download->accept();
    add(new DownloadWidget(download));

    show();
}


//ダウンロードアイテムを追加（ダウンロードの開始）するためのスロット
void DownloadManagerWidget::add(DownloadWidget *downloadWidget)
{

    //ダウンロードアイテム削除ボタンクリック時の紐付け
    connect(downloadWidget, &DownloadWidget::removeClicked, this, &DownloadManagerWidget::remove);

    m_itemsLayout->insertWidget(0, downloadWidget, 0, Qt::AlignTop);
    if(m_numDownloads++ == 0)
        m_zeroItemsLabel->hide();

}


//ダウンロードアイテムを削除するためのスロット
void DownloadManagerWidget::remove(DownloadWidget *downloadWidget)
{

    m_itemsLayout->removeWidget(downloadWidget);
    downloadWidget->deleteLater();
    if(--m_numDownloads == 0)
        m_zeroItemsLabel->show();

}
