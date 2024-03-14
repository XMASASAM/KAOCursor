#include"kc2.h"
#include<opencv2/opencv.hpp>
#include<thread>
#include<mutex>
#include"device.h"

using namespace cv;
using namespace std;

string fourcc_to_string(int f) {
	string a="0000";
	a[0] = f&255;
	a[1] = (f >> 8) & 255;
	a[2] = (f >> 16) & 255;
	a[3] = (f >> 24) & 255;
	return a;
}

class KC2VideoCapture {
private:
	VideoCapture cap;
	Mat current_frame;
	Mat out_cc_frame;
	thread th_cap;
	mutex mtx;
	bool is_active_capture = false;
	bool is_new_frame = false;
	int width;
	int height;
	kc2::MeasureFPS mfps;
	int pre_fourcc;
	int now_fourcc;
public:
	KC2VideoCapture(int capture_index,const CaptureProperty& prop) {
		vector<int> params;
		kc2::device_info_list_t* dlist;
		kc2::kc2np_init_device_info_list(&dlist);
		int vindex = kc2::kc2np_device_info_get_near_size_video_info_index(dlist->at(capture_index),prop.width,prop.height);
		if(vindex<0)return;

		kc2::VideoInfo vinfo = dlist->at(capture_index).video_info_list->at(vindex);
//		cap.open(capture_index, );
//		while(!cap.isOpened());
		if (0 < vinfo.width) {
			params.push_back(CAP_PROP_FRAME_WIDTH);
			params.push_back(vinfo.width);
		}
		if (0 < vinfo.height) {
			params.push_back(CAP_PROP_FRAME_HEIGHT);
			params.push_back(vinfo.height);
		}

		if (vinfo.format) {

			//now_fourcc = VideoWriter::fourcc('M', 'J', 'P', 'G');
			//cap.set(CAP_PROP_FOURCC,now_fourcc );// );
			//params.push_back(CAP_PROP_FOURCC);
			//params.push_back(VideoWriter::fourcc('m', 'j', 'p', 'g'));
			//params.push_back(CAP_PROP_CONVERT_RGB);
			//params.push_back(1);

			//params.push_back(VideoWriter::fourcc('M', 'J', 'P', 'G'));
			//params.push_back(VideoWriter::fourcc('m', 'j', 'p', 'g'));

		}


		cap.open(capture_index,CAP_ANY,params);
		width = cap.get(CAP_PROP_FRAME_WIDTH);
		height = cap.get(CAP_PROP_FRAME_HEIGHT);


		//cout << "fourcc:::: " << fourcc_to_string(pre_fourcc) << endl;
		//MessageBoxW(0, to_wstring(pre_fourcc).c_str(), L"pre_fourcc", S_OK);
		//MessageBoxW(0,to_wstring(VideoWriter::fourcc('Y', 'U', 'Y', 'V')).c_str(), L"yuyv", S_OK);
		
	}

	void start_capture() {
		is_active_capture = true;
		th_cap = thread(&KC2VideoCapture::update_capture,this);
		//th_cap.detach();
		
	}

	void update_capture() {
		//std::this_thread::sleep_for(std::chrono::seconds(1));
		while (is_active_capture) {
			Mat mat;
			bool ok = cap.read(mat);

			if (ok) {
			//	cout << "OK" << endl;
			}
			else {
				MessageBoxW(0,L"qq",L"Comped",S_OK);

			}

			{
				lock_guard<mutex> lock(mtx);
				current_frame = mat;
				is_new_frame = true;
			}
			//imshow("qq",mat);
			//waitKey(1);

			mfps.count();
		}

		if (pre_fourcc != now_fourcc) {
			//cap.set(CAP_PROP_FOURCC,pre_fourcc);
		}
		//waitKey(1);
		cap.release();
		//cap.~VideoCapture();
		//waitKey(1);
	//	MessageBoxW(0,L"qq",L"Comped",S_OK);

	}

	void release_capture() {
		is_active_capture = false;
		th_cap.join();
		cout << "closed!!!!" << endl;
		//MessageBoxW(0, L"qqOOOOO", L"Comped", S_OK);

	}

	FrameInfo get_current_frame() {
		FrameInfo info;
		if (!is_new_frame) {
			info.ptr = 0;
			return info;
		}
		is_new_frame = false;
		{
			lock_guard<mutex> lock(mtx);
			out_cc_frame = current_frame;
		}
		info.width = out_cc_frame.cols;
		info.height = out_cc_frame.rows;
		info.buffsize = out_cc_frame.total() * out_cc_frame.elemSize();
		info.stride = out_cc_frame.step;
		info.ptr = out_cc_frame.ptr();
		return info;
	}

	int get_width() {
		return width;
	}
	int get_height() {
		return height;
	}
	int get_fps() {
		return mfps.get_fps();
	}
	bool is_opened() {
		return cap.isOpened();
	}
};



EXPORT KC2VideoCapture* kc2np_get_KC2VideoCapture(int capture_index , CaptureProperty prop) {
	KC2VideoCapture* ans = new KC2VideoCapture(capture_index,prop);
	return ans;
}

EXPORT FrameInfo kc2np_KC2VideoCapture_get_current_frame(KC2VideoCapture* vc) {
	if (vc == nullptr) {
		return FrameInfo();
	}
	return vc->get_current_frame();
}

EXPORT void kc2np_KC2VideoCapture_start_capture(KC2VideoCapture* vc) {
	if(vc==nullptr)return;
	vc->start_capture();
}

EXPORT void kc2np_KC2VideoCapture_release_capture(KC2VideoCapture** vc) {
	if(*vc==nullptr)return;
	(*vc)->release_capture();
	delete *vc;
	*vc=nullptr;
}

EXPORT FrameInfo kc2np_KC2VideoCapture_get_frame_info(KC2VideoCapture* vc) {

	FrameInfo info;
	if(vc==nullptr)return info;
	info.width = vc->get_width();
	info.height = vc->get_height();
	return info;
}

EXPORT int kc2np_KC2VideoCapture_get_fps(KC2VideoCapture* vc) {
	if(vc==nullptr)return 0;
	return vc->get_fps();
}

EXPORT int kc2np_KC2VideoCapture_get_is_opened(KC2VideoCapture* vc) {
	if(vc==nullptr)return 0;
	return vc->is_opened();
}

EXPORT void test_capture() {
	/*CaptureProperty prop;
	prop.width = 6400;
	prop.height= 4800;
	KC2VideoCapture cap(0,prop);
	cap.start_capture();
	std::this_thread::sleep_for(std::chrono::seconds(10));
	//cap.get_current_frame();
	cap.release_capture();*/
	VideoCapture cap(1);
	Mat mat;
	if (!cap.isOpened()) {
		cout << 22 << endl;
	}
	while (1) {
		cap.read(mat);
		imshow("qq",mat);
		waitKey(1);
	}
}