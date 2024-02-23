#pragma once
#include"shareframe.h"
#include<thread>
#include"timer.h"
#include"data_struct.h"
#include"device.h"
#include<queue>
namespace kc2{
class F4MVideoCapture {
private:
	AVFormatContext* ifmt_ctx;
	AVCodecContext* dec_ctx;
	FilteringContext flt_ctx;
	AVPacket* read_packet = av_packet_alloc();
	AVFrame* read_frame = av_frame_alloc();
	std::thread th;
	bool is_active = false;
	bool is_new_frame=false;
	kc2::MeasureFPS fps;
	std::queue<ShareFrame> current_frame;// = nullptr;
	std::mutex m;
	std::mutex m_f;
	int64_t first_time=-1;
public:
	CaptureProperty prop;
	bool is_change_filter = false;
	bool is_out_fream_fresh = false;

	//‚¨‚à‚¢
	bool open(int index, const CaptureProperty& prop);

	void load();

	ShareFrame read();

	//‚¨‚à‚¢‚©‚à
	void release();
	int get_fps();

	void change_flip(int angle,bool vflip);

};

}