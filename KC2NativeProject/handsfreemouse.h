#pragma once
#include"f4mvideocapture.h"
#include"tracker.h"
#include"data_struct.h"
#include"mouse_click.h"
using namespace std;
using namespace cv;

namespace kc2 {
	namespace LowPass {
		Point2d del_lowpass_sensor;
		Point2d del_prev;
		bool is_first = true;
		double k = .9;
		Point2d input(Point2d del_sensor) {
			if (is_first) {
				del_lowpass_sensor = del_sensor;
				is_first = false;
			}
			del_lowpass_sensor = del_sensor * k + del_lowpass_sensor * (1 - k);
			return del_lowpass_sensor;
		}

		void release() {
			is_first = true;
		}
	}

	namespace DetectStay {
		Point2d pos_prev;
		kc2::timer time;
		bool is_first = true;
		bool is_stay = false;
		bool input(Point2d pos,Point2d del_lowpass) {
			double pow2del = del_lowpass.dot(del_lowpass);
			if (4.0 < pow2del) {
				is_first = true;
				is_stay = false;
				return false;
			}

			if(is_first){
				time.start();
				pos_prev = pos;
				is_first = false;
			}

			auto d = pos - pos_prev;
			if (4.0 < d.x * d.x + d.y * d.y) {
				is_first = true;
				is_stay = false;
				return false;
			}

			if (time.elapsed() < 1000) {
				is_stay = false;
				return false;
			}
			is_stay = true;
			return true;
		}

		void release() {
			is_first = true;
		}
	}

	namespace CursorOperator {

		enum KC2_HFM_RANGE {
			KC2_HFM_RANGE_LT = 0,
			KC2_HFM_RANGE_RT,
			KC2_HFM_RANGE_RB,
			KC2_HFM_RANGE_LB
		};

		enum KC2_HFM_AXIS {
			KC2_HFM_AXIS_X=0,
			KC2_HFM_AXIS_Y,
		};

		Point2d pos_cursor_screen{ 0,0 };
		POINT pos_send_screen;
		Point2d range[4];
		Point2d axis[2];
		Point2d uaxis[2];
		double absaxis[2];
		Point2d op;
		Point2d sop;
		RECT screen_rect;
		int flg_range = 0;
		bool is_set_ok = false;
		bool is_first = true;
		int flg_move_xy=0;
		//Point2d del_lowpass_sensor;

		double screen_width;// = screen_rect.right - screen_rect.left;
		double screen_height;// = screen_rect.bottom - screen_rect.top;
		double sensor_width;// = absaxis[0] * 2;
		double sensor_height;// = absaxis[1] * 2;

		Point2d screen_size;
		Point2d sensor_size;

		Point2d screen_from_sensor_k{30,30};

		Point2i cursor_prev_pos_on_screen;
		Point2d stay_cursor_pos_on_screen;

		Point2d vec_to_uvec(Point2d p) {
			double d = sqrt( p.dot(p));
			if(d==0)return{0,0};
			return p / d;
		}

		double vec_to_dis(Point2d p) {
			return sqrt(p.x*p.x+p.y*p.y);
		}
		double vec_to_dis_pow2(Point2d p) {
			return p.x * p.x + p.y * p.y;
		}

		//800x448

