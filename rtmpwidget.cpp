#include "rtmpwidget.h"
#include "ui_rtmpwidget.h"

rtmpWidget::rtmpWidget(QDialog *parent)
    : QDialog(parent)
    , ui(new Ui::rtmpWidget)
{
    ui->setupUi(this);
}

rtmpWidget::~rtmpWidget()
{
    delete ui;
}

void rtmpWidget::on_pushButton_clicked()
{
    rtmpUrl = ui->lineEdit->text();

    accept();
}


void rtmpWidget::on_pushButton_2_clicked()
{

    reject();
}

QString rtmpWidget::getUrl()
{
    return rtmpUrl;
}

