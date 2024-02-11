#include"ffmpeg.h"
#include"kc2.h"
namespace kc2 {
    using namespace std;
    struct VideoSpec {
        int64_t width;
        int64_t height;
        double fps;
    };



    static void set_device_supported_format(void* obj, int level, const char* szFmt, va_list varg) {
        AVFormatContext* fmt_ctx = (AVFormatContext*)obj;
        AVDictionaryEntry* tag = NULL;
        //tag = av_dict_get(fmt_ctx->metadata, "dinfo", tag, AV_DICT_IGNORE_SUFFIX);
        //if (!tag)return;
        //FrameFps* dinfo = (FrameFps*)stoull(tag->value);
        string fmt(szFmt);
        if (fmt.size() >= 100)return;
        for (int i = 0; i < (int)fmt.size() - 6; i++) {
            if (fmt.substr(i, 6) == "min s=") {
                int64_t width =va_arg(varg, int64_t);
                int64_t height = va_arg(varg, int64_t);
                double fps = va_arg(varg, double);
                width = va_arg(varg, int64_t);
                height = va_arg(varg, int64_t);

                fps = va_arg(varg, double);
                char buffer[100];
                std::snprintf(buffer, sizeof(buffer), "%g",fps);
                printf("width:%ld height:%ld fps:%s\n",width,height,buffer);
                break;
            }
        }
    }


    void get_device_list() {
        avdevice_register_all();

        AVFormatContext* ifmt = avformat_alloc_context();
        AVDictionary* opt = NULL;
        av_dict_set(&opt, "list_options", "true", 0);
        string pin = "video=C505 HD Webcam";//(string)device_pin;
      //  f4m::log::FrameFps* temp = new f4m::log::FrameFps;

        //av_dict_set_int(&ifmt->metadata, "dinfo", (int64_t)temp, 0);
        
        
        av_log_set_callback(set_device_supported_format);
        int ret = avformat_open_input(&ifmt, pin.c_str(), av_find_input_format("dshow"), &opt);
        av_log_set_callback(av_log_default_callback);

        avformat_free_context(ifmt);
    }

    EXPORT void test_dev2() {
        get_device_list();
    }


}