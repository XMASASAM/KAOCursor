#include"ffmpeg.h"
#include"data_struct.h"
#include"device.h"
#include<thread>
#include<queue>
#include"timer.h"
#include<opencv2/opencv.hpp>
namespace kc2 {
using namespace std;


class ShareFrame {
private:
	static unordered_map<AVFrame**,int> mp;
	static mutex m;
	AVFrame** fp = nullptr;
public:
	AVFrame* get() {
		if(fp==nullptr || *fp==nullptr)return nullptr;
		return *fp;
	}

	void try_free() {

		if(fp==nullptr)return;

		{
			lock_guard lock(m);
			mp[fp]--;
			//printf("pop: fp:%ld count:%d\n", fp, mp[fp]);

			if (mp[fp] <= 0) {

				mp.erase(fp);
				if(*fp!=nullptr){
					av_frame_free(fp);
					delete fp;
					*fp=nullptr;
				}
			}
			fp = nullptr;
		}
	}

	void set_frame(AVFrame** new_frame_ptr) {
		if (new_frame_ptr == nullptr || *new_frame_ptr == nullptr)return;

		{
			lock_guard lock(m);
			fp = new_frame_ptr;
			mp[fp]++;
			//printf("set: fp:%ld count:%d\n",fp,mp[fp]);
		}
	}

	ShareFrame(AVFrame* a) {

		try_free();
		auto f = new AVFrame*;
		*f = a;
		set_frame(f);
	}

	ShareFrame(ShareFrame& a) {
		try_free();
		set_frame(a.fp);
	}


	~ShareFrame() {
		try_free();
	}

	ShareFrame& operator=(ShareFrame&& a)noexcept {
		try_free();
		set_frame(a.fp);
		a.try_free();
		return *this;
	}


