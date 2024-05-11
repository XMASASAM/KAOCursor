#include <iostream>
#include <string>
#include <cstdlib>
#include"kc2.h"
#include"ffmpeg.h"
#include<mmsystem.h>
#include <libavutil/channel_layout.h>
#include<thread>
using namespace std;


EXPORT int test_sound() {
    //av_register_all();

    /*
    auto lpBuf = (LPBYTE)GlobalAlloc(GPTR, 8000);

    for (int i = 0; i < 8000; i++) {

        if ((i % 10) < 5)
            lpBuf[i] = 192;
        else
            lpBuf[i] = 64;

    }
    WAVEHDR wh;

    wh.lpData = (char*)lpBuf;
    wh.dwBufferLength = 8000;
    wh.dwBytesRecorded = 0;
    wh.dwUser = 0;
    wh.dwFlags = 0;
    wh.dwLoops = 1;
    wh.lpNext = NULL;
    wh.reserved = 0;*/

    //waveOutWrite(hWaveOut, &wh, sizeof(WAVEHDR));
    //std::this_thread::sleep_for(std::chrono::seconds(3));
    //exit(0);
    // mp3ファイルのパス
    //std::string mp3FilePath = "C:\\Users\\MaMaM\\source\\repos\\KC2\\KC2\\PC-Mouse03-06(R).mp3";
    std::string mp3FilePath = "C:\\Users\\MaMaM\\Downloads\\Morning (1).mp3";
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
    wf.wFormatTag =WAVE_FORMAT_PCM;
    wf.nChannels = 1;
    wf.nSamplesPerSec=codecContext->sample_rate;// = codecContext->sample_rate;
    wf.wBitsPerSample=32;// = av_get_bytes_per_sample(codecContext->sample_fmt)*8;
    wf.nBlockAlign = wf.wBitsPerSample/8;//codecContext->ch_layout.nb_channels*wf.wBitsPerSample/8;
    wf.nAvgBytesPerSec = wf.nSamplesPerSec*wf.nBlockAlign;

    wf.cbSize = 0;
    MMRESULT result = waveOutOpen(&hWaveOut, WAVE_MAPPER, &wf, 0, 0, CALLBACK_NULL);
    if (result != MMSYSERR_NOERROR) {
        //waveOutClose(hWaveOut);
        std::cerr << "Error obtaining HWAVEOUT." << std::endl;
    }
    std::cout << "HWAVEOUT obtained successfully." << std::endl;


    auto out_codec =  avcodec_find_decoder(AVCodecID::AV_CODEC_ID_PCM_S32LE);
    SwrContext* resample_ctx=nullptr;
    const AVChannelLayout mono_lay = AV_CHANNEL_LAYOUT_MONO;
    int ret = swr_alloc_set_opts2(&resample_ctx,&mono_lay,out_codec->sample_fmts[0], wf.nSamplesPerSec, &codecContext->ch_layout, codecContext->sample_fmt, codecContext->sample_rate, 0, NULL);
    ret = swr_init(resample_ctx);

    AVAudioFifo* fifo = NULL;
    fifo = av_audio_fifo_alloc(out_codec->sample_fmts[0], 1, 1);



    // 再生ループ
    AVPacket* read_packet = av_packet_alloc();
    AVFrame* read_frame = av_frame_alloc();
    int output_frame_size=wf.nSamplesPerSec/2;

    bool is_finish = false;
    bool is_first = true;
    int8_t* se_pcm_data = nullptr;
    int se_pcm_data_len = 0;
    while(!is_finish){
//        while (av_audio_fifo_size(fifo) < output_frame_size) {
            int ret_read;
            while (0<=(ret_read=av_read_frame(formatContext, read_packet)) && av_audio_fifo_size(fifo) < output_frame_size) {
                if (read_packet->stream_index == audioStreamIndex) {
                    // 音声パケットを再生
                    // ここで音声を再生する処理を追加してください
                    int ret;
                    ret = avcodec_send_packet(codecContext, read_packet);
                    while (0 <= avcodec_receive_frame(codecContext, read_frame)) {
                        uint8_t** converted_input_samples = NULL;
                        av_samples_alloc_array_and_samples(&converted_input_samples, NULL, 1, read_frame->nb_samples, out_codec->sample_fmts[0], 0);
                        
                 //       for (int i = 0; i < read_frame->nb_samples; i+=2) {
                           // printf("init%d:%d\n",i, (INT16)(read_frame->data[0][i + 1] << 8) + (read_frame->data[0][i]));
                  //      }
                        
                        ret = swr_convert(resample_ctx, converted_input_samples, read_frame->nb_samples, (const uint8_t**)(read_frame->extended_data), read_frame->nb_samples);
                    //    for (int i = 0; i < read_frame->nb_samples; i++) {
                           // printf("conv%d:%d\n", i, converted_input_samples[0][i]);
                      //  }

/*                        WAVEHDR* wh = new WAVEHDR();
                        wh->lpData = (char*)converted_input_samples[0];
                        wh->dwBufferLength = read_frame->nb_samples* wf.nBlockAlign;//output_frame->linesize[0];
                        wh->dwBytesRecorded = 0;
                        wh->dwUser = 0;
                        wh->dwFlags = 0;//flg;
                        wh->dwLoops = 0;
                        wh->lpNext = NULL;
                        wh->reserved = 0;
                        result = waveOutPrepareHeader(hWaveOut, wh, sizeof(WAVEHDR));

                        waveOutWrite(hWaveOut, wh, sizeof(WAVEHDR));*/
                        
                        int clen = read_frame->nb_samples * wf.nBlockAlign;
                        se_pcm_data = (int8_t*)std::realloc(se_pcm_data, se_pcm_data_len + clen);//cbCurrentLength);
                        std::memcpy(se_pcm_data + se_pcm_data_len, converted_input_samples[0], clen);
                        se_pcm_data_len += clen;

                        
                        /*
                        ret = av_audio_fifo_realloc(fifo, av_audio_fifo_size(fifo) + read_frame->nb_samples);
                        if (av_audio_fifo_write(fifo, (void**)converted_input_samples, read_frame->nb_samples) < read_frame->nb_samples) {
                            std::cout << "Could not write data" << endl;
                            is_finish=true;
                        }*/

                       // 解放.
                        if (converted_input_samples)
                            av_freep(&converted_input_samples[0]);
                        av_freep(&converted_input_samples);
                        av_frame_unref(read_frame);
                    }
                    std::cout << "size:" << av_audio_fifo_size(fifo) << endl;
                }
                av_packet_unref(read_packet);

            }
            if (ret_read < 0) {
                WAVEHDR* wh = new WAVEHDR();
                wh->lpData = (char*)se_pcm_data;
                wh->dwBufferLength = se_pcm_data_len;//output_frame->linesize[0];
                wh->dwBytesRecorded = 0;
                wh->dwUser = 0;
                wh->dwFlags = 0;//flg;
                wh->dwLoops = 0;
                wh->lpNext = NULL;
                wh->reserved = 0;
                result = waveOutPrepareHeader(hWaveOut, wh, sizeof(WAVEHDR));

                waveOutWrite(hWaveOut, wh, sizeof(WAVEHDR));
                std::this_thread::sleep_for(std::chrono::seconds(200));

                is_finish=true;
            }
            while (output_frame_size <= av_audio_fifo_size(fifo) ||
                (is_finish && av_audio_fifo_size(fifo) > 0)) {
                int frame_size;
                if (av_audio_fifo_size(fifo) < output_frame_size) {
                    frame_size = av_audio_fifo_size(fifo);
                }
                else {
                    frame_size = output_frame_size;
                }
                AVFrame* output_frame = av_frame_alloc();
                output_frame->nb_samples = frame_size;
                av_channel_layout_copy(&output_frame->ch_layout, &mono_lay);
                output_frame->format = out_codec->sample_fmts[0];
                output_frame->sample_rate = wf.nSamplesPerSec;
                av_frame_get_buffer(output_frame,0);
                av_audio_fifo_read(fifo, (void**)output_frame->data, frame_size);
                std::cout << "success:" << output_frame->nb_samples << endl;
                //for (int i = 0; i < output_frame->linesize[0]; i+=2) {
               //     printf("data%d:%ld\n",i,(UINT16)(output_frame->data[0][i+1] << 8)+ (UINT16)(output_frame->data[0][i]));

                //}
                WAVEHDR* wh = new WAVEHDR();

               /* wh.lpData = (char*)output_frame->data[0];
                wh.dwBufferLength = output_frame->linesize[0];
                wh.dwBytesRecorded = 0;
                wh.dwUser = 0;
                wh.dwFlags = 0;//flg;
                wh.dwLoops = 0;
                wh.lpNext = NULL;
                wh.reserved = 0;*/

                wh->lpData = (char*)output_frame->data[0];
                wh->dwBufferLength = frame_size*wf.nBlockAlign;
                wh->dwBytesRecorded = 0;
                wh->dwUser = 0;
                wh->dwFlags = 0;//flg;
                wh->dwLoops = 0;
                wh->lpNext = NULL;
                wh->reserved = 0;

                int flg = 0;
                if (is_first) {
                    flg |= WHDR_BEGINLOOP;
                    is_first = false;

                }
                if (is_finish)flg |= WHDR_ENDLOOP;
            //    HWAVEOUT hWaveOut;
           //     MMRESULT result = waveOutOpen(&hWaveOut, WAVE_MAPPER, &wf, 0, 0, CALLBACK_NULL);


                result = waveOutPrepareHeader(hWaveOut, wh, sizeof(WAVEHDR));

                waveOutWrite(hWaveOut, wh, sizeof(WAVEHDR));

                std::cout << "time_start" << endl;

//                std::this_thread::sleep_for(std::chrono::);
                std::cout<<"time_stop" << endl;
                //cout << "data:end" << endl;
            //    std::this_thread::sleep_for(std::chrono::microseconds(20000));
               // av_frame_free(&output_frame);
            }
        
    }
    std::cout << "end" << endl;
        std::this_thread::sleep_for(std::chrono::seconds(200));


                    //av_frame_free(&input_frame);
                
                
            
                /*cout << read_frame->linesize[0] << endl;
                WAVEHDR hdata;
                hdata.lpData = (char*)read_frame->data[0];
                hdata.dwBufferLength = read_frame->linesize[0];
              //  hdata.dwFlags= WHDR_BEGINLOOP | WHDR_ENDLOOP;
                hdata.dwLoops = 1;


                for (int i = 0; i < read_frame->linesize[0]; i+=2) {
                    cout << (int64_t)((read_frame->data[0][i]<<8) + read_frame->data[0][i+1]) << endl;
                }

                
                result = waveOutPrepareHeader(hWaveOut, &hdata, sizeof(WAVEHDR));
                if (result == MMSYSERR_NOERROR) {
                    cout << "error" << endl;
                }
                else {
                    
                    result = waveOutWrite(hWaveOut, &hdata, sizeof(WAVEHDR));
                    if (result == MMSYSERR_NOERROR) {
                        cout << "error2" << endl;
                    }
                    else {
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                    }
                }*/

    // 後片付け
    avcodec_close(codecContext);
    avformat_close_input(&formatContext);
    av_packet_free(&read_packet);
    av_frame_free(&read_frame);
    return 0;
}