		//return true is cursor move , false is cursor stay.
		void input(Point2d pos_sensor, Point2d del_sensor, Point2d acc_sensor, int64_t interval_micro,Mat mat) {
//			set_mouse(500,500);
			
			if (is_first) {
				stay_cursor_pos_on_screen = pos_sensor;
				is_first = false;
			}
			
			auto hdel = pos_sensor - stay_cursor_pos_on_screen;

			auto hk = (abs(hdel.x) + abs(hdel.y)) / (abs(del_sensor.x) + abs(del_sensor.y));

			Point2d del;
			if(flg_move_xy!=0){
				del = del_sensor * 1.5;
			}
			else del = hdel;//del_sensor.dot(hdel) * del_sensor;
			
			del= {del.x * screen_from_sensor_k.x /30 , del.y * screen_from_sensor_k.y/30};

			flg_move_xy=0;
			if (abs(del.x) < .4){
				del.x = 0;
			}
			else {
				flg_move_xy|=1;
				if (del.x < 0) {
					del.x+=.4;
				}
				else {
					del.x-=.4;
				}
			}

			if(abs(del.y)<.4){
				del.y=0;
			}
			else {
				flg_move_xy|=2;
				if (del.y < 0) {
					del.y += .4;
				}
				else {
					del.y -= .4;
				}
			}
			
			if (flg_move_xy) {
				stay_cursor_pos_on_screen.x = pos_sensor.x;
				stay_cursor_pos_on_screen.y = pos_sensor.y;
			}

			del*=8;

			double del_abs = abs(del.x) + abs(del.y);
			double del_sen = abs(del_sensor.x) + abs(del_sensor.y);
			del_sensor*=sqrt( vec_to_dis_pow2(del) / vec_to_dis_pow2(del_sensor) );

			mouse_event(MOUSEEVENTF_MOVE, del.x, del.y, NULL, NULL);
			/*
			//cout << interval << endl;
			double ts = interval_micro / 1e6;

			POINT pos_current_screen;
			GetCursorPos(&pos_current_screen);
			if (pos_send_screen.x != pos_current_screen.x || pos_send_screen.y != pos_current_screen.y) {
			//	pos_cursor_screen.x = pos_current_screen.x;
			//	pos_cursor_screen.y = pos_current_screen.y;
			//	del_lowpass_sensor = del_sensor;
			}

			if (is_first) {
				is_first = false;
				pos_cursor_screen.x = pos_current_screen.x;
				pos_cursor_screen.y = pos_current_screen.y;
			}

			auto pos_cursor_screen_prev = pos_cursor_screen;


			//del_lowpass_sensor = del_lowpass_sensor * .4 + del_sensor * .6;
			//double pow2_del = acc_sensor.x * acc_sensor.x + acc_sensor.y * acc_sensor.y;
			Point2d acc_sensor_ts = acc_sensor / ts;
			Point2d pow2_del ={abs( acc_sensor_ts.x ), abs(acc_sensor_ts.y)};
			double k_del = max(screen_from_screen_k.x, screen_from_screen_k.y)/12.0;//max(1.0, pow2_del.dot(screen_from_screen_k)*.01);//min(10.0, max(sensor_move_threshold_k, pow2_del));

			auto pos_add = del_sensor * k_del;


			if (abs(pos_add.x) < .8) {
				pos_add.x = 0;
			}

			if (abs(pos_add.y) < .8){
				pos_add.y = 0;
			}

			pos_add *= 5;

			pos_add.x = -pos_add.x;

			pos_cursor_screen += pos_add;

			//putText(mat, to_string(k_del), { 0,60 }, 0, 1, { 0,0,255 });
			//putText(mat, to_string(screen_from_screen_k.x), { 0,90 }, 0, 1, { 0,0,255 });
			//putText(mat, to_string(ts), { 0,120 }, 0, 1, { 0,0,255 });



			Point2d pos_del;
			//GetCursorPos(&pos_current_screen);
			//pos_del.x = pos_cursor_screen.x - pos_current_screen.x;
			//pos_del.y = pos_cursor_screen.y - pos_current_screen.y;

			pos_del.x = pos_cursor_screen.x - pos_cursor_screen_prev.x;
			pos_del.y = pos_cursor_screen.y - pos_cursor_screen_prev.y;

			double speed_pow2 = pos_del.dot(pos_del);

			if (is_set_ok) {



				Point2d v1,v2;
				double d1,d2;
				v1 = pos_sensor - op;
				d1 =std::max(-1.0, std::min( 1.0 , v1.dot(uaxis[0]) / absaxis[0] )  );
				d2 =std::max(-1.0, std::min( 1.0 , v1.dot(uaxis[1]) / absaxis[1] )  );

				auto offx = d1 * (screen_rect.right - screen_rect.left) * .5;
				auto offy = d2 * (screen_rect.bottom - screen_rect.top) *.5;

				Point2d sct;
				sct.x = offx + sop.x;
				sct.y = offy + sop.y;
				auto sct_del = sct - pos_cursor_screen_prev;
				//auto udel = vec_to_uvec(pos_del);

				double sct_k =max(1.0, pos_del.dot(sct_del) / pos_del.dot(pos_del));
				sct_del = pos_del * sct_k;
				//double sct_k =min( max(1.0,speed_pow2) , sct_max_k );
				//pos_del =sct_del;//sct_max_k;
//				pos_cursor_screen.x = t.x;
//				pos_cursor_screen.y = t.y;
				
				
				Point2d kxy{ 1 / sensor_width  ,1 / sensor_height };

				auto del_s = del_sensor / ts;
				del_s.x*=kxy.x;
				del_s.y*=kxy.y;

				double del_abs =min(1.0,abs(del_s.x + del_s.y)*.05);
				pos_del = pos_del * (1 - del_abs) + sct_del * del_abs;
				
				putText(mat, to_string(del_abs), { 0,30 }, 0, 1, { 0,0,255 });
				//putText(mat, to_string(del_abs.y), { 0,60 }, 0, 1, { 0,0,255 });

			}
			//imshow("qqqqq", mat);
			//waitKey(1);
			pos_send_screen.x = pos_cursor_screen.x;
			pos_send_screen.y = pos_cursor_screen.y;

		
			pos_cursor_screen.x = pos_cursor_screen_prev.x + pos_del.x;
			pos_cursor_screen.y = pos_cursor_screen_prev.y + pos_del.y;


			

			mouse_event(MOUSEEVENTF_MOVE, pos_del.x, pos_del.y, NULL, NULL);
			*/
		}


