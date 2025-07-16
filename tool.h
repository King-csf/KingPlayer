#ifndef TOOL_H
#define TOOL_H

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

class Tool
{
public:
    Tool();
    SDL_AudioFormat ToSDLFormat(enum AVSampleFormat format);
};

#endif // TOOL_H
