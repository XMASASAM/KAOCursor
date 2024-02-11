#pragma once
#include"ffmpeg.h"
#include<unordered_map>
#include<mutex>

namespace kc2 {

	class ShareFrame {
	private:
		static std::unordered_map<AVFrame**, int> mp;
		static std::mutex m;
		AVFrame** fp = nullptr;
	public:
		AVFrame* get() {
			if (fp == nullptr || *fp == nullptr)return nullptr;
			return *fp;
		}

		void try_free() {

			if (fp == nullptr)return;

			{
				std::lock_guard lock(m);
				mp[fp]--;
				//printf("pop: fp:%ld count:%d\n", fp, mp[fp]);

				if (mp[fp] <= 0) {

					mp.erase(fp);
					if (*fp != nullptr) {
						av_frame_free(fp);
						delete fp;
						*fp = nullptr;
					}
				}
				fp = nullptr;
			}
		}

		void set_frame(AVFrame** new_frame_ptr) {
			if (new_frame_ptr == nullptr || *new_frame_ptr == nullptr)return;

			{
				std::lock_guard lock(m);
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

	std::unordered_map<AVFrame**, int> ShareFrame::mp;
	std::mutex ShareFrame::m;
}