		void set_screen_rect(RECT rect) {
			screen_rect = rect;
			sop.x = (screen_rect.right + screen_rect.left) * .5;
			sop.y = (screen_rect.bottom + screen_rect.top) * .5;
		}

		void set_range_point(int index, double x, double y) {
			range[index] = { x,y };
			flg_range |= 1 << index;

			if (flg_range == 0b1111) {
				Point2d v1,v2;
				double d1,d2;
				op = (range[KC2_HFM_RANGE_LB] + range[KC2_HFM_RANGE_LT] + range[KC2_HFM_RANGE_RB] + range[KC2_HFM_RANGE_RT])/4.0;
				
				v1 = range[KC2_HFM_RANGE_RT] - range[KC2_HFM_RANGE_LT];
				v2 = range[KC2_HFM_RANGE_RB] - range[KC2_HFM_RANGE_LB];
				axis[KC2_HFM_AXIS_X] = (v1 + v2) / 4.0;

				v1 = range[KC2_HFM_RANGE_LB] - range[KC2_HFM_RANGE_LT];
				v2 = range[KC2_HFM_RANGE_RB] - range[KC2_HFM_RANGE_RT];
				
				Point2d v3 ={ -axis[KC2_HFM_AXIS_X].y, axis[KC2_HFM_AXIS_X].x};
				d1 = v3.dot(v1+v2);
				if(0<d1)
					axis[KC2_HFM_AXIS_Y] = {-axis[KC2_HFM_AXIS_X].y,axis[KC2_HFM_AXIS_X].x };
				else
					axis[KC2_HFM_AXIS_Y] = { axis[KC2_HFM_AXIS_X].y,-axis[KC2_HFM_AXIS_X].x };

				uaxis[KC2_HFM_AXIS_X] = vec_to_uvec(axis[KC2_HFM_AXIS_X]);
				uaxis[KC2_HFM_AXIS_Y] = vec_to_uvec( axis[KC2_HFM_AXIS_Y]);

				absaxis[KC2_HFM_AXIS_X] = LONG_MAX;
				absaxis[KC2_HFM_AXIS_Y] = LONG_MAX;

				for (int i = 0; i < 4; i++) {
					v1 = range[i] - op;
					d1 = abs( v1.dot(uaxis[0]));
					d2 = abs( v1.dot(uaxis[1]));
					absaxis[0] = min(absaxis[0], d1);
					absaxis[1] = min(absaxis[1], d2);
				}
				

				screen_width = screen_rect.right - screen_rect.left;
				screen_height = screen_rect.bottom - screen_rect.top;
				sensor_width = absaxis[0] * 2;
				sensor_height = absaxis[1] * 2;

				screen_size = {screen_width , screen_height};
				sensor_size = {sensor_width , screen_height};

				screen_from_sensor_k = {screen_width / sensor_width , screen_height / sensor_height};

				is_set_ok = true;
				//MessageBoxW(0, to_wstring(absaxis[0]).c_str(), L"q", 0);
				//MessageBoxW(0,to_wstring(absaxis[1]).c_str(), L"q", 0);
			}

		}



