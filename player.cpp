#include "player.h"


Player::Player()
{
    isStop = false;
    inCtx = nullptr;
    swrCtx = nullptr;
    //viCodeCtx = nullptr;
    //auCodeCtx = nullptr;
    vIdx = -1;
    aIdx = -1;
    speed = 1;
    isDemuxer = false;
    isAuDecode = false;
    isViDecode = false;
    isModSpeed = false;
    audioClock = 0;
    videoClock = 0;
    totalTime = 0;
    av_log_set_level(AV_LOG_WARNING);

}

Player::~Player()
{
    destory();
}

int Player::demuxer(const QString& filename)
{
    qDebug() << "run demux";
    int ret;
    //打开输入
    ret = avformat_open_input(&inCtx,filename.toStdString().c_str(),NULL,NULL);
    if(ret < 0)
    {
        char  errBuf[128];
        av_strerror(ret,errBuf,128);
        av_log(NULL,AV_LOG_ERROR,"input open failed:%s.\n",errBuf);
        return -1;
    }

    ret = avformat_find_stream_info(inCtx,NULL);
    if(ret < 0)
    {
        char  errBuf[128];
        av_strerror(ret,errBuf,128);
        av_log(NULL,AV_LOG_ERROR,"Find stream info failed:%s.\n",errBuf);
        return -1;
    }

    vIdx = av_find_best_stream(inCtx,AVMEDIA_TYPE_VIDEO,-1,-1,NULL,0);
    aIdx = av_find_best_stream(inCtx,AVMEDIA_TYPE_AUDIO,-1,-1,NULL,0);

    AVPacket pkt;
    int count = 0;

    //获取视频时长
    totalTime = inCtx->duration / AV_TIME_BASE;
    // qDebug() << "totalTime:" << totalTime
    //          << inCtx->duration;
    isDemuxer = true;

    while((ret = av_read_frame(inCtx,&pkt)) == 0)
    {
        if(isStop)
        {
            break;
        }
        //av_log(NULL,AV_LOG_DEBUG,"处理%d帧.\n",++count);
        //qDebug() << getpid() << " 处理" <<++count<< "帧" ;
        if(pkt.stream_index == vIdx)
        {

            videoPkt.pushPkt(pkt);
        }
        else if(pkt.stream_index == aIdx)
        {
            audioPkt.pushPkt(pkt);
        }
        if(isStop)
        {
            return 0;
        }
        //av_packet_unref(&pkt);
    }

    qDebug()<< "视频帧数：" << videoPkt.pktQueue.size() << Qt::endl
             <<"音频帧数："<< audioPkt.pktQueue.size();
    return 0;

}

Player* Player::getInstance()
{
    static Player * player = new Player();
    return player;
}

void Player::threadFun(threadType flag)
{
    Player * player = getInstance();
    switch (flag)
    {
    case THREAD_DEMUXER:
        player->demuxer(player->filename);
        break;
    case THREAD_VIDEO_DECODE:
        player->videoDeocode();
        break;
    case THREAD_AUDIO_DECODE:
        player->audioDecode();
        break;
    case THREAD_DELAY_VIDEO:
        player->delayVideo();
        break;
    case THREAD_PLAY_AUDIO:
        player->playAudio();
        break;
    default:
        break;
    }
}

