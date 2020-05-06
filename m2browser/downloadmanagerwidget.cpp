#include "downloadmanagerwidget.h"
#include "ui_downloadmanagerwidget.h"

DownloadManagerWidget::DownloadManagerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DownloadManagerWidget)
{
    ui->setupUi(this);
}

DownloadManagerWidget::~DownloadManagerWidget()
{
    delete ui;
}
