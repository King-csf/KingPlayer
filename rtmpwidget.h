#ifndef RTMPWIDGET_H
#define RTMPWIDGET_H

#include <QWidget>
#include <QDialog>

namespace Ui {
class rtmpWidget;
}

class rtmpWidget : public QDialog
{
    Q_OBJECT

public:
    explicit rtmpWidget(QDialog *parent = nullptr);
    ~rtmpWidget();
    QString getUrl();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::rtmpWidget *ui;
    QString rtmpUrl;
};

#endif // RTMPWIDGET_H
