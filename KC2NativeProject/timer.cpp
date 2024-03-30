#include"timer.h"
namespace kc2 {
    timer::timer(bool is_start) {
        if (is_start)start();
    }
    void timer::start() {
        ch = std::chrono::system_clock::now();
    }
    long long timer::elapsed() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - ch).count();
    }

    void MeasureFPS::count() {
        if (time.elapsed() >= 1000) {
            ans = sum;
            sum = 0;
            time.start();
        }
        sum++;


    }
    int MeasureFPS::get_fps() {
        
        return ans;
    }


    void CountPS::count(double v) {
        if (time.elapsed() >= 1000) {
            ans = sum / samples;
            samples=0;
            sum = 0;
            time.start();
        }
        sum+=v;
        samples++;
    }

    double CountPS::get_countps() {
        return ans;
    }

    void CountPS::clear() {
        sum=0;
        ans=0;
        samples=0;
    }

}