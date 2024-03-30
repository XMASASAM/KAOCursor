#include <iostream>
#include <string>
#include <cstdlib>
#include"kc2.h"
#include"ffmpeg.h"
#include<mmsystem.h>
#include <libavutil/channel_layout.h>
#include<mfapi.h>
#include <mfidl.h>
#include<mfreadwrite.h>

#include<thread>
using namespace std;


bool waveout_is_close=false;

void CALLBACK waveOutProc(HWAVEOUT m_hWO, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
    if (uMsg == WOM_OPEN) {
        cout << "WaveOut_Open" << endl;
    }

    if (uMsg == WOM_DONE) {
        WAVEHDR* wh = (WAVEHDR*)dwParam1;
        if (wh->dwUser) {
            if (wh->dwBufferLength==0) {
                cout << "close_flg" << endl;
                waveout_is_close = true;
            }
            else {
                AVFrame* frame = (AVFrame*)wh->dwUser;
                av_frame_free(&frame);
            }
        }
        delete wh;
    }

    if (uMsg == WOM_CLOSE) {
        cout << "WaveOut_Close" << endl;
    }

    if (dwParam2) {
        cout << 123456 << endl;
    }

}

void CALLBACK waveOutProc2(HWAVEOUT m_hWO, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
    if (uMsg == WOM_OPEN) {
        cout << "WaveOut_Open" << endl;
    }

    if (uMsg == WOM_DONE) {
        WAVEHDR* wh = (WAVEHDR*)dwParam1;
        if (wh->dwUser) {
            if (wh->dwBufferLength == 0) {
                cout << "close_flg" << endl;
                waveout_is_close = true;
            }
            else {
                BYTE* d = (BYTE*)wh->dwUser;
                delete[] d;
            }
        }
        delete wh;
    }

    if (uMsg == WOM_CLOSE) {
        cout << "WaveOut_Close" << endl;
    }

    if (dwParam2) {
        cout << 123456 << endl;
    }

}

void init_filter_context2(AVCodecContext* dec_ctx,AVSampleFormat out_fmt ,const AVChannelLayout* out_layout,FilteringContext* filter_ctx) {
    //	if (dec_ctx->codec_type != AVMEDIA_TYPE_VIDEO)return 0;
    char args[512];
    int ret = 0;
    const AVFilter* abuffersrc = avfilter_get_by_name("abuffer");
    const AVFilter* abuffersink = avfilter_get_by_name("abuffersink");
    const AVFilter* volume = avfilter_get_by_name("volume");
    AVFilterContext* abuffersrc_ctx = NULL;
    AVFilterContext* abuffersink_ctx = NULL;
    AVFilterContext* volume_ctx = NULL;

    AVFilterInOut* outputs = avfilter_inout_alloc();
    AVFilterInOut* inputs = avfilter_inout_alloc();
    AVFilterGraph* filter_graph = avfilter_graph_alloc();
    char buf[64];
    if (dec_ctx->ch_layout.order == AV_CHANNEL_ORDER_UNSPEC)
        av_channel_layout_default(&dec_ctx->ch_layout, dec_ctx->ch_layout.nb_channels);
    av_channel_layout_describe(&dec_ctx->ch_layout, buf, sizeof(buf));
    
    snprintf(args, sizeof(args),
        "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=%s",
        dec_ctx->pkt_timebase.num, dec_ctx->pkt_timebase.den,
        dec_ctx->sample_rate,
        av_get_sample_fmt_name(dec_ctx->sample_fmt),
        buf);
    

    avfilter_graph_create_filter(&abuffersrc_ctx, abuffersrc, "in", args, NULL, filter_graph);
    
    avfilter_graph_create_filter(&volume_ctx, volume, "vo", ".5", NULL, filter_graph);

    avfilter_graph_create_filter(&abuffersink_ctx, abuffersink, "out", NULL, NULL, filter_graph);
    
    av_opt_set_bin(abuffersink_ctx, "sample_fmts", (uint8_t*)&out_fmt, sizeof(out_fmt), AV_OPT_SEARCH_CHILDREN);
    av_channel_layout_describe(out_layout, buf, sizeof(buf));
    av_opt_set(abuffersink_ctx, "ch_layouts", buf, AV_OPT_SEARCH_CHILDREN);
    av_opt_set_bin(abuffersink_ctx, "sample_rates", (uint8_t*)&dec_ctx->sample_rate, sizeof(dec_ctx->sample_rate), AV_OPT_SEARCH_CHILDREN);
    
    //av_opt_set_bin(abuffersink_ctx, "sample_fmts", (uint8_t*)&out_fmt, sizeof(out_fmt), AV_OPT_SEARCH_CHILDREN);


    outputs->name = av_strdup("in");
    outputs->filter_ctx = abuffersrc_ctx;
    outputs->pad_idx = 0;
    outputs->next = NULL;

    inputs->name = av_strdup("out");
    inputs->filter_ctx = abuffersink_ctx;
    inputs->pad_idx = 0;
    inputs->next = NULL;

    //avfilter_graph_parse_ptr(filter_graph, "anull", &inputs, &outputs, NULL);
    
    avfilter_link(abuffersrc_ctx, 0, volume_ctx, 0);
    avfilter_link(volume_ctx,0,abuffersink_ctx,0);

    avfilter_graph_config(filter_graph, NULL);

    filter_ctx->buffersrc_ctx = abuffersrc_ctx;
    filter_ctx->buffersink_ctx = abuffersink_ctx;
    filter_ctx->filter_graph = filter_graph;

    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);
}

