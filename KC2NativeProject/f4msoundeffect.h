#pragma once
#include <iostream>
#include <string>
#include <cstdlib>
#include"kc2.h"
#include"ffmpeg.h"
#include<mmsystem.h>
#include <libavutil/channel_layout.h>
#include<thread>
#include<condition_variable>
#include<unordered_set>
using namespace std;
namespace f4m {
    namespace sound {
        using namespace std;

        struct seplay {
            HWAVEOUT hWaveOut;
            bool notify = false;
            condition_variable cv;
            mutex mtx;
        };
        class SoundEffect {
            WAVEFORMATEX waveFormat;
            char* mdata = nullptr;
            char* mdata_volume = nullptr;
            int64_t mdata_len = 0;
            std::condition_variable cv_close;
            double volume;
            thread th_play;
            bool cv_notify = false;
            bool is_finished = false;
            std::condition_variable cv_play;
            bool is_opened = false;
            unordered_set<seplay*> st;

            void wait_clse(seplay* hw);
            void release();
        public:


            SoundEffect(const wchar_t* file_name, double volume);
            void set_volume(double v);
            void play(double volume);
            ~SoundEffect();
        };
    
    
    }
}