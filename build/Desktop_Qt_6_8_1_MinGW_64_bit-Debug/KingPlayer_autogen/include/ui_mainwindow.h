/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QWidget *SDL_Widget;
    QWidget *widget_2;
    QHBoxLayout *horizontalLayout_2;
    QLabel *now_time;
    QSlider *progress_bar;
    QLabel *total_time;
    QWidget *widget;
    QHBoxLayout *horizontalLayout;
    QToolButton *toolButton;
    QToolButton *toolButton_2;
    QSpacerItem *horizontalSpacer;
    QComboBox *speedComboBox;
    QToolButton *last_btn;
    QToolButton *start_btn;
    QToolButton *stop_btn;
    QToolButton *next_btn;
    QSpacerItem *horizontalSpacer_2;
    QToolButton *toolButton_3;
    QSlider *horizontalSlider_2;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(665, 519);
        MainWindow->setStyleSheet(QString::fromUtf8("background-color: rgb(55, 55, 55);\n"
"color: rgb(255, 255, 255);\n"
"MainWindow{\n"
"border:3px solid #000000;\n"
"}"));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName("verticalLayout");
        SDL_Widget = new QWidget(centralwidget);
        SDL_Widget->setObjectName("SDL_Widget");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(SDL_Widget->sizePolicy().hasHeightForWidth());
        SDL_Widget->setSizePolicy(sizePolicy);

        verticalLayout->addWidget(SDL_Widget);

        widget_2 = new QWidget(centralwidget);
        widget_2->setObjectName("widget_2");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(widget_2->sizePolicy().hasHeightForWidth());
        widget_2->setSizePolicy(sizePolicy1);
        horizontalLayout_2 = new QHBoxLayout(widget_2);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        now_time = new QLabel(widget_2);
        now_time->setObjectName("now_time");
        now_time->setTextFormat(Qt::TextFormat::MarkdownText);

        horizontalLayout_2->addWidget(now_time);

        progress_bar = new QSlider(widget_2);
        progress_bar->setObjectName("progress_bar");
        progress_bar->setOrientation(Qt::Orientation::Horizontal);

        horizontalLayout_2->addWidget(progress_bar);

        total_time = new QLabel(widget_2);
        total_time->setObjectName("total_time");

        horizontalLayout_2->addWidget(total_time);


        verticalLayout->addWidget(widget_2);

        widget = new QWidget(centralwidget);
        widget->setObjectName("widget");
        sizePolicy1.setHeightForWidth(widget->sizePolicy().hasHeightForWidth());
        widget->setSizePolicy(sizePolicy1);
        horizontalLayout = new QHBoxLayout(widget);
        horizontalLayout->setObjectName("horizontalLayout");
        toolButton = new QToolButton(widget);
        toolButton->setObjectName("toolButton");
        toolButton->setStyleSheet(QString::fromUtf8("border:none;"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/new/prefix1/image/\347\264\240\346\235\220.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        toolButton->setIcon(icon);
        toolButton->setIconSize(QSize(20, 20));

        horizontalLayout->addWidget(toolButton);

        toolButton_2 = new QToolButton(widget);
        toolButton_2->setObjectName("toolButton_2");
        toolButton_2->setStyleSheet(QString::fromUtf8("border:none;\n"
""));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/new/prefix1/image/\345\210\227\350\241\250\345\275\242\345\274\217.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        toolButton_2->setIcon(icon1);
        toolButton_2->setIconSize(QSize(20, 20));

        horizontalLayout->addWidget(toolButton_2);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        speedComboBox = new QComboBox(widget);
        speedComboBox->addItem(QString());
        speedComboBox->addItem(QString());
        speedComboBox->addItem(QString());
        speedComboBox->addItem(QString());
        speedComboBox->addItem(QString());
        speedComboBox->addItem(QString());
        speedComboBox->setObjectName("speedComboBox");

        horizontalLayout->addWidget(speedComboBox);

        last_btn = new QToolButton(widget);
        last_btn->setObjectName("last_btn");
        last_btn->setStyleSheet(QString::fromUtf8("border:none;"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/new/prefix1/image/last.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        last_btn->setIcon(icon2);
        last_btn->setIconSize(QSize(20, 20));

        horizontalLayout->addWidget(last_btn);

        start_btn = new QToolButton(widget);
        start_btn->setObjectName("start_btn");
        start_btn->setStyleSheet(QString::fromUtf8("border:none;"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/new/prefix1/image/\346\222\255\346\224\276.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        start_btn->setIcon(icon3);
        start_btn->setIconSize(QSize(20, 20));

        horizontalLayout->addWidget(start_btn);

        stop_btn = new QToolButton(widget);
        stop_btn->setObjectName("stop_btn");
        stop_btn->setStyleSheet(QString::fromUtf8("border:none;"));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/new/prefix1/image/\346\255\243\346\226\271\345\275\242.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        stop_btn->setIcon(icon4);
        stop_btn->setIconSize(QSize(20, 20));

        horizontalLayout->addWidget(stop_btn);

        next_btn = new QToolButton(widget);
        next_btn->setObjectName("next_btn");
        next_btn->setStyleSheet(QString::fromUtf8("border:none;"));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/new/prefix1/image/next.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        next_btn->setIcon(icon5);
        next_btn->setIconSize(QSize(20, 20));

        horizontalLayout->addWidget(next_btn);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        toolButton_3 = new QToolButton(widget);
        toolButton_3->setObjectName("toolButton_3");
        toolButton_3->setStyleSheet(QString::fromUtf8("border:none;"));
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/new/prefix1/image/\351\237\263\351\207\217.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        toolButton_3->setIcon(icon6);
        toolButton_3->setIconSize(QSize(20, 20));

        horizontalLayout->addWidget(toolButton_3);

        horizontalSlider_2 = new QSlider(widget);
        horizontalSlider_2->setObjectName("horizontalSlider_2");
        horizontalSlider_2->setOrientation(Qt::Orientation::Horizontal);

        horizontalLayout->addWidget(horizontalSlider_2);


        verticalLayout->addWidget(widget);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 665, 18));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        now_time->setText(QCoreApplication::translate("MainWindow", "00:00:00", nullptr));
        total_time->setText(QCoreApplication::translate("MainWindow", "00.00.00", nullptr));
        toolButton->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
        toolButton_2->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
        speedComboBox->setItemText(0, QCoreApplication::translate("MainWindow", "2.0x", nullptr));
        speedComboBox->setItemText(1, QCoreApplication::translate("MainWindow", "1.5x", nullptr));
        speedComboBox->setItemText(2, QCoreApplication::translate("MainWindow", "1.25x", nullptr));
        speedComboBox->setItemText(3, QCoreApplication::translate("MainWindow", "1.0x", nullptr));
        speedComboBox->setItemText(4, QCoreApplication::translate("MainWindow", "0.75x", nullptr));
        speedComboBox->setItemText(5, QCoreApplication::translate("MainWindow", "0.5x", nullptr));

        last_btn->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
        start_btn->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
        stop_btn->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
        next_btn->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
        toolButton_3->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
