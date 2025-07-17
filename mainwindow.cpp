#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("KingPlayer");

    ui->SDL_Widget->setAttribute(Qt::WA_NativeWindow);
    ui->SDL_Widget->setAttribute(Qt::WA_DontCreateNativeAncestors);

    player = Player::getInstance();

    //初始化定时器
    timer = new QTimer(this);
    timer->start(1000);
    curSec = 0;
    ui->speedComboBox->setCurrentIndex(3);
    player->speed = 1;
}

void MainWindow::showEvent(QShowEvent *event)
{

    static bool initialized = false;
    if (!initialized)
    {
        setSDLWindow();
        initialized = true;
    }
    QMainWindow::showEvent(event);
}

void MainWindow::setSDLWindow()
{
    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
    // 在 setupUi 之后、initSDLWindow 之前

    WId win_id = ui->SDL_Widget->winId();
    qDebug() << "SDL_Widget winId() =" << win_id;

    sdlWindow = nullptr;
    render = nullptr;

    //将SDL窗口嵌入Qt窗口
    SDL_PropertiesID props = SDL_CreateProperties();

    bool flag = SDL_SetPointerProperty(props,
                                       SDL_PROP_WINDOW_CREATE_WIN32_HWND_POINTER,
                                       reinterpret_cast<void*>(win_id));
    if(!flag)
    {
        qDebug() << "SDL_SetPointerProperty 失败" << Qt::endl;
    }

    sdlWindow = SDL_CreateWindowWithProperties(props);
    if(!sdlWindow)
    {
        qDebug() << "sdlWindow 创建失败" << Qt::endl;
    }
    //SDL_SetWindowSize(sdlWindow,ui->SDL_Widget->width(),ui->SDL_Widget->height());
    qDebug() <<"w:"  <<ui->SDL_Widget->width() <<"h:"<< ui->SDL_Widget->height() <<Qt::endl;

    render = SDL_CreateRenderer(sdlWindow,NULL);

    /*SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
    SDL_FRect rect{10,10,100,100};
    SDL_RenderClear(render);
    SDL_SetRenderDrawColor(render, 0, 255, 0, 255);
    SDL_RenderFillRect(render,&rect);
    SDL_RenderPresent(render);*/

    //用于渲染线程
    player->render = render;
    player->sdlWindow = sdlWindow;

    SDL_DestroyProperties(props);
}

MainWindow::~MainWindow()
{
    //清理资源
    stopThread();
    SDL_DestroyWindow(sdlWindow);
    SDL_DestroyRenderer(render);
    SDL_Quit();
    delete player;
    delete ui;
}

void MainWindow::on_toolButton_clicked()
{

    //弹出文件对话框
    filename = QFileDialog::getOpenFileName(nullptr
                                            ,"选择文件" //对话框标题
                                            ,QDir::rootPath()   //打开的初始路径
                                            ,"图像文件(*.mp4 *.flv *.ts)");   //文件过滤器
    //未打开文件退出
    if(filename.isEmpty())
    {
        qDebug() << "filename is null.";
        return ;
    }


    stopThread();
    player->destory();
    qDebug() << "open file :"<< filename << Qt::endl;

    player->filename = filename;
    qDebug() << "player->filename :"<< player->filename << Qt::endl;
    startThread();
    //ui->total_time->setText()
}

void MainWindow::initProgressBar()
{
    qDebug() << "run initProgressbar";
    while(!player->isDemuxer);

    curSec = 0;

    totalTime = player->totalTime;
    int hour = totalTime /3600;
    int min = (int)totalTime % 3600 / 60;
    int sec = (int)totalTime % 60;

    qDebug() << "totalTime:" << totalTime
             << player->totalTime;
    ui->total_time->setText(QString("%1:%2:%3").arg(hour).arg(min).arg(sec));

    ui->progress_bar->setRange(0,totalTime);
    //取消旧连接
    disconnect(timer, &QTimer::timeout, this, nullptr);
    //进度条移动
    connect(timer,&QTimer::timeout,this,[&](){
        curSec += 1;
        if(curSec <= totalTime)
        {
            ui->progress_bar->setValue(curSec);
            ui->now_time->setText(QString("%1:%2:%3")
                                      .arg(curSec /3600)
                                      .arg((int)curSec % 3600 / 60)
                                      .arg((int)curSec % 60));
        }
    });
}

