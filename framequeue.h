#ifndef FRAMEQUEUE_H
#define FRAMEQUEUE_H

#include <queue>
#include  <mutex>
#include <QDebug>
#include <condition_variable>
extern "C"
{
#include "SDL3/SDL.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
#include "libavfilter/avfilter.h"
}


class FrameQueue
{
public:
    FrameQueue();
    std::queue<AVFrame *> frameQueue;
    int maxNum;
    std::mutex mtx;
    bool isStop;
    std::condition_variable cv;

    void pushFrame(AVFrame * frame);
    AVFrame* popFrame();
    //清理队列
    void cleanQueue();
};

#endif // FRAMEQUEUE_H