		void clear_range_point() {
			flg_range=0;
			is_set_ok = false;
		}

		void release(bool flg_clear_range = false) {
			is_first = true;
			if (flg_clear_range)clear_range_point();
		}

		void draw_ui(Mat mat) {
			for (int i = 0; i < 4; i++) {
				if (flg_range & (1 << i)) {
					circle(mat, range[i], 5, { 185,218,255 }, -1);
				}
			}

			if (is_set_ok) {
				auto v1 = axis[0];// - op;
				auto v2 = axis[1];// - op;
				line(mat, op - v1, op + v1, { 0,255,0 }, 2);
				line(mat,op - v2,op + v2,{0,255,0},2);
			}

		}

	};

	namespace DetectCursorStay {
		kc2::timer time;
		int64_t limit=500;
		int64_t current=0;
		bool is_first = true;
		POINT pos_prev;
		//return 1 is trigger mouse click
		bool input(bool flg_move_cursor) {
			POINT pos_current_screen;
			GetCursorPos(&pos_current_screen);
			if (pos_current_screen.x != pos_prev.x || pos_current_screen.y != pos_prev.y) {
				flg_move_cursor=true;
			}
			else {
				flg_move_cursor=false;
			}
			pos_prev = pos_current_screen;

			if (flg_move_cursor || is_first) {
				time.start();
				current=0;
				is_first = false;
				return false;
			}
			current = time.elapsed();
			if (current < limit) {
				return false;
			}
			return true;
		}

		double get_progress(int64_t* stay_time) {
			*stay_time = current;
			return min(1.0 , current / (double)limit);
		}

		void release() {
			is_first = true;
			current = 0;
		}

	}


	namespace hfm {
		
		kc2::Tracker* tr;
		bool is_draw_ui = true;
		int angle_cap = 0;
		bool flip_v = 0;
		bool is_avtive = false;
		bool is_found_face = false;
		bool is_setting_range = true;
		bool have_range = false;
		bool is_stay_sensor = false;
		bool is_new_frame = false;
		bool is_first = true;
		bool is_enable_click = true;
		//Mat current_mat;
		//Mat out_mat;
		ShareFrame current_frame=nullptr;
		ShareFrame out_frame = nullptr;
		mutex mtx;
		Point2d point_sensor;
		int range_set_count=0;
		int64_t time_prev=0;
		CursorClickEvent::KC2_MouseEvent flg_mouse_event=CursorClickEvent::KC2_MouseEvent_LeftClick;
		Mat avframe_to_mat(AVFrame* f) {
			return Mat({ f->width,f->height }, CV_8UC3, f->data[0], f->linesize[0]);
		}

		void init(CaptureProperty cap_prop,  int num_track_point, int is_setting_range) {
			hfm::angle_cap = angle_cap;
			hfm::flip_v = flip_v;
			hfm::is_setting_range = is_setting_range;
			int w = cap_prop.get_actual_width();//cap_prop.width;
			int h = cap_prop.get_actual_height();//cap_prop.height;
			//out_frame.try_free();
			//out_mat.release();
			current_frame.try_free();
			//current_mat.release();
			have_range = false;
			tr = new kc2::Tracker(w, h, num_track_point);
			hfm::is_avtive = true;
			is_new_frame = false;
			is_first = true;
			if(is_setting_range)
				CursorOperator::clear_range_point();

		}


