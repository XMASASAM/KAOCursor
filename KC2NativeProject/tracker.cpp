#include"tracker.h"
namespace kc2 {

	cv::Rect rect_resize(cv::Rect rect, float s) {
		int cx = rect.x + (rect.width) / 2;
		int cy = rect.y + (rect.height) / 2;
		int w = rect.width * s;
		int h = rect.height * s;
		int x = cx - w / 2;
		int y = cy - h / 2;
		return { x,y,w,h };
	}

	namespace Tracker {
		bool is_first = true;
		bool flg_blob_tracker=true;//blobtrackerƒAƒ‹ƒSƒŠƒYƒ€‚ð—˜—p‚·‚é‚©.
		bool is_mono_prev = false;

		Tracker_Base* tr = nullptr;

		int capture_width = 1;
		int capture_height = 1;
		int num_opt_point=1;
		double div_count_detect_mono=0;
		bool is_enable_inff=false;
		void init(int cap_width, int cap_height, int max_p) {
			capture_width = cap_width;
			capture_height = cap_height;
			num_opt_point = max_p;

			is_first = true;
			is_mono_prev = false;
			div_count_detect_mono = 1.0 / (ceil((cap_width/10.0)) * ceil((capture_height/10.0)));
		}
		bool input(Mat mat, int64_t interval) {
			bool is_mono; 
			if(mat.rows!=capture_height || mat.cols!=capture_width){
				is_first = true;
				return false;
			}
			if ((50000 < interval || is_first ) && flg_blob_tracker) {
				is_mono = detect_mono(mat, interval);
			}
			else {
				is_mono = is_mono_prev;
			}
	//		is_mono = false;
			
			if (is_first) {
				is_mono_prev = !is_mono;
				is_first = false;
			}
			
			if (is_mono_prev != is_mono) {
				if (tr != nullptr) {
					delete tr;
				}
			
				if (is_mono) {
					cout << "change_tacker_blob" << endl;
					tr = new Tracker_Blob();
				}
				else {
					cout << "change_tacker_face" << endl;
					tr = new Tracker_Face(capture_width,capture_height,num_opt_point);
				}
			}

			is_mono_prev = is_mono;
			return tr->input(mat);

		}

		Point2d get_point() {
			if (tr == nullptr)return { 0,0 };
			return tr->get_point();
		}

		Point2d get_delta() {
			if (tr == nullptr)return { 0,0 };
			return tr->get_delta();

		}

		bool is_active() {
			return tr != nullptr;
		}

		void draw_ui(Mat mat) {
			if (tr == nullptr)return;
			tr->draw_ui(mat);
		}

		void release() {
			is_first = true;
			if (tr != nullptr) {
				tr->release();
				delete tr;
				tr = nullptr;
			}
		}

		bool detect_mono(Mat mat, int64_t interval) {
			cout << "detect_blob" << endl;
			double v=0;
			for (int y = 0; y < mat.rows; y+=10) {
				for (int x = 0; x < mat.cols; x+=10) {
					int b = mat.data[y * mat.step + x*3    ];
					int g = mat.data[y * mat.step + x*3 + 1];
					int r = mat.data[y * mat.step + x*3 + 2];
					double m = (b+g+r)/3;
					v+=(b*b + g*g + r*r)/3 - m*m;
				//	count++;
				}
			}
			v*=div_count_detect_mono;
			cout << "bunsan::" << v << endl;
			return v <= 1;
		}
		void set_enable_inff(bool flg) {
			flg_blob_tracker = flg;
		}


	}
}