EXPORT int test_sound2() {
    //std::string mp3FilePath = "C:\\Users\\MaMaM\\Downloads\\Morning (1).mp3";
    std::string mp3FilePath = "C:\\Users\\MaMaM\\source\\repos\\KC2\\KC2\\pc-mouse-3.mp3";

    //    std::string mp3FilePath = "C:\\Users\\MaMaM\\source\\repos\\KC2\\KC2\\output.wav";

    AVFormatContext* formatContext = nullptr;
    if (avformat_open_input(&formatContext, mp3FilePath.c_str(), nullptr, nullptr) != 0) {
        std::cerr << "Error opening input file." << std::endl;
        return 1;
    }

    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        std::cerr << "Error finding stream information." << std::endl;
        avformat_close_input(&formatContext);
        return 1;
    }

    // 最初の音声ストリームを取得
    int audioStreamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (audioStreamIndex < 0) {
        std::cerr << "Error finding audio stream." << std::endl;
        avformat_close_input(&formatContext);
        return 1;
    }

    // ストリームを開く
    auto codecpar = formatContext->streams[audioStreamIndex]->codecpar;
    const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
    if (!codec) {
        std::cerr << "Error finding codec." << std::endl;
        avformat_close_input(&formatContext);
        return 1;
    }

    //AVCodecContext* codecContext=nullptr;
    /* Allocate a new decoding context. */
    AVCodecContext* codecContext = avcodec_alloc_context3(codec);


    avcodec_parameters_to_context(codecContext, codecpar);


    if (avcodec_open2(codecContext, codec, nullptr) < 0) {
        std::cerr << "Error opening codec." << std::endl;
        avformat_close_input(&formatContext);
        return 1;
    }

    codecContext->pkt_timebase = formatContext->streams[audioStreamIndex]->time_base;



    HWAVEOUT hWaveOut;
    WAVEFORMATEX wf;
    std::cout << av_get_sample_fmt_name(codecContext->sample_fmt) << endl;;// == AVSampleFormat::
    printf("ch:%d sample_rate:%d bit_par_sample:%d blockAlign:%d", codecContext->ch_layout.nb_channels, codecContext->sample_rate, av_get_bytes_per_sample(codecContext->sample_fmt) * 8, codecContext->ch_layout.nb_channels * av_get_bytes_per_sample(codecContext->sample_fmt));
    wf.wFormatTag = WAVE_FORMAT_PCM;
    wf.nChannels = 1;
    wf.nSamplesPerSec = codecContext->sample_rate;// = codecContext->sample_rate;
    wf.wBitsPerSample = 32;// = av_get_bytes_per_sample(codecContext->sample_fmt)*8;
    wf.nBlockAlign = wf.wBitsPerSample / 8;//codecContext->ch_layout.nb_channels*wf.wBitsPerSample/8;
    wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;

    wf.cbSize = 0;
    MMRESULT result = waveOutOpen(&hWaveOut, WAVE_MAPPER, &wf, (DWORD_PTR)waveOutProc,NULL, CALLBACK_FUNCTION | WAVE_ALLOWSYNC);//0, 0, CALLBACK_NULL);
    if (result != MMSYSERR_NOERROR) {
        //waveOutClose(hWaveOut);
        std::cerr << "Error obtaining HWAVEOUT." << std::endl;
    }
    std::cout << "HWAVEOUT obtained successfully." << std::endl;


    auto out_codec = avcodec_find_decoder(AVCodecID::AV_CODEC_ID_PCM_U32LE);
    SwrContext* resample_ctx = nullptr;
    const AVChannelLayout mono_lay = AV_CHANNEL_LAYOUT_MONO;
    FilteringContext filt_ctx;
    init_filter_context2(codecContext,out_codec->sample_fmts[0],&mono_lay,&filt_ctx);


    // 再生ループ
    AVPacket* read_packet = av_packet_alloc();
    AVFrame* read_frame = av_frame_alloc();
    while (0 <= av_read_frame(formatContext, read_packet)) {
        if (read_packet->stream_index == audioStreamIndex) {
            // 音声パケットを再生
            // ここで音声を再生する処理を追加してください
            int ret;
            ret = avcodec_send_packet(codecContext, read_packet);
            while (0 <= avcodec_receive_frame(codecContext, read_frame)) {

                ret = av_buffersrc_add_frame_flags(filt_ctx.buffersrc_ctx, read_frame, 0);
                
                while (true) {
                    AVFrame* frame = av_frame_alloc();
                    ret = av_buffersink_get_frame(filt_ctx.buffersink_ctx, frame);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                        av_frame_free(&frame);
                        break;
                    }
                    WAVEHDR* wh = new WAVEHDR();
                        wh->lpData = (char*)frame->data[0];
                        wh->dwBufferLength =frame->nb_samples*wf.nBlockAlign;//output_frame->linesize[0];
                        wh->dwBytesRecorded = 0;
                        wh->dwUser = (int64_t)frame;
                        wh->dwFlags = 0;//flg;
                        wh->dwLoops = 0;
                        wh->lpNext = NULL;
                        wh->reserved = 0;
                        result = waveOutPrepareHeader(hWaveOut, wh, sizeof(WAVEHDR));
                        waveOutWrite(hWaveOut, wh, sizeof(WAVEHDR));
                }
                av_frame_unref(read_frame);
            }
            av_packet_unref(read_packet);
        }
    }
    WAVEHDR* wh = new WAVEHDR();
    wh->lpData = 0;//(char*)frame->data[0];
    wh->dwBufferLength = 0;//frame->nb_samples * wf.nBlockAlign;//output_frame->linesize[0];
    wh->dwBytesRecorded = 0;
    wh->dwUser = 1;//(int64_t)frame;
    wh->dwFlags = 0;//flg;
    wh->dwLoops = 0;
    wh->lpNext = NULL;
    wh->reserved = 0;
    result = waveOutPrepareHeader(hWaveOut, wh, sizeof(WAVEHDR));
    waveOutWrite(hWaveOut, wh, sizeof(WAVEHDR));
    cout << "end" << endl;
    while(!waveout_is_close){
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    waveOutClose(hWaveOut);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}