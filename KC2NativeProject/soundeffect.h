#pragma once
#include <iostream>
#include <string>
#include <cstdlib>
#include"kc2.h"
#include<mmeapi.h >
#include<mmsystem.h>
#include<mfapi.h>
#include <mfidl.h>
#include<mfreadwrite.h>
#include<thread>
#include <mutex>
#include<condition_variable>
#include <chrono>
#include<unordered_set>
namespace kc2 {
    namespace sound {
        using namespace std;


        void CALLBACK waveOutProc(HWAVEOUT m_hWO, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
        struct seplay {
            HWAVEOUT hWaveOut;
            bool notify = false;
            condition_variable cv;
            mutex mtx;
        };
        class SoundEffect {
            IMFSourceReader* pMFSourceReader = nullptr;
            WAVEFORMATEX* waveFormat = nullptr;
            //  HWAVEOUT hWaveOut;// = nullptr;
            IMFMediaType* pMFMediaType = nullptr;
            char* mdata = nullptr;
            char* mdata_volume = nullptr;
            int64_t mdata_len = 0;
        public:
            double volume;
            thread th_play;
            bool cv_notify = false;
            bool is_finished = false;
            std::condition_variable cv_play;
            unordered_set<seplay*> st;
            SoundEffect(const wchar_t* file_name, double volume) {
                CoInitializeEx(nullptr, COINIT_MULTITHREADED);
                MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
                auto hr = MFCreateSourceReaderFromURL(file_name, NULL, &pMFSourceReader);
                //CreateSourceReaderAsync(file_name,NULL,&pMFSourceReader);
                if (FAILED(hr)) {
                    cout << "error!!!!!" << endl;
                }
                hr = MFCreateMediaType(&pMFMediaType);
                if (FAILED(hr)) {
                    cout << "error!!!!!" << endl;
                }
                hr = pMFMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
                if (FAILED(hr)) {
                    cout << "error!!!!!" << endl;
                }
                hr = pMFMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
                if (FAILED(hr)) {
                    cout << "error!!!!!" << endl;
                }
                hr = pMFSourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, pMFMediaType);
                if (FAILED(hr)) {
                    cout << "error!!!!!" << endl;
                }
                hr = pMFMediaType->Release();
                if (FAILED(hr)) {
                    cout << "error!!!!!" << endl;
                }
                pMFMediaType = nullptr;
                hr = pMFSourceReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &pMFMediaType);
                if (FAILED(hr)) {
                    cout << "error!!!!!" << endl;
                }


                hr = MFCreateWaveFormatExFromMFMediaType(pMFMediaType, &waveFormat, nullptr);
                if (FAILED(hr)) {
                    cout << "error!!!!!" << endl;
                }
                //    MMRESULT result = waveOutOpen(&hWaveOut, WAVE_MAPPER, waveFormat, (DWORD_PTR)waveOutProc, (DWORD_PTR)this, CALLBACK_FUNCTION | WAVE_ALLOWSYNC);//0, 0, CALLBACK_NULL);

                while (true) {
                    IMFSample* pMFSample{ nullptr };
                    DWORD dwStreamFlags{ 0 };
                    pMFSourceReader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, nullptr, &dwStreamFlags, nullptr, &pMFSample);

                    if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM)
                    {
                        break;
                    }

                    IMFMediaBuffer* pMFMediaBuffer{ nullptr };
                    pMFSample->ConvertToContiguousBuffer(&pMFMediaBuffer);

                    BYTE* pBuffer{ nullptr };
                    DWORD cbCurrentLength{ 0 };
                    pMFMediaBuffer->Lock(&pBuffer, nullptr, &cbCurrentLength);
                    //BYTE* mdata = new BYTE[cbCurrentLength];
                    mdata = (char*)std::realloc(mdata, mdata_len + cbCurrentLength);//cbCurrentLength);
                    std::memcpy(mdata + mdata_len, pBuffer, cbCurrentLength);
                    mdata_len += cbCurrentLength;

                    pMFMediaBuffer->Unlock();
                    pMFMediaBuffer->Release();
                    pMFSample->Release();

                    //byteÇÃëÂÇ´Ç≥å„Ç≈ê∏ç∏.
              /*      for (size_t i = 0; i < cbCurrentLength; i += 2) {
                        int16_t sample = *reinterpret_cast<int16_t*>(&mdata[i]);
                        int64_t temp = sample * volume;
                        if (MAXINT16 < temp)temp = MAXINT16;
                        if (MININT16 > temp)temp = MININT16;
                        //  printf("id:%d data:%d\n",i,temp);
                        *reinterpret_cast<int16_t*>(&mdata[i]) = static_cast<int16_t>(temp);
                    }*/
                }

                //   waveOutClose(hWaveOut);
       //            CoTaskMemFree(waveFormat);
                pMFMediaType->Release();
                pMFSourceReader->Release();
                MFShutdown();
                CoUninitialize();

