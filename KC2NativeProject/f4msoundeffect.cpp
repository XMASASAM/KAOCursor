#include"f4msoundeffect.h"
using namespace std;
namespace f4m {
    namespace sound {
        using namespace std;

        char* to_utf8(const wchar_t* src) {
            char* buf;
            int dst_size, rc;

            rc = WideCharToMultiByte(CP_UTF8, 0, src, -1, NULL, 0, NULL, NULL);//CP_ACP
            if (rc == 0) {
                return NULL;
            }

            dst_size = rc + 1;
            buf = (char*)malloc(dst_size);
            if (buf == NULL) {
                return NULL;
            }

            rc = WideCharToMultiByte(CP_UTF8, 0, src, -1, buf, dst_size, NULL, NULL);
            if (rc == 0) {
                free(buf);
                return NULL;
            }
            buf[rc] = '\0';

            return buf;
        }

        void CALLBACK waveOutProc(HWAVEOUT m_hWO, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

    //    void CALLBACK waveOutProc(HWAVEOUT m_hWO, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

            SoundEffect::SoundEffect(const wchar_t* file_name, double volume) {
                auto utf8_name = to_utf8(file_name);

                AVFormatContext* formatContext = nullptr;
                if (avformat_open_input(&formatContext, utf8_name, nullptr, nullptr) != 0) {
                    std::cerr << "Error opening input file." << std::endl;
                    return;
                }

                if (avformat_find_stream_info(formatContext, nullptr) < 0) {
                    std::cerr << "Error finding stream information." << std::endl;
                    avformat_close_input(&formatContext);
                    return;
                }

                // 最初の音声ストリームを取得
                int audioStreamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
                if (audioStreamIndex < 0) {
                    std::cerr << "Error finding audio stream." << std::endl;
                    avformat_close_input(&formatContext);
                    return;
                }

                // ストリームを開く
                auto codecpar = formatContext->streams[audioStreamIndex]->codecpar;
                const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
                if (!codec) {
                    std::cerr << "Error finding codec." << std::endl;
                    avformat_close_input(&formatContext);
                    return;
                }

                //AVCodecContext* codecContext=nullptr;
                /* Allocate a new decoding context. */
                AVCodecContext* codecContext = avcodec_alloc_context3(codec);


                avcodec_parameters_to_context(codecContext, codecpar);


                if (avcodec_open2(codecContext, codec, nullptr) < 0) {
                    std::cerr << "Error opening codec." << std::endl;
                    avformat_close_input(&formatContext);
                    return ;
                }

                codecContext->pkt_timebase = formatContext->streams[audioStreamIndex]->time_base;



               // HWAVEOUT hWaveOut;
               // WAVEFORMATEX wf;
                std::cout << av_get_sample_fmt_name(codecContext->sample_fmt) << endl;;// == AVSampleFormat::
                printf("ch:%d sample_rate:%d bit_par_sample:%d blockAlign:%d", codecContext->ch_layout.nb_channels, codecContext->sample_rate, av_get_bytes_per_sample(codecContext->sample_fmt) * 8, codecContext->ch_layout.nb_channels * av_get_bytes_per_sample(codecContext->sample_fmt));
                waveFormat.wFormatTag = WAVE_FORMAT_PCM;
                waveFormat.nChannels = 1;
                waveFormat.nSamplesPerSec = codecContext->sample_rate;// = codecContext->sample_rate;
                waveFormat.wBitsPerSample = 32;// = av_get_bytes_per_sample(codecContext->sample_fmt)*8;
                waveFormat.nBlockAlign = waveFormat.wBitsPerSample / 8;//codecContext->ch_layout.nb_channels*wf.wBitsPerSample/8;
                waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;

                waveFormat.cbSize = 0;
                auto out_codec = avcodec_find_decoder(AVCodecID::AV_CODEC_ID_PCM_S32LE);
                SwrContext* resample_ctx = nullptr;
                const AVChannelLayout mono_lay = AV_CHANNEL_LAYOUT_MONO;
                int ret = swr_alloc_set_opts2(&resample_ctx, &mono_lay, out_codec->sample_fmts[0], waveFormat.nSamplesPerSec, &codecContext->ch_layout, codecContext->sample_fmt, codecContext->sample_rate, 0, NULL);
                ret = swr_init(resample_ctx);

                // 再生ループ
                AVPacket* read_packet = av_packet_alloc();
                AVFrame* read_frame = av_frame_alloc();


                int ret_read;
                while (0 <= (ret_read = av_read_frame(formatContext, read_packet))) {
                    if (read_packet->stream_index == audioStreamIndex) {
                        // 音声パケットを再生
                        // ここで音声を再生する処理を追加してください
                        int ret;
                        ret = avcodec_send_packet(codecContext, read_packet);
                        while (0 <= avcodec_receive_frame(codecContext, read_frame)) {
                            uint8_t** converted_input_samples = NULL;
                            av_samples_alloc_array_and_samples(&converted_input_samples, NULL, 1, read_frame->nb_samples, out_codec->sample_fmts[0], 0);

                            ret = swr_convert(resample_ctx, converted_input_samples, read_frame->nb_samples, (const uint8_t**)(read_frame->extended_data), read_frame->nb_samples);

                            int clen = read_frame->nb_samples * waveFormat.nBlockAlign;
                            mdata = (char*)std::realloc(mdata, mdata_len + clen);//cbCurrentLength);
                            std::memcpy(mdata + mdata_len, converted_input_samples[0], clen);
                            mdata_len += clen;


                            // 解放.
                            if (converted_input_samples)
                                av_freep(&converted_input_samples[0]);
                            av_freep(&converted_input_samples);
                            av_frame_unref(read_frame);
                        }
                    }
                    av_packet_unref(read_packet);
                }

                mdata_volume = (char*)malloc(mdata_len);
                std::memcpy(mdata_volume, mdata, mdata_len);

                // 後片付け
                avcodec_close(codecContext);
                avformat_close_input(&formatContext);
                av_packet_free(&read_packet);
                av_frame_free(&read_frame);

            }

            void SoundEffect::set_volume(double v) {
                for (size_t i = 0; i < mdata_len; i += 2) {
                    int16_t sample = *reinterpret_cast<int16_t*>(&mdata[i]);
                    int64_t temp = sample * v;
                    if (MAXINT16 < temp)temp = MAXINT16;
                    if (MININT16 > temp)temp = MININT16;
                    //  printf("id:%d data:%d\n",i,temp);
                    *reinterpret_cast<int16_t*>(&mdata_volume[i]) = static_cast<int16_t>(temp);
                }
            }


            void SoundEffect::play(double volume) {
                seplay* hw = new seplay();
                MMRESULT result = waveOutOpen(&hw->hWaveOut, WAVE_MAPPER, &waveFormat, (DWORD_PTR)waveOutProc, (DWORD_PTR)this, CALLBACK_FUNCTION | WAVE_ALLOWSYNC);
                if (result != 0) {
                    int el[] = { MMSYSERR_ALLOCATED ,MMSYSERR_BADDEVICEID ,MMSYSERR_NODRIVER ,MMSYSERR_NOMEM ,WAVERR_BADFORMAT ,WAVERR_SYNC };
                    for (auto& i : el) {
                        if (result == i) {
                            cout << "error:" << i << endl;
                        }
                    }
                }
                condition_variable* cv_done = new condition_variable();
                WAVEHDR* wh = new WAVEHDR();
                wh->lpData = (char*)mdata_volume;
                wh->dwBufferLength = mdata_len;
                wh->dwBytesRecorded = 0;
                wh->dwUser = (int64_t)hw;
                wh->dwFlags = 0;
                wh->dwLoops = 0;
                wh->lpNext = NULL;
                wh->reserved = 0;
                waveOutPrepareHeader(hw->hWaveOut, wh, sizeof(WAVEHDR));
                waveOutWrite(hw->hWaveOut, wh, sizeof(WAVEHDR));
                st.insert(hw);
                thread th1(&f4m::sound::SoundEffect::wait_clse, this, hw);
                th1.detach();
            }

            void SoundEffect::wait_clse(seplay* hw) {
                {
                    std::unique_lock<std::mutex> l(hw->mtx);
                    hw->cv.wait(l, [&] {return hw->notify; });

                    auto h = waveOutClose(hw->hWaveOut);
                    if (h != 0) {
                        int el[] = { MMSYSERR_INVALHANDLE ,MMSYSERR_NODRIVER ,MMSYSERR_NOMEM ,WAVERR_STILLPLAYING };
                        cout << "error:" << h << endl;

                    }
                }
                cout << "close" << endl;
                st.erase(hw);
                delete hw;

                cv_close.notify_one();
            }

            SoundEffect::~SoundEffect() {
                release();
            }

            void SoundEffect::release() {
                auto t = st;
                cout << "reset" << endl;
                mutex mtx;
                for (auto& i : t) {
                    waveOutReset(i->hWaveOut);
                }
                {
                    std::unique_lock<std::mutex> l(mtx);
                    cv_close.wait(l, [&] {return st.size()==0; });
                }

                free(mdata);
                free(mdata_volume);
            }


        

        void CALLBACK waveOutProc(HWAVEOUT m_hWO, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
            SoundEffect* se = (sound::SoundEffect*)dwInstance;
            if (uMsg == WOM_OPEN) {
                cout << "call open" << endl;
            }

            if (uMsg == WOM_DONE) {
                cout << "call done" << endl;
                WAVEHDR* wh = (WAVEHDR*)dwParam1;
                seplay* sp = (seplay*)wh->dwUser;
                sp->notify = true;
                sp->cv.notify_one();
                delete wh;
            }

            if (uMsg == WOM_CLOSE) {
                cout << "call close" << endl;
            }
        }
        EXPORT void test_sound11() {
            std::wstring FilePath = L"C:\\Users\\MaMaM\\source\\repos\\KC2\\KC2\\pc-mouse-3.mp3";
            wstring f2 = L"C:\\Users\\MaMaM\\Downloads\\Morning (1).mp3";
            //    C:\Users\MaMaM\source\repos\KC2\KC2\PC - Mouse03 - 06(R).mp3"
            SoundEffect se(FilePath.c_str(), 1.0);//f2.c_str(), 1.0);
         //   SoundEffect se(f2.c_str(),1.0);//f2.c_str(), 1.0);
            //   se.play(1);
               se.set_volume(1);
            for (int i = 0; i < 10; i++) {
                se.play(1);

                cout << i << "end" << endl;
           //     std::this_thread::sleep_for(std::chrono::milliseconds(10000));
            }
           // se.release();
          //  std::this_thread::sleep_for(std::chrono::seconds(5));

        }

    }
}