	ShareFrame& operator=(ShareFrame& a) {
		try_free();
		set_frame(a.fp);
		return *this;
	}

};

unordered_map<AVFrame**, int> ShareFrame::mp;
mutex ShareFrame::m;

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


void init_filter_context(AVCodecContext* dec_ctx, AVPixelFormat input_pix_fmt, AVPixelFormat output_pix_fmt, FilteringContext* filter_ctx) {
//	if (dec_ctx->codec_type != AVMEDIA_TYPE_VIDEO)return 0;
	char args[512];
	int ret = 0;
	const AVFilter* buffersrc = NULL;
	const AVFilter* buffersink = NULL;
	AVFilterContext* buffersrc_ctx = NULL;
	AVFilterContext* buffersink_ctx = NULL;
	AVFilterInOut* outputs = avfilter_inout_alloc();
	AVFilterInOut* inputs = avfilter_inout_alloc();
	AVFilterGraph* filter_graph = avfilter_graph_alloc();
	buffersrc = avfilter_get_by_name("buffer");
	buffersink = avfilter_get_by_name("buffersink");
	snprintf(args, sizeof(args),
		"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
		dec_ctx->width, dec_ctx->height, input_pix_fmt,
		dec_ctx->pkt_timebase.num, dec_ctx->pkt_timebase.den,
		dec_ctx->sample_aspect_ratio.num,
		dec_ctx->sample_aspect_ratio.den);

	avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in", args, NULL, filter_graph);
	avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out", NULL, NULL, filter_graph);
	av_opt_set_bin(buffersink_ctx, "pix_fmts", (uint8_t*)&output_pix_fmt, sizeof(output_pix_fmt), AV_OPT_SEARCH_CHILDREN);

	outputs->name = av_strdup("in");
	outputs->filter_ctx = buffersrc_ctx;
	outputs->pad_idx = 0;
	outputs->next = NULL;

	inputs->name = av_strdup("out");
	inputs->filter_ctx = buffersink_ctx;
	inputs->pad_idx = 0;
	inputs->next = NULL;

	avfilter_graph_parse_ptr(filter_graph, "null", &inputs, &outputs, NULL);
	avfilter_graph_config(filter_graph, NULL);

	filter_ctx->buffersrc_ctx = buffersrc_ctx;
	filter_ctx->buffersink_ctx = buffersink_ctx;
	filter_ctx->filter_graph = filter_graph;

	avfilter_inout_free(&inputs);
	avfilter_inout_free(&outputs);
}


bool open_input_video_device(const wchar_t* name,const kc2::VideoInfo& prop,bool flag_fps_option,AVFormatContext** ifmt_ctx,AVCodecContext** codec_ctx){
	*ifmt_ctx = nullptr;
	AVDictionary* opt=nullptr;

	string opt1_str = to_string(prop.width) + "x" + to_string(prop.height);
	av_dict_set(&opt, "video_size", opt1_str.c_str(), 0);

	if (flag_fps_option) {
		char buffer[30];
		std::snprintf(buffer, sizeof(buffer), "%g", prop.fps);
		av_dict_set(&opt, "framerate", buffer, 0);
	}
	
	string open_device_name = "video=" + (string)(to_utf8(name));
	int ret = avformat_open_input(ifmt_ctx, open_device_name.c_str(), av_find_input_format("dshow"), &opt);

	if (ret < 0)return false;

	opt = nullptr;

	ret = avformat_find_stream_info(*ifmt_ctx, &opt);

	for (int i = 0; i < (*ifmt_ctx)->nb_streams; i++) {
		AVStream* stream = (*ifmt_ctx)->streams[i];
		const AVCodec* dec = avcodec_find_decoder(stream->codecpar->codec_id);

		if (dec == nullptr)continue;

		if (dec->type != AVMEDIA_TYPE_VIDEO)continue;

		*codec_ctx = avcodec_alloc_context3(dec);
		if (codec_ctx == nullptr)continue;

		ret = avcodec_parameters_to_context(*codec_ctx, stream->codecpar);
		if (ret < 0) {
			avcodec_free_context(codec_ctx);
			continue;
		}

		(*codec_ctx)->pkt_timebase = stream->time_base;
		(*codec_ctx)->framerate = av_guess_frame_rate(*ifmt_ctx, stream, NULL);
		ret = avcodec_open2(*codec_ctx, dec, NULL);
		if (ret < 0) {
			avcodec_free_context(codec_ctx);
			avformat_close_input(ifmt_ctx);
			return false;
		}
		break;
	}

	av_dump_format(*ifmt_ctx, 0, open_device_name.c_str(), 0);
	return true;
}



class F4MVideoCapture{
private:
	AVFormatContext* ifmt_ctx;
	AVCodecContext* dec_ctx;
	FilteringContext flt_ctx;
	AVPacket* read_packet = av_packet_alloc();
	AVFrame* read_frame = av_frame_alloc();
	thread th;
	bool is_active=false;
	kc2::MeasureFPS fps;
	ShareFrame current_frame=nullptr;
	mutex m;
public:
	CaptureProperty prop;

	//‚¨‚à‚¢
	bool open(int index,const CaptureProperty& prop) {
		avdevice_register_all();

		kc2::device_info_list_t* d_list;
		kc2::kc2np_init_device_info_list(&d_list);

		if(index<0 || d_list->size()<=index)return false;
		
		kc2::DeviceInfo d_info = d_list->at(index);

		int v_index = kc2::kc2np_device_info_get_near_size_video_info_index(d_info,prop.width,prop.height);

		if(v_index<0)return false;

		kc2::VideoInfo& v_info = d_info.video_info_list->at(v_index);

		//‚±‚±‚©‚çffmpeg
		bool ok = open_input_video_device(d_info.name,v_info,prop.flag_fps_option,&ifmt_ctx,&dec_ctx);
		if(!ok)return false;

		init_filter_context(dec_ctx,dec_ctx->pix_fmt,AV_PIX_FMT_BGR24,&flt_ctx);

		is_active=true;
		th=thread(&F4MVideoCapture::load,this);

		this->prop.width = dec_ctx->width;
		this->prop.height = dec_ctx->height;
		this->prop.flag_fps_option = prop.flag_fps_option;

		return true;
	}

