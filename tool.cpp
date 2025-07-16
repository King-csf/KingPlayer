#include "tool.h"

Tool::Tool() {}

SDL_AudioFormat Tool::ToSDLFormat(enum AVSampleFormat format) {
    switch (format)
    {
        case AV_SAMPLE_FMT_U8:
        case AV_SAMPLE_FMT_U8P:
            return SDL_AUDIO_U8;
        case AV_SAMPLE_FMT_S16:
        case AV_SAMPLE_FMT_S16P:
            return SDL_AUDIO_S16;
        case AV_SAMPLE_FMT_S32:
        case AV_SAMPLE_FMT_S32P:
            return SDL_AUDIO_S32;
        case AV_SAMPLE_FMT_FLT:
        case AV_SAMPLE_FMT_FLTP:
            return SDL_AUDIO_F32;
        // 以下是其他可能的格式，但不太常用
        // case AV_SAMPLE_FMT_DBL:
        // case AV_SAMPLE_FMT_DBLP:
        //     return SDL_AUDIO_F64; // 假设 SDL3 未来支持
        // case AV_SAMPLE_FMT_S64:
        // case AV_SAMPLE_FMT_S64P:
        //     return SDL_AUDIO_S64; // 假设 SDL3 未来支持
        default:
            return SDL_AUDIO_UNKNOWN; // 返回 0 表示不支持或未知的格式
    }
}
