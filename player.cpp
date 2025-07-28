#include "player.h"


Player::Player()
{
    isStop = false;
    inCtx = nullptr;
    swrCtx = nullptr;
    graph = nullptr;
    speedFilter = nullptr;
    srcBuffer = nullptr;
    sinkBuffer = nullptr;
    viCodeCtx = nullptr;
    auCodeCtx = nullptr;
    vIdx = -1;
    aIdx = -1;
    speed = 1;
    isDemuxer = false;
    isAuDecode = false;
    isViDecode = false;
    isModSpeed = false;
    isJumpInitFilter= false;
    isJump = false;
    isJumpAuDecode = false;
    isJumpViDecode = false;
    isPause = false;
    jumpSec = 0;
    audioClock = 0;
    videoClock = 0;
    currentAudioPts = 0;
    stream = nullptr;
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

    sar = inCtx->streams[vIdx]->sample_aspect_ratio;

    AVPacket pkt;
    int count = 0;

    //获取视频时长
    totalTime = inCtx->duration / AV_TIME_BASE;
    // qDebug() << "totalTime:" << totalTime
    //          << inCtx->duration;
    isDemuxer = true;

    while(!isStop)
    {
        av_read_frame(inCtx,&pkt);
        if(ret < 0 && ret != AVERROR_EOF)
        {
            break;
        }

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
        if(isJump)
        {
            ret = av_seek_frame(inCtx,-1,jumpSec * AV_TIME_BASE,AVSEEK_FLAG_BACKWARD);
            if(ret < 0)
            {
                qDebug() << "jump failed.";
                break;
            }

            audioPkt.cleanQueue();
            videoPkt.cleanQueue();
            isJump = false;
            isJumpAuDecode = true;
            isJumpViDecode = true;
            isJumpInitFilter = true;
        }
        while(isPause);

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

    if(vIdx == -1)
    {
        return 0;
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
    while(!isStop)
    {
        if(isStop)
        {
            break;
        }


        if(!videoPkt.popPkt(pkt) && isStop)
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

        if(isJumpViDecode)
        {
            videoFrame.cleanQueue();
            avcodec_flush_buffers(viCodeCtx);
            isJumpViDecode = false;
        }
        while(isPause);

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

    if(aIdx == -1)
    {
        return 0;
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
     // SwrContext *swrCtx = NULL;

     // swr_alloc_set_opts2(&swrCtx,
     //                     &auCodeCtx->ch_layout,AV_SAMPLE_FMT_FLT,auCodeCtx->sample_rate,
     //                     &auCodeCtx->ch_layout,auCodeCtx->sample_fmt, auCodeCtx->sample_rate,
     //                     0,NULL);

     // if(swr_init(swrCtx) < 0)
     // {
     //     qDebug() << "SwrContext init failed.";
     //     return -1;
     // }

    int count = 0;


    isAuDecode = true;
    while(!isStop)
    {
        if(isStop)
        {
            break;
        }

        //一直让音频解码线程循环

        if(!audioPkt.popPkt(pkt) && isStop)
        {
            break;
        }
        ret = avcodec_send_packet(auCodeCtx,&pkt);
        if(ret < 0)
        {
            qDebug() << "send audio packet to codec failed.";
            av_packet_unref(&pkt);
            continue;
            //break;
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
        if(isJumpAuDecode)
        {
            audioFrame.cleanQueue();
             avcodec_flush_buffers(auCodeCtx);
            isJumpAuDecode = false;
        }
        while(isPause);
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

    if(vIdx == -1)
    {
        return ;
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


    while(!isStop)
    {
        while(isPause);
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

        // 告诉 SDL：我们的“逻辑坐标系”是 videoW x videoH
        SDL_SetRenderLogicalPresentation(render, viCodeCtx->width, viCodeCtx->height,SDL_LOGICAL_PRESENTATION_LETTERBOX);
        qDebug() << "宽高比："  << sar.den << ":" << sar.num;
        SDL_RenderClear(render);
        SDL_RenderTexture(render,texture,NULL,NULL);
        SDL_RenderPresent(render);

        double timeBase = av_q2d(inCtx->streams[vIdx]->time_base);
        double duration = frame->duration * timeBase * 1000;
        // 根据播放速度调整帧间延迟
        double delay = duration / speed;
        // 视频时钟计算，保持原始时间轴
        videoClock = frame->pts * timeBase * 1000;
        double diff = videoClock-audioClock;
        //qDebug() <<  ;
        if(fabs(diff) <= delay)
        {
            delay = duration / speed;
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
        qDebug() << "Speed:" << speed << "VideoClock:" << videoClock
                 << "AudioClock:" << audioClock << "Diff:" << (videoClock - audioClock)
                 << "Delay:" << delay;

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

    if(aIdx == -1)
    {
        return;
    }

    qDebug() << "run playAudio";

    SDL_AudioSpec wantSpec;
    SDL_zero(wantSpec);

    wantSpec.freq     = inCtx->streams[aIdx]->codecpar->sample_rate;
    wantSpec.format   = tool.ToSDLFormat(AV_SAMPLE_FMT_FLT);  // 32-bit float 示例
    wantSpec.channels = inCtx->streams[aIdx]->codecpar->ch_layout.nb_channels;

    stream = SDL_OpenAudioDeviceStream(
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
    AVFrame * frame = nullptr;

    int maxSize = 4096;

    swr_alloc_set_opts2(&swrCtx,
                        &auCodeCtx->ch_layout,AV_SAMPLE_FMT_FLT,auCodeCtx->sample_rate,
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

    initSpeedFilter();

    AVFrame * endFrame = av_frame_alloc();
    speedFrame = av_frame_alloc();

    while(!isStop)
    {
        while(isPause);
        if(isStop)
        {
            break;
        }

        //倍速改变
        if(isModSpeed)
        {
           //重置过滤器
            initSpeedFilter();
            SDL_ClearAudioStream(stream);
            if (!audioFrame.frameQueue.empty())
            {
                // 倍速改变时重置音频时钟，需要与视频时钟保持一致
                double basePts = audioFrame.frameQueue.front()->pts * av_q2d(inCtx->streams[aIdx]->time_base) * 1000;
                audioClock = basePts;
                // 同时重置currentAudioPts
                currentAudioPts = audioFrame.frameQueue.front()->pts;
            }
            else
            {
                // 如果音频队列为空，使用视频时钟作为参考
                audioClock = videoClock;
                currentAudioPts = 0;
            }
            isModSpeed = false;
        }
        if(isJumpInitFilter)
        {
            initSpeedFilter();
            isJumpInitFilter = false;
        }
        if((ret = av_buffersink_get_frame(sinkBuffer,endFrame)) >= 0)
        {
            while (SDL_GetAudioStreamQueued(stream) > maxSize * 2)
            {
                SDL_Delay(1);
            }

            int bytes = ret * wantSpec.channels * sizeof(float);

            // 每秒播放的字节数
            // 使用输入帧的PTS计算音频时钟，避免atempo滤镜对PTS的影响
            if(currentAudioPts > 0)
            {
                audioClock = currentAudioPts * av_q2d(inCtx->streams[aIdx]->time_base) * 1000;
            }

            SDL_PutAudioStreamData(stream,endFrame->data[0],endFrame->linesize[0]);

            av_frame_unref(endFrame);
        }
        else
        {
            if(ret == AVERROR(EAGAIN))
            {


                speedFrame->format = AV_SAMPLE_FMT_FLT;
                //speedFrame->sample_rate = auCodeCtx->sample_rate * speed;
                av_channel_layout_copy(&speedFrame->ch_layout, &auCodeCtx->ch_layout);
                speedFrame->sample_rate = auCodeCtx->sample_rate;

                frame = audioFrame.popFrame();


                if(frame)
                {
                    speedFrame->pts = frame->pts;
                    speedFrame->nb_samples = 1024;
                    // 保存当前输入帧的PTS用于时钟计算
                    currentAudioPts = frame->pts;
                }
                else
                {
                    speedFrame->pts = 0;
                    speedFrame->nb_samples =0;
                    currentAudioPts = 0;
                }

                av_frame_get_buffer(speedFrame,0);
                //qDebug() << "frame->nb_samples: " << frame->nb_samples;
                ret = swr_convert(swrCtx,
                                  &speedFrame->data[0],speedFrame->nb_samples,
                                  (const uint8_t**)frame->data,frame->nb_samples);
                if(ret < 0)
                {
                    qDebug() << "swr_convert failed.";
                }

                speedFrame->nb_samples = ret;

                av_buffersrc_add_frame_flags(srcBuffer,speedFrame,AV_BUFFERSRC_FLAG_KEEP_REF);

                av_frame_free(&frame);
                av_frame_unref(speedFrame);
            }

        }



    }

    av_frame_free(&frame);
    av_frame_free(&speedFrame);
    av_frame_free(&endFrame);
    //关闭设备
    if(stream)
    {
        SDL_PauseAudioStreamDevice(stream);
        SDL_DestroyAudioStream(stream);
        stream = nullptr;
    }

}

bool Player::initSpeedFilter()
{
    int ret = 0;
    if(srcBuffer)
    {
        avfilter_free(srcBuffer);
        srcBuffer = nullptr;
    }
    if(sinkBuffer)
    {
        avfilter_free(sinkBuffer);
        sinkBuffer = nullptr;
    }
    if(speedFilter)
    {
        avfilter_free(speedFilter);
        speedFilter = nullptr;
    }
    if(graph)
    {
        avfilter_graph_free(&graph);
        graph = nullptr;
    }

    graph = avfilter_graph_alloc();
    if(!graph)
    {
        qDebug() << "speedFilter graph alloc failed.";
        return false;
    }
    char buf[20];
    av_channel_layout_describe(&auCodeCtx->ch_layout,
                               buf, 20);
    uint64_t mask = auCodeCtx->ch_layout.u.mask;
    qDebug() << "mask:"<< mask;
    QString arg1 = QString("time_base=%1/%2"
                          ":sample_rate=%3"
                          ":sample_fmt=flt"
                          ":channel_layout=0x%4").arg(1).arg(inCtx->streams[aIdx]->time_base.den)
                       .arg(inCtx->streams[aIdx]->codecpar->sample_rate)
                       .arg(mask, 0, 16);
    qDebug() << "arg1:" << arg1;
    ret = avfilter_graph_create_filter(&srcBuffer,avfilter_get_by_name("abuffer"),
                                        "in",arg1.toStdString().c_str(),nullptr,graph);
    if(ret < 0)
    {
        qDebug() <<"srcBuffer filter alloc failed.";
        return false;
    }
    ret = avfilter_graph_create_filter(&sinkBuffer,avfilter_get_by_name("abuffersink"),
                                       "in",nullptr,nullptr,graph);
    if(ret < 0)
    {
        qDebug() << "sinkBuffer filter alloc failed";
        return false;
    }
    QString tempo = QString("tempo=%1").arg(speed);
    ret = avfilter_graph_create_filter(&speedFilter,avfilter_get_by_name("atempo"),
                                               "tempo",tempo.toStdString().c_str(),nullptr,graph);
    if(ret < 0)
    {
        qDebug() << "speedFilter alloc failed";
        return false;
    }

    avfilter_link(srcBuffer, 0, speedFilter, 0);
    avfilter_link(speedFilter, 0, sinkBuffer, 0);

    ret = avfilter_graph_config(graph,nullptr);
    if(ret < 0)
    {
        qDebug() << "avfilter graph config failed.";
        return false;
    }

    return true;
}

AVFrame * Player::useSpeedFilter(AVFrame * frame)
{
    if (frame->format != AV_SAMPLE_FMT_FLT) {
        qDebug() << "滤镜格式不匹配：预期" << AV_SAMPLE_FMT_FLT << "实际" << frame->format;
        return nullptr;
    }
    if (frame->sample_rate != auCodeCtx->sample_rate) {
        qDebug() << "采样率不匹配：预期" << auCodeCtx->sample_rate << "实际" << frame->sample_rate;
        return nullptr;
    }
    if (!av_channel_layout_compare(&frame->ch_layout, &auCodeCtx->ch_layout)) {
        qDebug() << "声道布局不匹配";

        return nullptr;
    }
    int ret = 0;
    ret = av_buffersrc_add_frame_flags(srcBuffer,frame,AV_BUFFERSRC_FLAG_KEEP_REF);
    if(ret < 0)
    {
        char errbuf[128];
        av_strerror(ret, errbuf, sizeof(errbuf));
        qDebug() << "failed to add frame to buffersrc: " << errbuf;
        return nullptr;
    }
    AVFrame * resFrame = av_frame_alloc();
    av_frame_get_buffer(resFrame,0);

    while(true)
    {
        ret = av_buffersink_get_frame_flags(sinkBuffer,resFrame,AV_BUFFERSRC_FLAG_KEEP_REF);
        if(ret < 0 && ret != AVERROR(EAGAIN))
        {
            break;
        }
    }
    return resFrame;
}

void Player::destory()
{
    qDebug() << "run destory";

    if(viCodeCtx)
    {
        qDebug() << "***********";
        avcodec_free_context(&viCodeCtx);
        viCodeCtx = nullptr;
    }
    qDebug() << "del viCodeCtx";
    if(auCodeCtx)
    {
        avcodec_free_context(&auCodeCtx);
        auCodeCtx = nullptr;
    }
    qDebug() << "del auCodeCtx";
    if(inCtx)
    {
        avformat_close_input(&inCtx);
    }
    qDebug() << "del inCtx";
    if(srcBuffer)
    {
        avfilter_free(srcBuffer);
        srcBuffer = nullptr;
    }
    qDebug() << "del srcBuffer";
    if(sinkBuffer)
    {
        avfilter_free(sinkBuffer);
        sinkBuffer = nullptr;
    }
    qDebug() << "del sinkBuffer";
    if(speedFilter)
    {
        avfilter_free(speedFilter);
        speedFilter = nullptr;
    }
    qDebug() << "del speedFilter";
    if(graph)
    {
        avfilter_graph_free(&graph);
        graph = nullptr;
    }
    qDebug() << "del graph";
    vIdx = -1;
    aIdx = -1;
    isDemuxer = false;
    isAuDecode = false;
    isViDecode = false;
    isJump = false;
    jumpSec = 0;
    isJumpAuDecode = false;
    isJumpViDecode = false;
    audioClock = 0;
    videoClock = 0;
    currentAudioPts = 0;
    isModSpeed = false;
    isPause = false;
    isJumpInitFilter = false;
    //totalTime = 0;

    if(texture)
    {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
    qDebug() << "del texture";
    if(swrCtx)
    {
        swr_free(&swrCtx);
        swrCtx = nullptr;
    }
    qDebug() << "del swrCtx";
    if(stream)
    {
        SDL_DestroyAudioStream(stream);
        stream = nullptr;
    }
    qDebug() << "del stream";
    audioPkt.cleanQueue();
    videoPkt.cleanQueue();
    audioFrame.cleanQueue();
    videoFrame.cleanQueue();
}