	void load() {
		while (is_active) {

			int ret = av_read_frame(ifmt_ctx, read_packet);
			if(ret<0)break;
			ret = avcodec_send_packet(dec_ctx, read_packet);
			if(ret<0)break;

			ret = avcodec_receive_frame(dec_ctx,read_frame);
			if (ret<0)break;

			ret = av_buffersrc_add_frame_flags(flt_ctx.buffersrc_ctx,read_frame, 0);
			if (ret < 0)break;

			while (true) {
				AVFrame* frame = av_frame_alloc();
				ret = av_buffersink_get_frame(flt_ctx.buffersink_ctx,frame);
				if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
					break;
				}

				{
					lock_guard lock(m);
					current_frame = frame;
				}

				fps.count();
			}
			av_packet_unref(read_packet);
		}

		//‰ð•ú.
		is_active = false;
		int ret = avcodec_send_packet(dec_ctx,nullptr);
		while(0<=ret){
			ret = avcodec_receive_frame(dec_ctx, read_frame);
			if (ret < 0)break;
			av_frame_unref(read_frame);
		}


		ret = av_buffersrc_add_frame_flags(flt_ctx.buffersrc_ctx, nullptr, 0);
		while (true) {
			ret = av_buffersink_get_frame(flt_ctx.buffersink_ctx, read_frame);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
				break;
				av_frame_unref(read_frame);
			}
		}
		av_packet_unref(read_packet);

		current_frame.try_free();

		av_packet_free(&read_packet);
		av_frame_free(&read_frame);
		avcodec_free_context(&dec_ctx);
		avfilter_graph_free(&flt_ctx.filter_graph);
		avformat_close_input(&ifmt_ctx);

	}

	ShareFrame read() {
		{
			lock_guard lock(m);
			return current_frame;
		}
	}

	//‚¨‚à‚¢‚©‚à
	void release() {
		is_active = false;
		th.join();
	}
	int get_fps() {
		return fps.get_fps();
	}
};



EXPORT F4MVideoCapture* kc2np_get_F4MVideoCapture(int capture_index, CaptureProperty prop) {
	F4MVideoCapture* ans = new F4MVideoCapture();
	if(!ans->open(capture_index,prop))return nullptr;
	return ans;
}

EXPORT FrameInfo kc2np_F4MVideoCapture_get_current_frame(F4MVideoCapture* vc) {
	if (vc == nullptr) {
		return FrameInfo();
	}
	ShareFrame sf = vc->read();
	if(sf.get()==nullptr)return FrameInfo();
	AVFrame* f = sf.get();
	FrameInfo info;
	info.width = f->width;
	info.height = f->height;
	info.stride = f->linesize[0];
	info.buffsize = f->height * f->linesize[0];
	info.ptr = f->data[0];
	return info;
}


EXPORT void kc2np_F4MVideoCapture_release_capture(F4MVideoCapture** vc) {
	if (*vc == nullptr)return;
	(*vc)->release();
	delete* vc;
	*vc = nullptr;
}

EXPORT CaptureProperty kc2np_F4MVideoCapture_get_CaptureProperty(F4MVideoCapture* vc) {

	if (vc == nullptr)return CaptureProperty();
	return vc->prop;
}

EXPORT int kc2np_F4MVideoCapture_get_fps(F4MVideoCapture* vc) {
	if (vc == nullptr)return 0;
	return vc->get_fps();
}


}





EXPORT void test_cap2() {
	kc2::F4MVideoCapture cap;
	int w=320;
	int h=240;
	if(!cap.open(2,{w,h,true})){
		std::cout << "error" << std::endl;
		return;
	}
	kc2::timer time;

	while (time.elapsed() < 5000) {
		auto sh = cap.read();

		AVFrame* frame = sh.get();
		if(frame!=nullptr){
			cv::Mat mat(frame->height,frame->width,CV_8UC3,frame->data[0],frame->linesize[0]);
			cv::imshow("qq",mat);
			//av_frame_free(&frame);
		}
		cv::waitKey(1);
	}
	cap.release();
}

EXPORT void test_cap3() {
	auto cap = kc2::kc2np_get_F4MVideoCapture(0,{600,400,0});
	kc2::timer time;

	while (time.elapsed() < 10000) {
		auto fi = kc2::kc2np_F4MVideoCapture_get_current_frame(cap);

	}
	

}