                mdata_volume = (char*)malloc(mdata_len);
                std::memcpy(mdata_volume, mdata, mdata_len);

            }

            void set_volume(double v) {
                for (size_t i = 0; i < mdata_len; i += 2) {
                    int16_t sample = *reinterpret_cast<int16_t*>(&mdata[i]);
                    int64_t temp = sample * v;
                    if (MAXINT16 < temp)temp = MAXINT16;
                    if (MININT16 > temp)temp = MININT16;
                    //  printf("id:%d data:%d\n",i,temp);
                    *reinterpret_cast<int16_t*>(&mdata_volume[i]) = static_cast<int16_t>(temp);
                }
            }


            void play(double volume) {
                seplay* hw = new seplay();
                MMRESULT result = waveOutOpen(&hw->hWaveOut, WAVE_MAPPER, waveFormat, (DWORD_PTR)waveOutProc, (DWORD_PTR)this, CALLBACK_FUNCTION | WAVE_ALLOWSYNC);
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
                thread th1(&kc2::sound::SoundEffect::wait_clse, this, hw);
                th1.detach();
            }

            void wait_clse(seplay* hw) {
                {
                    std::unique_lock<std::mutex> l(hw->mtx);
                    hw->cv.wait(l, [&] {return hw->notify; });

                    auto h = waveOutClose(hw->hWaveOut);
                    if (h != 0) {
                        int el[] = { MMSYSERR_INVALHANDLE ,MMSYSERR_NODRIVER ,MMSYSERR_NOMEM ,WAVERR_STILLPLAYING };
                        cout << "error:" << h << endl;

                    }
                }
                st.erase(hw);
                delete hw;
            }

            ~SoundEffect() {
                release();
            }

            void release() {
                auto t = st;
                cout << "reset" << endl;
                for (auto& i : t) {
                    waveOutReset(i->hWaveOut);
                }
                free(mdata);
            }
            void thread_func_play(double volume) {
                CoInitializeEx(nullptr, COINIT_MULTITHREADED);
                MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
                this->volume = volume;


                MFShutdown();
                CoUninitialize();
                cout << "se_end" << endl;
            }

            void send(char* mdata, int64_t len) {
                WAVEHDR* wh = new WAVEHDR();
                wh->lpData = (char*)mdata;//(char*)frame->data[0];
                wh->dwBufferLength = len;//frame->nb_samples * wf.nBlockAlign;//output_frame->linesize[0];
                wh->dwBytesRecorded = 0;
                wh->dwUser = (int64_t)mdata;//(int64_t)frame;
                wh->dwFlags = 0;//flg;
                wh->dwLoops = 0;
                wh->lpNext = NULL;
                wh->reserved = 0;
                //   waveOutPrepareHeader(hWaveOut, wh, sizeof(WAVEHDR));
                  // waveOutWrite(hWaveOut, wh, sizeof(WAVEHDR));
            }

            bool read_and_send(char** data, int64_t* data_len) {
                cout << "read_and_send" << endl;
                if (is_finished) {
                    *data = nullptr;
                    *data_len = 0;
                    return false;
                }
                IMFSample* pMFSample{ nullptr };
                DWORD dwStreamFlags{ 0 };
                pMFSourceReader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, nullptr, &dwStreamFlags, nullptr, &pMFSample);

                if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM)
                {
                    send(nullptr, 0);
                    is_finished = true;
                    *data = nullptr;
                    *data_len = 0;
                    return true;
                }

                IMFMediaBuffer* pMFMediaBuffer{ nullptr };
                pMFSample->ConvertToContiguousBuffer(&pMFMediaBuffer);

                BYTE* pBuffer{ nullptr };
                DWORD cbCurrentLength{ 0 };
                pMFMediaBuffer->Lock(&pBuffer, nullptr, &cbCurrentLength);

                BYTE* mdata = new BYTE[cbCurrentLength];

                std::memcpy(mdata, pBuffer, cbCurrentLength);
                pMFMediaBuffer->Unlock();
                pMFMediaBuffer->Release();
                pMFSample->Release();

                //byteÇÃëÂÇ´Ç≥å„Ç≈ê∏ç∏.
                for (size_t i = 0; i < cbCurrentLength; i += 2) {
                    int16_t sample = *reinterpret_cast<int16_t*>(&mdata[i]);
                    int64_t temp = sample * volume;
                    if (MAXINT16 < temp)temp = MAXINT16;
                    if (MININT16 > temp)temp = MININT16;
                    //  printf("id:%d data:%d\n",i,temp);
                    *reinterpret_cast<int16_t*>(&mdata[i]) = static_cast<int16_t>(temp);
                }
                //send((char*)mdata,cbCurrentLength);
                *data = (char*)mdata;
                *data_len = cbCurrentLength;
                return true;
            }


        };

        void CALLBACK waveOutProc(HWAVEOUT m_hWO, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
            SoundEffect* se = (SoundEffect*)dwInstance;
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
        EXPORT void test_sound10() {
            std::wstring FilePath = L"C:\\Users\\MaMaM\\source\\repos\\KC2\\KC2\\pc-mouse-3.mp3";
            wstring f2 = L"C:\\Users\\MaMaM\\Downloads\\Morning (1).mp3";
            //    C:\Users\MaMaM\source\repos\KC2\KC2\PC - Mouse03 - 06(R).mp3"
            SoundEffect se(f2.c_str(), 1.0);
            //   se.play(1);
             //  se.set_volume(2);
            for (int i = 0; i < 100; i++) {
                se.play(1);

                cout << i << "end" << endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(11000));
            }
            se.release();
            std::this_thread::sleep_for(std::chrono::seconds(5));

        }

    }
}
