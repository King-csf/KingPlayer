#ifndef PLAYER_H
#define PLAYER_H

#include "packetqueue.h"
#include "framequeue.h"
#include <QString>
#include <QDebug>
#include <atomic>
#include <chrono>
#include "tool.h"
extern "C"
{
#include "SDL3/SDL.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
#include "libavutil/error.h"
#include "libavfilter/avfilter.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
}


class Player
{
public:
    Player();
    ~Player();

    Tool tool;
    //播放的文件
    QString filename;

    SDL_Window * sdlWindow; //窗口
    SDL_Renderer * render;  //渲染器
    SDL_Texture * texture;  //纹理

    PacketQueue audioPkt;   //音频pkt队列
    PacketQueue videoPkt;   //视频pkt队列
    FrameQueue  audioFrame;
    FrameQueue  videoFrame;
    //输入上下文
    AVFormatContext * inCtx;

    int vIdx;
    int aIdx;

    //线程是否停止
    std::atomic<bool> isStop;
    //相关操作是否结束
    std::atomic<bool> isDemuxer;
    std::atomic<bool> isAuDecode;
    std::atomic<bool> isViDecode;

    double totalTime;

    //音频解码
    //const AVCodec * auCodec;
    AVCodecContext *auCodeCtx;
    //视频解码
    //const AVCodec * viCodec;
    AVCodecContext *viCodeCtx;

    enum threadType{
        THREAD_DEMUXER = 0,
        THREAD_AUDIO_DECODE,
        THREAD_VIDEO_DECODE,
        THREAD_DELAY_VIDEO,
        THREAD_PLAY_AUDIO
    };

    //获取单例
    static Player* getInstance();
    //线程运行函数
    static void threadFun(threadType flag);
    //解封装
    int demuxer(const QString& filename);
    int videoDeocode();
    int audioDecode();
    void delayVideo();
    void playAudio();
    void destory();

    double audioClock;
    double videoClock;

    float speed; //倍速
    std::atomic<bool> isModSpeed;
    SwrContext *swrCtx;
    AVFrame * speedFrame;

    //是否快进
    std::atomic<bool> isJump;
    //跳转到的时间
    int jumpSec;
    std::atomic<bool> isJumpAuDecode;
    std::atomic<bool> isJumpViDecode;
    std::atomic<bool> isPause;
};

#endif // PLAYER_H