int Player::videoDeocode()
{
    while(!isDemuxer && !isStop)
    {
        SDL_Delay(5);
    }

    int ret = 0;
    qDebug() << "run videoDecode";
    const AVCodec * codec = avcodec_find_decoder(inCtx->streams[vIdx]->codecpar->codec_id);
    if(!codec)
    {
        qDebug() << "video codec find failed";
        return -1;
    }

    viCodeCtx = avcodec_alloc_context3(codec);
    if(!viCodeCtx)
    {
        qDebug() << "video codecCtx alloc failed";
        return -1;
    }
    avcodec_parameters_to_context(viCodeCtx,inCtx->streams[vIdx]->codecpar);
    ret = avcodec_open2(viCodeCtx, codec, NULL);
    if(ret < 0)
    {
        qDebug() << "video codec open failed.";
        return -1;
    }

    // 解码得到 frame，format == YUV420P
    // SwsContext* sws = sws_getContext(
    //     viCodeCtx->width, viCodeCtx->height, viCodeCtx->pix_fmt,
    //     viCodeCtx->width, viCodeCtx->height, AV_PIX_FMT_YUV420P,
    //     SWS_BILINEAR, nullptr, nullptr, nullptr);

    qDebug() << "format : " << viCodeCtx->pix_fmt ;
    AVPacket pkt;
    //av_init_packet(&pkt);
    AVFrame * frame = av_frame_alloc();
    av_frame_get_buffer(frame,0);

    // AVFrame * dstFrame= av_frame_alloc();
    // dstFrame->width = viCodeCtx->width;
    // dstFrame->height = viCodeCtx->height;
    // dstFrame->format = AV_PIX_FMT_YUV420P;
    // ret = av_frame_get_buffer(dstFrame,1);
    // if(ret < 0)
    // {
    //     char errBuf[128];
    //     av_strerror(ret,errBuf,128);
    //     qDebug() << "sws dst frame buffer get failed :"
    //              << QString(errBuf);
    // }

    isViDecode = true;

    int count = 0;
    while(true)
    {
        if(isStop)
        {
            break;
        }


        if(!videoPkt.popPkt(pkt))
        {
            break;
        }
        ret = avcodec_send_packet(viCodeCtx,&pkt);
        av_packet_unref(&pkt);
        if(ret < 0)
        {
            qDebug() << "send video packet to codec failed.";
            break;
        }
        count++;
        if(count % 1000 == 0)
        {
            //qDebug() << " 解码视频：" << count << "帧";
        }

        //qDebug() << getpid() << " 解码视频：" << count << "帧";
        while((ret = avcodec_receive_frame(viCodeCtx,frame)) == 0)
        {
            //av_frame_make_writable(dstFrame);

            // sws_scale(sws, frame->data, frame->linesize, 0, frame->height,
            //          dstFrame->data, dstFrame->linesize);
            //qDebug() << getpid() << " 解码视频：" << ++count << "帧";
            //qDebug() << frame->pts;
            if(isStop)
            {
                av_frame_unref(frame);
                break;
            }
            AVFrame * cloneFrame = av_frame_clone(frame);
            videoFrame.pushFrame(cloneFrame);
            av_frame_unref(frame);

            //videoFrame.pushFrame(dstFrame);

        }
        if(ret < 0 && ret != AVERROR_EOF && ret != AVERROR(EAGAIN))
        {
            qDebug() << "receive video packet from codec failed.";
            break;
        }

    }

    avcodec_send_packet(viCodeCtx,NULL);
    while(avcodec_receive_frame(viCodeCtx,frame) == 0)
    {
        //qDebug()  << " 解码视频：" << ++count << "帧";
        // sws_scale(sws, frame->data, frame->linesize, 0, frame->height,
        //           dstFrame->data, dstFrame->linesize);
        // videoFrame.pushFrame(dstFrame);
        //qDebug() << frame->pts;
        if(isStop)
        {
            av_frame_unref(frame);
            return 0;
        }
        AVFrame * cloneFrame = av_frame_clone(frame);
        videoFrame.pushFrame(cloneFrame);
        av_frame_unref(frame);
    }

    qDebug() << "视频解码完成";
    av_frame_free(&frame);
    //sws_freeContext(sws);

    return 0;
}

