#pragma once
#include"ffmpeg.h"
#include<unordered_map>
#include<mutex>

namespace kc2 {

	class ShareFrame {
	private:
		static std::unordered_map<AVFrame*, int> mp;
		static std::mutex m;
		AVFrame* fp = nullptr;
	public:
		AVFrame* get();

		void try_free();

		void set_frame(AVFrame* new_frame_ptr);

		ShareFrame(AVFrame* a);

		ShareFrame(ShareFrame& a);

		ShareFrame(const ShareFrame& a) noexcept;

		~ShareFrame();

		ShareFrame& operator=(ShareFrame&& a)noexcept;

		ShareFrame& operator=(ShareFrame& a);

	};

	//std::unordered_map<AVFrame**, int> ShareFrame::mp;
	//std::mutex ShareFrame::m;
}