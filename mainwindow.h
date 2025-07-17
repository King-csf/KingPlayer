#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QTimer>
#include <QFileDialog>
#include <QString>
#include <thread>
#include <chrono>
#include <future>
#include "player.h"
extern "C"
{
#include "SDL3/SDL.h"
}

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();
    void setSDLWindow();
    void showEvent(QShowEvent *event);
private slots:
    void on_toolButton_clicked();

    void on_speedComboBox_currentIndexChanged(int index);

private:
    Ui::MainWindow *ui;
    SDL_Window * sdlWindow ;
    SDL_Renderer * render ;
    QString filename;
    Player * player;

    std::thread muxerThread;
    std::thread viDecoderThread;
    std::thread auDecoderThread;
    std::thread delayVideoThread;
    std::thread playAudioThread;

    //回收线程
    void stopThread();
    void startThread();
    //初始化进度条
    void initProgressBar();
    QTimer *timer;  //定时器
    int curSec;     //当前进度条
    double totalTime; // 总进度
    float speed; //播放速率
};
#endif // MAINWINDOW_H