//停止线程
void MainWindow::stopThread()
{
    qDebug() << "run stopThread";
    player->isStop = true;
    player->audioPkt.isStop = true;
    player->videoPkt.isStop = true;
    player->videoFrame.isStop = true;
    player->audioFrame.isStop = true;

    player->videoPkt.cv.notify_all();
    player->audioPkt.cv.notify_all();
    player->videoFrame.cv.notify_all();
    player->audioFrame.cv.notify_all();

    if(muxerThread.joinable())
    {

        muxerThread.join();
        qDebug() << "a";
    }
    if(viDecoderThread.joinable())
    {

        viDecoderThread.join();
        qDebug() << "s";
    }
    if(auDecoderThread.joinable())
    {

        auDecoderThread.join();
        qDebug() << "d";
    }
    if(delayVideoThread.joinable())
    {
        delayVideoThread.join();
    }
    if(playAudioThread.joinable())
    {
        playAudioThread.join();
        qDebug() << "f";
    }



}

void MainWindow::startThread()
{
    qDebug() << "run startThread";

    player->isStop = false;
    player->audioPkt.isStop = false;
    player->videoPkt.isStop = false;
    player->videoFrame.isStop = false;
    player->audioFrame.isStop = false;


    muxerThread = std::thread(&player->threadFun,Player::THREAD_DEMUXER);
    //;muxerThread.join();
    viDecoderThread = std::thread(&player->threadFun,Player::THREAD_VIDEO_DECODE);
    auDecoderThread = std::thread(&player->threadFun,Player::THREAD_AUDIO_DECODE);
    delayVideoThread = std::thread(&player->threadFun,Player::THREAD_DELAY_VIDEO);
    playAudioThread = std::thread(&player->threadFun,Player::THREAD_PLAY_AUDIO);
    //viDeocderThread.detach();
    initProgressBar();
}

//调整倍速
void MainWindow::on_speedComboBox_currentIndexChanged(int index)
{
    speed = ui->speedComboBox->currentText().toDouble();
    switch (index)
    {
        case 0:
            player->speed = 2;
            player->isModSpeed = true;
            break;
        case 1:
            player->speed = 1.5;
            player->isModSpeed = true;
            break;
        case 2:
            player->speed = 1.25;
            player->isModSpeed = true;
            break;
        case 3:
            player->speed = 1;
            player->isModSpeed = true;
            break;
        case 4:
            player->speed = 0.75;
            player->isModSpeed = true;
            break;
        case 5:
            player->speed  = 0.5;
            player->isModSpeed = true;
            break;
        default:
            break;
    }
        qDebug() <<"倍速：" << player->speed ;
}


//进度条跳转
void MainWindow::on_progress_bar_sliderPressed()
{
    int value = ui->progress_bar->value();
    ui->progress_bar->setValue(value);
    curSec = value;
    player->isJump = true;
    player->jumpSec = value;
    //qDebug() << "value:" << value;
}


void MainWindow::on_next_btn_clicked()
{
    int value = curSec + JUMP_TIME;
    if(value < 0)
    {
        value = 0;
    }
    if(value > totalTime)
    {
        value = totalTime;
    }
    curSec = value;
    ui->progress_bar->setValue(value);
    player->isJump = true;
    player->jumpSec = value;
}


void MainWindow::on_last_btn_clicked()
{
    int value = curSec - JUMP_TIME;
    if(value < 0)
    {
        value = 0;
    }
    if(value > totalTime)
    {
        value = totalTime;
    }
    curSec = value;
    ui->progress_bar->setValue(value);
    player->isJump = true;
    player->jumpSec = value;
}

//暂停
void MainWindow::on_stop_btn_clicked()
{
    player->isPause = true;
}

//继续
void MainWindow::on_start_btn_clicked()
{
    player->isPause = false;
}

