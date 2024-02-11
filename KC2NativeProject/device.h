#pragma once
#include"kc2.h"

namespace kc2{

struct VideoInfo {
    int width;
    int height;
    int format;
    double fps;
};
using video_info_list_t = std::vector<VideoInfo>;

struct DeviceInfo {
    wchar_t* name;
    wchar_t* path;
    std::vector<VideoInfo>* video_info_list;
    int video_info_list_size;
};

using device_info_list_t = std::vector<DeviceInfo>;


EXPORT int kc2np_init_device_info_list(device_info_list_t** dlist);
EXPORT DeviceInfo kc2np_device_info_list_get_device_info(device_info_list_t* d, int index);

EXPORT VideoInfo kc2np_video_info_list_get_video_info(video_info_list_t* v, int index);

EXPORT int kc2np_device_info_get_near_size_video_info_index(DeviceInfo device_info, int width, int height);



}