int Player::audioDecode()
{

    while(!isDemuxer && !isStop)
    {
        SDL_Delay(5);
    }

    int ret = 0;
    qDebug() << "run audioDecode";
    const AVCodec * codec = avcodec_find_decoder(inCtx->streams[aIdx]->codecpar->codec_id);
    if(!codec)
    {
        qDebug() << "audio codec find failed";
        return -1;
    }

    auCodeCtx = avcodec_alloc_context3(codec);
    if(!auCodeCtx)
    {
        qDebug() << "audio codecCtx alloc failed";
        return -1;
    }
    avcodec_parameters_to_context(auCodeCtx,inCtx->streams[aIdx]->codecpar);
    ret = avcodec_open2(auCodeCtx, codec, NULL);
    if(ret < 0)
    {
        qDebug() << "audio codec open failed.";
        return -1;
    }

    AVPacket pkt;
    //av_init_packet(&pkt);
    AVFrame * frame = av_frame_alloc();
    frame->format = auCodeCtx->sample_fmt;
    frame->ch_layout = auCodeCtx->ch_layout;
    frame->nb_samples = 1024;
    av_frame_get_buffer(frame,0);



    //
     SwrContext *swrCtx = NULL;

     swr_alloc_set_opts2(&swrCtx,
                         &auCodeCtx->ch_layout,AV_SAMPLE_FMT_FLT,auCodeCtx->sample_rate,
                         &auCodeCtx->ch_layout,auCodeCtx->sample_fmt, auCodeCtx->sample_rate,
                         0,NULL);

     if(swr_init(swrCtx) < 0)
     {
         qDebug() << "SwrContext init failed.";
         return -1;
     }

    int count = 0;


    isAuDecode = true;
    while(true)
    {
        if(isStop)
        {
            break;
        }

        //解封装完成，并且队列为空退出循环
        if(isDemuxer && audioPkt.pktQueue.empty())
        {
            break;
        }

        if(!audioPkt.popPkt(pkt))
        {
            break;
        }
        ret = avcodec_send_packet(auCodeCtx,&pkt);
        if(ret < 0)
        {
            qDebug() << "send audio packet to codec failed.";
            break;
        }
        count++;
        if(count % 500 == 0)
        {
            //qDebug() << getpid() << " 解码音频：" << count << "帧";
        }


        while((ret = avcodec_receive_frame(auCodeCtx,frame)) == 0)
        {
            // AVFrame * outFrame = av_frame_alloc();
            // outFrame->format = AV_SAMPLE_FMT_FLT;
            // outFrame->ch_layout = auCodeCtx->ch_layout;
            // outFrame->nb_samples = 1024;
            // av_frame_get_buffer(outFrame,0);
            // //qDebug() << getpid() << " 解码音频：" << ++count << "帧";
            // ret = swr_convert(swrCtx,
            //             &outFrame->data[0],outFrame->nb_samples,
            //             (const uint8_t**)frame->data,frame->nb_samples);
            //qDebug() << frame->pts << "  "<< outFrame->pts;

            // if(ret < 0)
            // {
            //     qDebug() << "swr_convert failed.";
            //     continue;
            // }
            //outFrame->pts = frame->pts;
            //qDebug() << outFrame->data[0];
            if(isStop)
            {
                av_frame_unref(frame);
                break;
            }
            AVFrame * cloneFrame = av_frame_clone(frame);
            audioFrame.pushFrame(cloneFrame);
            av_frame_unref(frame);

        }
        if(ret < 0 && ret != AVERROR_EOF && ret != AVERROR(EAGAIN))
        {
            qDebug() << "receive audio packet from codec failed.";
            break;
        }
        av_packet_unref(&pkt);
    }

    avcodec_send_packet(auCodeCtx,NULL);
    while((ret = avcodec_receive_frame(auCodeCtx,frame)) == 0)
    {
        // AVFrame * outFrame = av_frame_alloc();
        // outFrame->format = AV_SAMPLE_FMT_FLT;
        // outFrame->ch_layout = auCodeCtx->ch_layout;
        // outFrame->nb_samples = 1024;
        // av_frame_get_buffer(outFrame,0);
        // ret = swr_convert(swrCtx,
        //                   outFrame->data,outFrame->nb_samples,
        //                   frame->data,frame->nb_samples);
        //qDebug() << getpid() << " 解码音频：" << ++count << "帧";
        if(isStop)
        {
            av_frame_unref(frame);
            break;
        }
        AVFrame * cloneFrame = av_frame_clone(frame);
        audioFrame.pushFrame(cloneFrame);
        av_frame_unref(frame);
        //av_frame_unref(frame);
    }
    qDebug() << "音频解码完成";
    av_frame_free(&frame);
    swr_free(&swrCtx);
    return 0;
}

void Player::delayVideo()
{

    while ((!isDemuxer || !isViDecode) && !isStop)
    {
        SDL_Delay(5);
    }

    // 跳出循环后，必须检查是不是因为 isStop 才跳出的
    if (isStop)
    {
        qDebug() << "delayVideo thread stopped before init.";
        return; // 如果是，必须立刻返回，不能执行后面的代码
    }

    qDebug() << "run delayVideo";

    texture = SDL_CreateTexture(render,
                                SDL_PIXELFORMAT_IYUV,
                                SDL_TEXTUREACCESS_STREAMING,
                                viCodeCtx->width,
                                viCodeCtx->height);
    if (!texture)
    {
        qDebug() << "SDL_CreateTexture failed:" << SDL_GetError();
        return;
    }
    AVFrame *frame = av_frame_alloc();
    av_frame_get_buffer(frame,0);


    while(true)
    {
        if(isStop)
        {
            break;
        }

        frame = videoFrame.popFrame();
        if(!frame)
        {
            break;
        }

        SDL_UpdateYUVTexture(texture,NULL,
                             (Uint8 *)frame->data[0],frame->linesize[0],
                             (Uint8 *)frame->data[1],frame->linesize[1],
                             (Uint8 *)frame->data[2],frame->linesize[2]);
        // qDebug() << frame->data[0]<< " " << frame->linesize[0] << " "
        //          << frame->data[1]<< " " << frame->linesize[1] << " "
        //          << frame->data[2] << " "<< frame->linesize[3];

        SDL_RenderClear(render);
        SDL_RenderTexture(render,texture,NULL,NULL);
        SDL_RenderPresent(render);

        double timeBase = av_q2d(inCtx->streams[vIdx]->time_base);
        double duration = frame->duration * timeBase * 1000;
        double delay = duration;
        videoClock = frame->pts * timeBase * 1000;
        double diff = videoClock-audioClock;
        //qDebug() <<  ;
        if(fabs(diff) <= delay)
        {
            delay = duration;
        }
        else if(diff > duration)
        {
            delay *= 2;
        }
        else if(diff < -duration)
        {
            delay = 0;
        }

        av_frame_unref(frame);

        SDL_Delay(delay);
    }
    av_frame_free(&frame);
}

