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
    // 延迟执行，确保 Qt 完成创建
    //QTimer::singleShot(0, this, &MainWindow::setSDLWindow);
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
    muxerThread = std::thread(&player->threadFun,Player::THREAD_DEMUXER);
    //;muxerThread.join();
    viDecoderThread = std::thread(&player->threadFun,Player::THREAD_VIDEO_DECODE);
    auDecoderThread = std::thread(&player->threadFun,Player::THREAD_AUDIO_DECODE);
    delayVideoThread = std::thread(&player->threadFun,Player::THREAD_DELAY_VIDEO);
    playAudioThread = std::thread(&player->threadFun,Player::THREAD_PLAY_AUDIO);
    //viDeocderThread.detach();
    initProgressBar();

    //ui->total_time->setText()
}

void MainWindow::initProgressBar()
{
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

void MainWindow::stopThread()
{
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
    }
    if(viDecoderThread.joinable())
    {
        viDecoderThread.join();
    }
    if(auDecoderThread.joinable())
    {
        auDecoderThread.join();
    }
    if(delayVideoThread.joinable())
    {
        delayVideoThread.join();
    }
    if(playAudioThread.joinable())
    {
        playAudioThread.join();
    }

    player->isStop = false;
    player->audioPkt.isStop = false;
    player->videoPkt.isStop = false;
    player->videoFrame.isStop = false;
    player->audioFrame.isStop = false;

}
