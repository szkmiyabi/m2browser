#include "downloadwidget.h"

#include <QFileInfo>
#include <QUrl>
#include <QWebEngineDownloadItem>

//コンストラクタ
DownloadWidget::DownloadWidget(QWebEngineDownloadItem *download, QWidget *parent) :
    QFrame(parent),
    m_download(download),
    m_timeAdded()
{

    //処理時間計測の開始
    m_timeAdded.start();
    setupUi(this);

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    m_dstName->setText(m_download->downloadFileName());
#else
    m_dstName->setText(QFileInfo(m_download->path()).fileName());
#endif

    m_srcUrl->setText(m_download->url().toDisplayString());

    //キャンセルボタンクリック時の紐付け
    connect(m_cancelButton, &QPushButton::clicked,
        [this](bool) {
            if(m_download->state() == QWebEngineDownloadItem::DownloadInProgress)
                m_download->cancel();
            else
                emit removeClicked(this);
        }
    );

    //ダウンロードの進捗状況に応じた紐付け
    connect(m_download, &QWebEngineDownloadItem::downloadProgress, this, &DownloadWidget::updateWidget);

    //ダウンロード処理の状態変化に応じた紐付け
    connect(m_download, &QWebEngineDownloadItem::stateChanged, this, &DownloadWidget::updateWidget);

    updateWidget();
}


//ダウンロードアイテムの表示更新をするためのスロット
void DownloadWidget::updateWidget()
{
    qreal totalBytes = m_download->totalBytes();
    qreal receivedBytes = m_download->receivedBytes();
    qreal bytesPerSecond = receivedBytes / m_timeAdded.elapsed() * 1000;

    auto state = m_download->state();

    switch (state) {

    //ダウンロード開始のとき
    case QWebEngineDownloadItem::DownloadRequested:
        //変数領域に配置しない処理
        Q_UNREACHABLE();
        break;

    //ダウンロード中のとき
    case QWebEngineDownloadItem::DownloadInProgress:
        if(totalBytes >= 0) {
            m_progressBar->setValue(qRound(100 * receivedBytes / totalBytes));
            m_progressBar->setDisabled(false);
            m_progressBar->setFormat(
                        tr("%p% - %1 of %2 downloaded - %3/s")
                        .arg(withUnit(receivedBytes))
                        .arg(withUnit(totalBytes))
                        .arg(withUnit(bytesPerSecond))
            );
        } else {
            m_progressBar->setValue(0);
            m_progressBar->setDisabled(false);
            m_progressBar->setFormat(
                        tr("unknown size - %1 downloaded - %2/s")
                        .arg(withUnit(receivedBytes))
                        .arg(withUnit(bytesPerSecond))
            );
        }
        break;

    //ダウンロードが完了したとき
    case QWebEngineDownloadItem::DownloadCompleted:
        m_progressBar->setValue(100);
        m_progressBar->setDisabled(true);
        m_progressBar->setFormat(
                    tr("completed - %1 downloaded - %2/s")
                    .arg(withUnit(receivedBytes))
                    .arg(withUnit(bytesPerSecond))
        );
        break;

    //ダウンロードがキャンセルされたとき
    case QWebEngineDownloadItem::DownloadCancelled:
        m_progressBar->setValue(0);
        m_progressBar->setDisabled(true);
        m_progressBar->setFormat(
                    tr("cancelled - %1 downloaded - %2/s")
                    .arg(withUnit(receivedBytes))
                    .arg(withUnit(bytesPerSecond))
        );
        break;

    //ダウンロードが中断したとき
    case QWebEngineDownloadItem::DownloadInterrupted:
        m_progressBar->setValue(0);
        m_progressBar->setDisabled(true);
        m_progressBar->setFormat(
                    tr("interrupted: - %1")
                    .arg(m_download->interruptReasonString())
        );
        break;
    }

    //ダウンロード中はキャンセルボタン表示
    if(state == QWebEngineDownloadItem::DownloadInProgress) {
        static QIcon cancelIcon(QStringLiteral(":process-stop.svg"));
        m_cancelButton->setIcon(cancelIcon);
        m_cancelButton->setToolTip(tr("Stop downloading"));

    //それ以外は削除ボタン表示
    } else {
        static QIcon removeIcon(QStringLiteral(":edit-clear.svg"));
        m_cancelButton->setIcon(removeIcon);
        m_cancelButton->setToolTip(tr("Remove from list"));
    }
}


//ビット数を単位変換して返す
inline QString DownloadWidget::withUnit(qreal bytes)
{
    //ビット演算を駆使して、B、KB、MB、GBに変換
    if(bytes < (1 << 10))
        return tr("%L1 B").arg(bytes);
    else if(bytes < (1 << 20))
        return tr("%L1 KiB").arg(bytes / (1 << 10), 0, 'f', 2);
    else if(bytes < (1 << 30))
        return tr("%L1 MiB").arg(bytes / (1 << 20), 0, 'f', 2);
    else
        return tr("%L1 GiB").arg(bytes / (1 << 30), 0, 'f', 2);
}
