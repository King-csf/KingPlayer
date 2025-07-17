#include "framequeue.h"

FrameQueue::FrameQueue()
{

    maxNum = 100;
}

void FrameQueue::cleanQueue()
{
    std::unique_lock<std::mutex> lock(mtx);

    while(!frameQueue.empty())
    {
        AVFrame* frame = frameQueue.front();
        frameQueue.pop();
        av_frame_free(&frame);
    }

}

void FrameQueue::pushFrame(AVFrame * frame)
{
    std::unique_lock<std::mutex> lock(mtx);

    cv.wait(lock,[&]{
        return frameQueue.size() < maxNum || isStop;
    });
    if (isStop)
    {
        qDebug() << "pushFrame stop";
        av_frame_free(&frame);
        return;
    }

    frameQueue.push(frame);
    cv.notify_one();

}
AVFrame* FrameQueue::popFrame()
{
    std::unique_lock<std::mutex> lock(mtx);

    cv.wait(lock,[&]{
        return !frameQueue.empty() || isStop;
    });
    if(isStop)
    {
        qDebug() << "popFrame stop";
        return NULL;
    }
    if(isStop && frameQueue.empty())
    {
        return NULL;
    }
    AVFrame* frame = frameQueue.front();
    frameQueue.pop();
    cv.notify_one();
    return frame;
}