		void input(ShareFrame& frame) {
			AVFrame* f = frame.get();
			if(f==nullptr)return;
			current_frame = frame;
			is_new_frame = true;

			if (!hfm::is_avtive) {
				return;
			}

			int64_t interval;
			if (is_first) {
				is_first = false;
				time_prev = frame.get()->best_effort_timestamp;
			}
			interval = frame.get()->best_effort_timestamp - time_prev;
			time_prev = frame.get()->best_effort_timestamp;

			Mat mat({ f->width,f->height }, CV_8UC3, f->data[0], f->linesize[0]);

			{
				lock_guard lock(mtx);

				if(tr==nullptr)return;
				
				if(tr->input(mat)){
					auto low_pass_delta = LowPass::input(tr->get_delta());

					is_stay_sensor = DetectStay::input(tr->get_point(),low_pass_delta);


					if (!hfm::is_setting_range) {
						CursorOperator::input(tr->get_point(), low_pass_delta, tr->get_acceleration(), interval, mat);
						bool click = DetectCursorStay::input(CursorOperator::flg_move_xy);
						if (is_enable_click) {
							CursorClickEvent::input(flg_mouse_event,click);
						}
					}

					point_sensor = tr->get_point();
				}
				else {
					LowPass::release();
				}
				//cv::blur(mat, mat, { 100,100 });

				//current_mat = mat;
			}

		}

		ShareFrame get_current_frame() {

			{
				lock_guard lock(mtx);

				if(current_frame.get()==nullptr || !is_new_frame)return nullptr;
				out_frame = current_frame;
				Mat mat = avframe_to_mat(out_frame.get());

			//	cv::blur(mat, mat, { 100,100 });


				if(hfm::is_avtive){
				//out_mat = current_mat;//clone()‚ÍÁ‚µ‚Ä‚¢‚¢‚©‚à.
					if (tr != nullptr) {
						tr->draw_ui(mat);
					}

					CursorOperator::draw_ui(mat);

				}
				is_new_frame = false;
			}

			return out_frame;//.get();

		}

		bool detect_stay_sensor() {
			return is_stay_sensor;
		}

		Point2d get_point_sensor() {
			return point_sensor;
		}



		void release() {
			hfm::is_avtive = false;
			{
				lock_guard lock(mtx);
				if(tr!=nullptr){
					tr->release();
					delete tr;
					tr = nullptr;
				}
				is_first = true;
				LowPass::release();
				DetectStay::release();
				DetectCursorStay::release();
				CursorOperator::release();
				CursorClickEvent::release();
				out_frame.try_free();
				current_frame.try_free();
			}
			have_range = false;
			//flg_range = 0;
		}

		EXPORT int kc2np_hansfreemouse_detect_stay_sensor() {
			if(!hfm::is_avtive)return 0;
			return hfm::detect_stay_sensor();
		}

		EXPORT void kc2np_hansfreemouse_get_point_sensor(double* x,double* y) {
			if (!hfm::is_avtive) {
				*x=0;
				*y=0;
			}
			else {
				auto p = hfm::get_point_sensor();
				*x = p.x;
				*y = p.y;
			}
		}

		EXPORT void kc2np_hansfreemouse_set_range_point(int index,double x,double y) {
			if (!hfm::is_avtive)return;
			return CursorOperator::set_range_point(index,x,y);
		}

		EXPORT void kc2np_hansfreemouse_set_screen_rect(RECT rect) {
	//		if (!hfm::is_avtive)return;
			return CursorOperator::set_screen_rect (rect);
		}

		EXPORT void test_hfm() {
		//	CursorOperator::set_mouse(100,100);
		}

		EXPORT double kc2np_hansfreemouse_get_cursor_click_progress(int64_t* stay_time) {
			return DetectCursorStay::get_progress(stay_time);
		}

		EXPORT void kc2np_hansfreemouse_set_enable_click(int enable_click) {
			hfm::is_enable_click = enable_click;
		}
		EXPORT int kc2np_hansfreemouse_get_enable_click() {
			return hfm::is_enable_click;
		}

		EXPORT void kc2np_hansfreemouse_set_mouse_click_event(int click_event) {
			hfm::flg_mouse_event = (CursorClickEvent::KC2_MouseEvent)click_event;
		}
	}
}

