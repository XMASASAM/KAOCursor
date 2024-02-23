#include"f4mvideocapture.h"
#include<opencv2/opencv.hpp>
#include"handsfreemouse.h"
namespace kc2 {
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


Mat mat2;


FrameInfo mat2frameinfo(Mat mat) {
	FrameInfo info;
	info.width = mat.cols;
	info.height = mat.rows;
	info.stride = mat.step;
	info.buffsize = mat.total() * mat.elemSize();
	info.ptr = mat.ptr();
	return info;
}

void init_filter_context(AVCodecContext* dec_ctx,int angle,bool hf, AVPixelFormat output_pix_fmt, FilteringContext* filter_ctx) {
//	if (dec_ctx->codec_type != AVMEDIA_TYPE_VIDEO)return 0;
	char args[512];
	int ret = 0;
	const AVFilter* buffersrc = avfilter_get_by_name("buffer");
	const AVFilter* buffersink = avfilter_get_by_name("buffersink");
	const AVFilter* transpose = avfilter_get_by_name("transpose");
	const AVFilter* hflip = avfilter_get_by_name("hflip");
	const AVFilter* vflip = avfilter_get_by_name("vflip");
	AVFilterContext* buffersrc_ctx = NULL;
	AVFilterContext* buffersink_ctx = NULL;
	AVFilterContext* transpose_ctx = NULL;
	AVFilterContext* hflip_ctx = NULL;
	AVFilterContext* vflip_ctx = NULL;
	AVFilterInOut* outputs = avfilter_inout_alloc();
	AVFilterInOut* inputs = avfilter_inout_alloc();
	AVFilterGraph* filter_graph = avfilter_graph_alloc();


	snprintf(args, sizeof(args),
		"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
		dec_ctx->width, dec_ctx->height, dec_ctx->pix_fmt,
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

	
	if (angle == 0 && !hf) {
		avfilter_graph_parse_ptr(filter_graph, "null", &inputs, &outputs, NULL);
	}

	if (angle == 90 && !hf) {
		avfilter_graph_create_filter(&transpose_ctx, transpose, "f1", "dir=clock", NULL, filter_graph);
		avfilter_link(buffersrc_ctx, 0, transpose_ctx, 0);
		avfilter_link(transpose_ctx,0,buffersink_ctx,0);
	}

	if (angle == 180 && !hf) {
		avfilter_graph_create_filter(&vflip_ctx, vflip, "f1", NULL, NULL, filter_graph);
		avfilter_graph_create_filter(&hflip_ctx, hflip, "f2", NULL, NULL, filter_graph);
		avfilter_link(buffersrc_ctx, 0, vflip_ctx, 0);
		avfilter_link(vflip_ctx, 0, hflip_ctx, 0);
		avfilter_link(hflip_ctx, 0, buffersink_ctx, 0);
	}

	if (angle == 270 && !hf) {
		avfilter_graph_create_filter(&transpose_ctx, transpose, "f1", "dir=cclock", NULL, filter_graph);
		avfilter_link(buffersrc_ctx, 0, transpose_ctx, 0);
		avfilter_link(transpose_ctx,0,buffersink_ctx,0);
	}


	if (angle == 0 && hf) {
		avfilter_graph_create_filter(&hflip_ctx, hflip, "f1", NULL, NULL, filter_graph);
		avfilter_link(buffersrc_ctx, 0, hflip_ctx, 0);
		avfilter_link(hflip_ctx, 0, buffersink_ctx, 0);
	}

	if (angle == 90 && hf) {
		avfilter_graph_create_filter(&transpose_ctx, transpose, "f1", "dir=clock_flip", NULL, filter_graph);
		avfilter_link(buffersrc_ctx, 0, transpose_ctx, 0);
		avfilter_link(transpose_ctx, 0, buffersink_ctx, 0);
	}

	if (angle == 180 && hf) {
		avfilter_graph_create_filter(&vflip_ctx, vflip, "f1", NULL, NULL, filter_graph);
		avfilter_link(buffersrc_ctx, 0, vflip_ctx, 0);
		avfilter_link(vflip_ctx, 0, buffersink_ctx, 0);
	}

	if (angle == 270 && hf) {
		avfilter_graph_create_filter(&transpose_ctx, transpose, "f1", "dir=cclock_flip", NULL, filter_graph);
		avfilter_link(buffersrc_ctx, 0, transpose_ctx, 0);
		avfilter_link(transpose_ctx, 0, buffersink_ctx, 0);
	}


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




	//‚¨‚à‚¢
	bool kc2::F4MVideoCapture::open(int index,const CaptureProperty& prop) {
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

		init_filter_context(dec_ctx,prop.angle,prop.hflip,AV_PIX_FMT_BGR24,&flt_ctx);

		is_active=true;
		th=thread(&F4MVideoCapture::load,this);

		this->prop.width = dec_ctx->width;
		this->prop.height = dec_ctx->height;
		this->prop.flag_fps_option = prop.flag_fps_option;
		this->prop.angle = prop.angle;
		this->prop.hflip = prop.hflip;



		return true;
	}

	void kc2::F4MVideoCapture::load() {
		while (is_active) {
			
//			if (is_change_filter && !is_out_fream_fresh) {
				if (is_change_filter){ //&& !is_out_fream_fresh) {
				{
					lock_guard lock(m);
					//current_frame.try_free();
					avfilter_graph_free(&flt_ctx.filter_graph);
					init_filter_context(dec_ctx,prop.angle,prop.hflip,AV_PIX_FMT_BGR24,&flt_ctx);
					is_change_filter = false;
				}
			
			}


			int ret = av_read_frame(ifmt_ctx, read_packet);
			if(ret<0)break;
			ret = avcodec_send_packet(dec_ctx, read_packet);
			if(ret<0)break;

			ret = avcodec_receive_frame(dec_ctx,read_frame);
			if (ret<0)break;

			ret = av_buffersrc_add_frame_flags(flt_ctx.buffersrc_ctx,read_frame, 0);
			if (ret < 0)break;


			{
				lock_guard lock(m);
				bool ok = false;
				ShareFrame sf = nullptr;
				while (true) {
					AVFrame* frame = av_frame_alloc();
					ret = av_buffersink_get_frame(flt_ctx.buffersink_ctx,frame);
					if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
						break;
					}

					if(first_time==-1)first_time = frame->best_effort_timestamp;
					auto t = frame->best_effort_timestamp - first_time;
					frame->time_base = { 1,1000000 };
					frame->best_effort_timestamp = av_rescale_q(t,dec_ctx->pkt_timebase,frame->time_base);
					sf = frame;
	//				current_frame.push(frame);
					fps.count();
					ok = true;
				}

				if (ok) {
					is_new_frame = true;
					hfm::input(sf);
					//current_frame.push(sf);
				}

			}
			av_packet_unref(read_packet);

			
			//if (hfm::is_avtive) {
			//}
			//else {
			//	AVFrame* f = current_frame.get();
			//	if (f != nullptr) {
					//Mat mat({ f->width,f->height }, CV_8UC3, f->data[0], f->linesize[0]);
					//cv::blur(mat, mat2, { 100,100 });
			//	}

			//}

		}

		//‰ð•ú.
		is_active = false;
		int ret = avcodec_send_packet(dec_ctx,nullptr);
		while (0 <= ret) {
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

//		current_frame.try_free();must do it

		av_packet_free(&read_packet);
		av_frame_free(&read_frame);
		avcodec_free_context(&dec_ctx);
		avfilter_graph_free(&flt_ctx.filter_graph);
		avformat_close_input(&ifmt_ctx);

	}

	ShareFrame kc2::F4MVideoCapture::read() {
		//if(!is_new_frame)return nullptr;

		{
			lock_guard lock(m);
			//if (is_change_filter || is_out_fream_fresh || !is_new_frame)return nullptr;
			//if (!is_new_frame)return nullptr;
//			is_new_frame = false;
//			is_new_frame = false;
			//auto sf = current_frame.front();
			//current_frame.pop();
			return hfm::get_current_frame();//current_frame;
		}
	}

	//‚¨‚à‚¢‚©‚à
	void kc2::F4MVideoCapture::release() {
		is_active = false;
		th.join();
		hfm::release();
	}
	int kc2::F4MVideoCapture::get_fps() {
		return fps.get_fps();
	}

	void kc2::F4MVideoCapture::change_flip(int angle, bool hflip) {
		if(!is_active)return;

		if (prop.angle != angle || prop.hflip != hflip) {
			prop.angle = angle;
			prop.hflip = hflip;
			{
				lock_guard lock(m);
				is_out_fream_fresh = true;
				is_change_filter = true;
			}

		}


	}



EXPORT F4MVideoCapture* kc2np_get_F4MVideoCapture(int capture_index, CaptureProperty prop) {
	F4MVideoCapture* ans = new F4MVideoCapture();
	if(!ans->open(capture_index,prop))return nullptr;
	return ans;
}

EXPORT FrameInfo kc2np_F4MVideoCapture_get_current_frame(F4MVideoCapture* vc,int is_draw_hfm_ui) {
	if (vc == nullptr) {
		return FrameInfo();
	}
	ShareFrame sf = nullptr;
	//if(hfm::is_avtive && is_draw_hfm_ui){
	//	return hfm::get_current_frame();
	//}else
	{
		FrameInfo info{0,0,0,0,0};
		sf = vc->read();
		
		if(sf.get()==nullptr){
			//vc->is_out_fream_fresh = false;
			return info;
		}
		AVFrame* f = sf.get();
		//Mat mat2;
		info.width = f->width;
		info.height = f->height;
		info.stride = f->linesize[0];
		info.buffsize = f->height * f->linesize[0];
		info.ptr = f->data[0];
		
		//Mat mat = hfm::avframe_to_mat(f);
		//imshow("www", mat);

		//info.ptr = mat2.data;
		return info;
		//return mat2frameinfo(mat2);//info;
	}
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

EXPORT int kc2np_F4MVideoCapture_start_hansfreemouse(F4MVideoCapture* vc,int track_points,int is_setting_range) {
	if (vc == nullptr)return 0;
	
	hfm::init(vc->prop,track_points, is_setting_range);
	return 1;
}

EXPORT void kc2np_F4MVideoCapture_release_hansfreemouse() {
	if(!hfm::is_avtive)return;
	hfm::release();
}

EXPORT void kc2np_F4MVideoCapture_change_flip(F4MVideoCapture* vc, int angle,int hflip) {
	if (vc==nullptr)return;
	vc->change_flip(angle,hflip);
}

}





EXPORT void test_cap2() {
	kc2::F4MVideoCapture cap;
	int w=640;
	int h=480;
	if(!cap.open(0,{w,h,true})){
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
			cout << frame->best_effort_timestamp << endl;
			//av_frame_free(&frame);
		}
		cv::waitKey(1);
	}
	cv::destroyAllWindows();
	cap.release();
}

EXPORT void test_cap3() {
	auto cap = kc2::kc2np_get_F4MVideoCapture(0,{600,400,0});
	kc2::timer time;

	while (time.elapsed() < 10000) {
		auto fi = kc2::kc2np_F4MVideoCapture_get_current_frame(cap,1);

	}
	

}


