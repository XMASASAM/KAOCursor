#pragma once
#include<chrono>
namespace kc2 {
    class timer {
    private:
        std::chrono::system_clock::time_point ch;
    public:
        timer(bool is_start = true);
        void start();
        long long elapsed();
    };

    class MeasureFPS {
    private:
        timer time;
        int sum;
        int ans;
    public:
        void count();
        int get_fps();
    };
}