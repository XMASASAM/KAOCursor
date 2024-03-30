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
        int sum=0;
        int ans=0;
    public:
        void count();
        int get_fps();
    };

    class CountPS {
    private:
        timer time;
        double sum=0;
        double ans=0;
        int samples=0;
    public:
        void count(double v);
        double get_countps();
        void clear();
    };
}