void Player::playAudio()
{
    isModSpeed = false;
    while ((!isDemuxer || !isAuDecode) && !isStop)
    {
        SDL_Delay(5);
    }

    if (isStop)
    {
        qDebug() << "play thread stopped before init.";
        return;
    }

    qDebug() << "run playAudio";

    SDL_AudioSpec wantSpec;
    SDL_zero(wantSpec);

    wantSpec.freq     = inCtx->streams[aIdx]->codecpar->sample_rate;
    wantSpec.format   = tool.ToSDLFormat(AV_SAMPLE_FMT_FLT);  // 32-bit float 示例
    wantSpec.channels = inCtx->streams[aIdx]->codecpar->ch_layout.nb_channels;

    SDL_AudioStream* stream = SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
        &wantSpec,
        nullptr,
        nullptr
        );

    if(!stream)
    {
        qDebug() << "SDL_OpenAudioDeviceStream failed.";
        return ;
    }


    int count = 0;
    //开始播放音频

    SDL_ResumeAudioStreamDevice(stream);
    AVFrame * frame = av_frame_alloc();
    av_frame_get_buffer(frame,0);
    int maxSize = 4096;

    swr_alloc_set_opts2(&swrCtx,
                        &auCodeCtx->ch_layout,AV_SAMPLE_FMT_FLT,auCodeCtx->sample_rate * speed,
                        &auCodeCtx->ch_layout,auCodeCtx->sample_fmt, auCodeCtx->sample_rate,
                        0,NULL);

    int ret = swr_init(swrCtx);
    if(ret < 0)
    {
        qDebug() << "playAudio swrCtx init failed.";
        return;
    }


    if(ret < 0)
    {
        qDebug() << "123";
    }

    while(true)
    {
        if(isStop)
        {
            break;
        }

        //倍速改变
        if(isModSpeed)
        {
            swr_free(&swrCtx);
            swr_alloc_set_opts2(&swrCtx,
                                &auCodeCtx->ch_layout,AV_SAMPLE_FMT_FLT,auCodeCtx->sample_rate * speed,
                                &auCodeCtx->ch_layout,auCodeCtx->sample_fmt, auCodeCtx->sample_rate,
                                0,NULL);

            swr_init(swrCtx);
            isModSpeed = false;
        }
        speedFrame = av_frame_alloc();
        speedFrame->format = AV_SAMPLE_FMT_FLT;
        //speedFrame->sample_rate = auCodeCtx->sample_rate * speed;
        speedFrame->ch_layout = auCodeCtx->ch_layout;
        speedFrame->nb_samples = 1024;

        ret = av_frame_get_buffer(speedFrame,0);

        frame = audioFrame.popFrame();
        if(!frame)
        {
            break;
        }
        //qDebug() << "frame->nb_samples: " << frame->nb_samples;
        ret = swr_convert(swrCtx,
                    &speedFrame->data[0],speedFrame->nb_samples,
                    (const uint8_t**)frame->data,frame->nb_samples);
        if(ret < 0)
        {
            qDebug() << "swr_convert failed.";
        }
        speedFrame->pts = frame->pts;
        //qDebug()  << "ret :"<< ret;

        while (SDL_GetAudioStreamQueued(stream) > maxSize * 2)
        {
            SDL_Delay(1);
        }
        int bytes = speedFrame->nb_samples * wantSpec.channels * sizeof(float);

        audioClock = speedFrame->pts * av_q2d(auCodeCtx->time_base) * 1000;
        //qDebug() << audioClock;
        SDL_PutAudioStreamData(stream,speedFrame->data[0],bytes);
        //qDebug() << "音频播放"<< ++count << "次.";

        av_frame_unref(frame);
        av_frame_unref(speedFrame);
    }

    av_frame_free(&frame);
    av_frame_free(&speedFrame);
    //关闭设备
    SDL_PauseAudioStreamDevice(stream);
}

void Player::destory()
{
    qDebug() << "run destory";

    if(viCodeCtx)
    {
        avcodec_free_context(&viCodeCtx);
        viCodeCtx = nullptr;
    }
    if(auCodeCtx)
    {
        avcodec_free_context(&auCodeCtx);
        auCodeCtx = nullptr;
    }
    if(inCtx)
    {
        avformat_close_input(&inCtx);
    }
    vIdx = -1;
    aIdx = -1;
    isDemuxer = false;
    isAuDecode = false;
    isViDecode = false;
    audioClock = 0;
    videoClock = 0;
    //totalTime = 0;

    if(texture)
    {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
    if(swrCtx)
    {
        swr_free(&swrCtx);
    }
    audioPkt.cleanQueue();
    videoPkt.cleanQueue();
    audioFrame.cleanQueue();
    videoFrame.cleanQueue();
}
