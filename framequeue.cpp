#include "framequeue.h"

FrameQueue::FrameQueue()
{

    maxNum = INT_MAX;
}

void FrameQueue::cleanQueue()
{
    std::unique_lock<std::mutex> lock(mtx);

    while(!frameQueue.empty())
    {
        av_frame_unref(frameQueue.front());
        av_frame_free(&frameQueue.front());
        frameQueue.pop();
    }

}

void FrameQueue::pushFrame(AVFrame * frame)
{
    std::unique_lock<std::mutex> lock(mtx);

    cv.wait(lock,[&]{
        return frameQueue.size() < maxNum;
    });
    AVFrame * temp = av_frame_alloc();
    av_frame_get_buffer(temp,0);
    av_frame_move_ref(temp,frame);
    frameQueue.push(temp);
    cv.notify_one();

}
bool FrameQueue::popFrame(AVFrame * frame)
{
    std::unique_lock<std::mutex> lock(mtx);

    cv.wait(lock,[&]{
        return !frameQueue.empty() || isStop;
    });
    if(isStop && frameQueue.empty())
    {
        return false;
    }
    av_frame_move_ref(frame,frameQueue.front());
    frameQueue.pop();
    cv.notify_one();
    return true;
}
