#ifndef DOWNLOADMANAGERWIDGET_H
#define DOWNLOADMANAGERWIDGET_H

#include <QWidget>

namespace Ui {
class DownloadManagerWidget;
}

class DownloadManagerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DownloadManagerWidget(QWidget *parent = nullptr);
    ~DownloadManagerWidget();

private:
    Ui::DownloadManagerWidget *ui;
};

#endif // DOWNLOADMANAGERWIDGET_H
