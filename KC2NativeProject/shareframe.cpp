#include"shareframe.h"

using namespace std;

		AVFrame* kc2::ShareFrame::get() {
			if (fp == nullptr)return nullptr;
			return fp;
		}

		void kc2::ShareFrame::try_free() {


			{
				std::lock_guard lock(m);
				if (fp == nullptr)return;

				mp[fp]--;
				//printf("pop: fp:%ld count:%d\n", fp, mp[fp]);

				if (mp[fp] <= 0) {

					mp.erase(fp);
					if (fp != nullptr) {
						av_frame_free(&fp);
						//delete fp;
						//*fp = nullptr;
					}
				}
				fp = nullptr;
			}
		}

		void kc2::ShareFrame::set_frame(AVFrame* new_frame_ptr) {

			{
				std::lock_guard lock(m);
				if (new_frame_ptr == nullptr)return;

				fp = new_frame_ptr;
				mp[fp]++;
				//printf("set: fp:%ld count:%d\n",fp,mp[fp]);
			}
		}

		kc2::ShareFrame::ShareFrame(AVFrame* a) {
			try_free();
			//auto f = new AVFrame*;
			//*f = a;
			set_frame(a);
		}

		kc2::ShareFrame::ShareFrame(ShareFrame& a) {
			try_free();
			set_frame(a.fp);
		}

		kc2::ShareFrame::ShareFrame(const ShareFrame& a)noexcept {
			try_free();
			set_frame(a.fp);
		}



		kc2::ShareFrame::~ShareFrame() {
			try_free();
		}

		kc2::ShareFrame& kc2::ShareFrame::operator=(kc2::ShareFrame&& a)noexcept {
			try_free();
			set_frame(a.fp);
			//a.try_free();
			return *this;
		}


		kc2::ShareFrame& kc2::ShareFrame::operator=(kc2::ShareFrame& a) {
			try_free();
			set_frame(a.fp);
			return *this;
		}

	

	std::unordered_map<AVFrame*, int> kc2::ShareFrame::mp;
	std::mutex kc2::ShareFrame::m;
