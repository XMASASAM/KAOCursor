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
		Point2d input(Point2d del_sensor) {
			if (is_first) {
				del_lowpass_sensor = del_sensor;
				is_first = false;
			}
			double k = hfm_prop.LowPathRate;

			del_lowpass_sensor = del_sensor * (1-k) + del_lowpass_sensor * k;
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
			double th = hfm_prop.DetectStayPixelThresholdPow2;
			if (th < pow2del) {
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
			if (th < d.x * d.x + d.y * d.y) {
				is_first = true;
				is_stay = false;
				return false;
			}

			if (time.elapsed() < hfm_prop.DetectStayMillisecondTimeThreshold) {
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

	namespace DetectRampaging {
		timer time;
		int samples=0;
		int samples_move=0;
		double sum=0;
		bool is_first=true;
		Point2d del_prev;
		Point2d lowp;
		void release() {
			samples = 0;
			sum = 0;
			samples_move = 0;
			is_first = true;
		}

		bool input(Point2d d,double max_size) {
		return false;
			if (is_first) {
				release();
				is_first=false;
				del_prev = d;
				lowp=d;
				time.start();
			}

			if (!DetectStay::is_stay) {
				is_first=true;
				return false;
			}

			/*
			samples++;
			auto dis = d.x * d.x + d.y * d.y;;
			cout << "dis::" << dis << endl;
			sum+=dis;
			samples_move+=1<dis;

			auto t = sum / samples;
			cout << "rampa::" << t << endl;
			cout << "sm/s::" <<  samples_move << "/" << samples <<endl;
			cout << "threshold::" << threshold << endl;*/
			lowp = lowp*.5 + d*.5;
			auto ddd = d - lowp;
			auto dis = ddd.x * ddd.x + ddd.y * ddd.y;
			sum+= dis;

			del_prev = d;
			cout << "ddd::" << dis << endl;

			samples++;
			samples_move+= 10 < dis;

			if (time.elapsed() < 1000) {
				return false;
			}

			auto t = sum / samples;
			cout << "t::" << t << endl;
			cout << "sm/s::" << samples_move << "/" << samples << endl;

			if ( t<=100 && samples > samples_move*2 ) {
				is_first = true;
				return false;
			}
			is_first = true;

			return true;

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

		Point2d cursor_lowpass_filter;
		Point2d cursor_highpass_filter;
		Point2d del_prev;
		CountPS count_highpass;
		bool is_rampaging=false;
		double c_sin;
		double c_cos;
		bool is_flip=false;
		bool is_enable_rotate=true;
	//	bool is_first=true;
		Point2d pos_on_screen{0,0};
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
		void input(Point2d pos_sensor, Point2d del_sensor, int64_t interval_micro,Mat mat) {
			if (is_first) {
				POINT p;
				GetCursorPos(&p);
				pos_on_screen = { (double)p.x,(double)p.y };
				is_first = false;
			}

			auto del = del_sensor;
			if (is_enable_rotate) {
				auto temp = del;
				del.x = temp.x * c_cos - temp.y * c_sin;
				del.y = temp.x * c_sin + temp.y * c_cos;

			}
			if (is_flip)del.x = -del.x;

			auto del_r = del;



			auto del_s = del_sensor * (1000000.0 / interval_micro);
			double minsize = (sensor_width + sensor_height)*.5;
			del_s.x *= 1.0 / minsize;
			del_s.y *= 1.0 / minsize;
			del_s.x = abs(del_s.x);
			del_s.y = abs(del_s.y);
			Point2d del_k = { max(.0, min(1.0, abs(del_s.x)))  ,max(.0 , min(1.0, abs(del_s.y))) };
		//	Point2d del_k2 = {1,1};//{max(.5, min(1.0, abs(del_s.x)))  ,max(.5 , min(1.0, abs(del_s.y)))};
			if (del_s.x < 1) {
			//	del.x*=del_s.x;
			}
			if (del_s.y < 1) {
			//	del.y*=del_s.y;
			}
			
			cout << "del_k::" << del_k << endl;

			if (vec_to_dis_pow2(del_k) < 0.02) {
				del_k.x=0;//del.x = del.x* .1;// + del_prev.x * (1.0 - .6);
				del_k.y=0;//del.y = del.y* .1;// + del_prev.y * (1.0 - .6);
				cout << "del_zero2" << endl;
			}

			del.x = del.x * del_k.x + del_prev.x * (1.0 - del_k.x);
			del.y = del.y * del_k.y + del_prev.y * (1.0 - del_k.y);
			del_prev = del;




			del = { 
				del.x * screen_from_sensor_k.x * del_k.x, 
				del.y * screen_from_sensor_k.y * del_k.y 
			};
			cout << "del::" << del << endl;
			//if (abs(del.x) < .6)del.x = 0;
			//if(abs(del.y)<.6)del.y=0;

			//del = { del.x * screen_from_sensor_k.x, del.y * screen_from_sensor_k.y};

//			del.x = del_sensor.x * screen_from_sensor_k.x;//del.x * del_k.x + del_prev.x * (1.0 - del_k.x);
	//		del.y = del_sensor.y * screen_from_sensor_k.y;//del.y * del_k.y + del_prev.y * (1.0 - del_k.y);
		//	del_prev = del;
			

			del *= hfm_prop.CursorMoveMultiplier;
			del.x *= hfm_prop.CursorMoveXFactor;
			del.y *= hfm_prop.CursorMoveYFactor;

			del = del_r * sqrt(vec_to_dis_pow2(del) / vec_to_dis_pow2(del_sensor));


			//SetCursorPos(0,0);
			pos_on_screen+=del;
			pos_on_screen.x = max(0.0, min(pos_on_screen.x, screen_width-1));
			pos_on_screen.y = max(0.0,min(pos_on_screen.y , screen_height-1));
			cout << "del::" << del << endl;




			SetCursorPos(pos_on_screen.x,pos_on_screen.y);
		
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
				uaxis[KC2_HFM_AXIS_X] = vec_to_uvec(axis[KC2_HFM_AXIS_X]);
				auto ac = uaxis[KC2_HFM_AXIS_X].dot({1,0});
				auto dc = 180 * acos(ac) / 3.14;
				cout << "cos::" << dc << endl;
				
				v1 = range[KC2_HFM_RANGE_LB] - range[KC2_HFM_RANGE_LT];
				v2 = range[KC2_HFM_RANGE_RB] - range[KC2_HFM_RANGE_RT];
				
				axis[KC2_HFM_AXIS_Y] = (v1 + v2) / 4.0;

				/*
				cout << "si2::" << ds << endl;

				cout << "x:" << axis[KC2_HFM_AXIS_X] << endl;
				cout << "y:" << axis[KC2_HFM_AXIS_Y] << endl;*/

				if (axis[KC2_HFM_AXIS_X].cross(axis[KC2_HFM_AXIS_Y])<0) {
					//”½“]‚µ‚Ä‚¢‚é.
					axis[KC2_HFM_AXIS_Y] = { axis[KC2_HFM_AXIS_X].y,-axis[KC2_HFM_AXIS_X].x };
					cout << "flip" << endl;
					is_flip=true;
					//auto ac = uaxis[KC2_HFM_AXIS_X].dot({ -1,0 });
					//auto dc = 180 * acos(ac) / 3.14;
					//cout << "Hflip" << endl;
					//cout << "cos::" << dc / 3.14 << endl;
					//cout << "sin::" << ds << endl;
				}
				else {
				is_flip=false;
					axis[KC2_HFM_AXIS_Y] = { -axis[KC2_HFM_AXIS_X].y,axis[KC2_HFM_AXIS_X].x };
				}

				uaxis[KC2_HFM_AXIS_Y] = vec_to_uvec(axis[KC2_HFM_AXIS_Y]);
				auto as = uaxis[KC2_HFM_AXIS_Y].dot({ 1,0 });
				auto ds = 180 * acos(as) / 3.14;
				auto move_degree = 90 - ds;
				
				if (uaxis[KC2_HFM_AXIS_Y].y < 0) {
					move_degree = 90 + ds;
				}
				
				cout << "x:" << axis[KC2_HFM_AXIS_X] << endl;
				cout << "y:" << axis[KC2_HFM_AXIS_Y] << endl;
				cout << "move_degr1ee;;" << move_degree << endl;
				//move_degree = -move_degree;
				c_sin = sin(3.14 * move_degree / 180);
				c_cos = cos(3.14 * move_degree/180);
				/*if (abs(move_degree)<5) {
					is_enable_rotate = false;
				}*/
				/*
				Point2d v3 ={ -axis[KC2_HFM_AXIS_X].y, axis[KC2_HFM_AXIS_X].x};
				d1 = v3.dot(v1+v2);
				if(0<d1)
					axis[KC2_HFM_AXIS_Y] = {-axis[KC2_HFM_AXIS_X].y,axis[KC2_HFM_AXIS_X].x };
				else
					axis[KC2_HFM_AXIS_Y] = { axis[KC2_HFM_AXIS_X].y,-axis[KC2_HFM_AXIS_X].x };

				uaxis[KC2_HFM_AXIS_X] = vec_to_uvec(axis[KC2_HFM_AXIS_X]);
				uaxis[KC2_HFM_AXIS_Y] = vec_to_uvec( axis[KC2_HFM_AXIS_Y]);
				*/




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
				sensor_width =min(abs( vec_to_dis(range[0] - range[1]) ),abs( vec_to_dis(range[3] - range[2]))); //absaxis[0] * 2;
				//sensor_width = vec_to_dis();//absaxis[0] * 2;
				sensor_height = min(abs(vec_to_dis(range[3] - range[0])), abs(vec_to_dis(range[2] - range[1]))); //absaxis[0] * 2;

				//sensor_height =vec_to_dis((range[3] - range[0] + range[2] - range[1]) * .5);//absaxis[1] * 2;
				//sensor_height = absaxis[1] * 2;

				screen_size = {screen_width , screen_height};
				sensor_size = {sensor_width , sensor_height};
				//double mm = max(screen_width / sensor_width, screen_height / sensor_height);
				screen_from_sensor_k = { screen_width / sensor_width , screen_height / sensor_height };
				//screen_from_sensor_k = {min(screen_from_sensor_k.x,screen_from_sensor_k.y) ,min(screen_from_sensor_k.x,screen_from_sensor_k.y) };//screen_width / sensor_width , screen_height / sensor_height};

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
			is_rampaging = false;
			count_highpass.clear();
			if (flg_clear_range)clear_range_point();
		}

		void draw_ui(Mat mat) {
			Point2d center{0,0};
			for (int i = 0; i < 4; i++) {
				if (flg_range & (1 << i)) {
					circle(mat, range[i], 5, { 185,218,255 }, -1);
					center+=range[i];
				}
			}

			if (is_set_ok) {
				auto v1 = axis[0];// - op;
				auto v2 = axis[1];// - op;
				line(mat, op - v1, op + v1, { 0,255,0 }, 2);
				line(mat,op - v2,op + v2,{0,255,0},2);

				center *= .25;
				for (int i = 0; i < 4; i++) {
					
					auto del = range[i] - center;
					if (is_enable_rotate) {
						auto temp = del;
						del.x = temp.x * c_cos - temp.y * c_sin;
						del.y = temp.x * c_sin + temp.y * c_cos;

					}
					if (is_flip)del.x = -del.x;

					circle(mat, del + center, 5, { 185,0,255 }, -1);					
				}





			}

		}

	};
	namespace DetectMovePyhsicsMouse {
		kc2::timer time;
		kc2::timer start_time;
		bool is_first = true;
		bool is_moved = false;
		POINT pos_prev;
		bool input(Point2d pos_on_screen) {
			POINT p;
			GetCursorPos(&p);

			if (is_first) {
				pos_prev = p;
				start_time.start();
				is_first = false;
			}
			if (start_time.elapsed() < 2000) {
				return false;
			}


			//Point2d d={pos_on_screen.x - p.x , pos_on_screen.y - p.y};
			printf("cursor hikaku:: screen x:%d y:%d    soft x:%lf y:%lf\n", p.x, p.y, pos_on_screen.x, pos_on_screen.y);
			Point2d dis = pos_on_screen;
			dis.x -= p.x;
			dis.y -= p.y;
			double d = dis.x * dis.x + dis.y * dis.y;

			dis = { (double)p.x , (double)p.y };
			dis.x -= pos_prev.x;
			dis.y -= pos_prev.y;
			d = min(d, dis.x * dis.x + dis.y * dis.y);


			pos_prev = p;

			if (25.0 <= d) {
				time.start();
				is_moved = true;
				return true;
			}
			if (is_moved) {

				if (time.elapsed() < 2000) {
					return true;
				}

			}

			is_moved = false;
			return false;
		}

		void release() {
			is_first = true;
			is_moved = false;
		}

	}
	namespace DetectCursorStay {
		kc2::timer time;
		int64_t current=0;
		bool is_first = true;
		POINT pos_prev;
		//return 1 is trigger mouse click
		bool input(bool flg_move_cursor) {

			POINT pos_current_screen;
			GetCursorPos(&pos_current_screen);

			if (!is_first) {
				auto dx = abs(pos_current_screen.x - pos_prev.x);
				auto dy = abs(pos_current_screen.y - pos_prev.y);

				auto dis = dx * dx + dy * dy;

				if (10<=dis) {
					flg_move_cursor=true;
				}
				else {
					flg_move_cursor = false;
				}

			}
			
			if(DetectMovePyhsicsMouse::is_moved)flg_move_cursor=true;
			

			if ((flg_move_cursor || is_first)) {
				time.start();
				current=0;
				is_first = false;
				pos_prev = pos_current_screen;

				return false;
			}


			current = time.elapsed();
			if (current < hfm_prop.DetectCursorStayMillisecondTimeThreshold) {
				return false;
			}
			return true;
		}

		double get_progress(int64_t* stay_time) {
			int64_t limit = hfm_prop.DetectCursorStayMillisecondTimeThreshold;
			*stay_time = current;
			return min(1.0 , current / (double)hfm_prop.DetectCursorStayMillisecondTimeThreshold);
		}

		void release() {
			is_first = true;
			current = 0;
		}

	}



	namespace hfm {
		
		//kc2::Tracker* tr;
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
		bool is_tracking_sccess=false;
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
			//tr = new kc2::Tracker(w, h, num_track_point);
			kc2::Tracker::init(w,h,num_track_point);
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

				//if(!kc2::Tracker::is_active())return;
				is_tracking_sccess = kc2::Tracker::input(mat, interval);
				if(is_tracking_sccess){
//					auto low_pass_delta = LowPass::input(tr->get_delta());
					auto low_pass_delta = LowPass::input(kc2::Tracker::get_delta());

					//is_stay_sensor = DetectStay::input(tr->get_point(), low_pass_delta);
					is_stay_sensor = DetectStay::input(kc2::Tracker::get_point(), low_pass_delta);


					if (!hfm::is_setting_range) {
						//CursorOperator::input(tr->get_point(), low_pass_delta, tr->get_acceleration(), interval, mat);
						if (CursorOperator::is_first == false && DetectMovePyhsicsMouse::input(CursorOperator::pos_on_screen)) {
							printf("mouse move now\n");
							//CursorOperator::release();
							POINT p;
							GetCursorPos(&p);
							CursorOperator::pos_on_screen.x = p.x;
							CursorOperator::pos_on_screen.y = p.y;
							DetectCursorStay::input(1);
						}
						else {
							CursorOperator::input(kc2::Tracker::get_point(), low_pass_delta, interval, mat);
							bool click = DetectCursorStay::input(CursorOperator::flg_move_xy);
							if (is_enable_click) {
								CursorClickEvent::input(flg_mouse_event, click);
							}
						}
					}

					point_sensor = kc2::Tracker::get_point();//tr->get_point();
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
				

				if(hfm::is_avtive && hfm_prop.FlagDrawUILayout){
				//out_mat = current_mat;//clone()‚ÍÁ‚µ‚Ä‚¢‚¢‚©‚à.
					/*if (tr != nullptr) {
						tr->draw_ui(mat);
					}*/
					kc2::Tracker::draw_ui(mat);
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
				/*if (tr != nullptr) {
					tr->release();
					delete tr;
					tr = nullptr;
				}*/
				kc2::Tracker::release();
				is_first = true;
				LowPass::release();
				DetectStay::release();
				DetectCursorStay::release();
				CursorOperator::release();
				CursorClickEvent::release();
				out_frame.try_free();
				current_frame.try_free();
				DetectRampaging::release();
				DetectMovePyhsicsMouse::release();
			}
			have_range = false;
			hfm::is_tracking_sccess=false;
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
		//	if (!hfm::is_avtive)return;
			return CursorOperator::set_range_point(index,x,y);
		}
		EXPORT void kc2np_hansfreemouse_clear_range_point() {
			CursorOperator::clear_range_point();
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

		EXPORT int kc2np_hansfreemouse_get_mouse_click_event() {
			return hfm::flg_mouse_event;
		}

		EXPORT void kc2np_hansfreemouse_set_propery(const HansFreeMouseProperty* prop) {
			hfm_prop=*prop;
		}

		EXPORT void kc2np_hansfreemouse_set_click_allowed(int enable) {
			CursorClickEvent::set_click_allowed(enable);
		}

		EXPORT void kc2np_hansfreemouse_call_se() {
			CursorClickEvent::play_se();
		}

		EXPORT void kc2np_hansfreemouse_set_enable_se(int ok) {
			CursorClickEvent::set_enable_se(ok);
		}

		EXPORT void kc2np_hansfreemouse_set_flg_setting_range(int flg) {
			hfm::is_setting_range = flg;
		}

		EXPORT int kc2np_hansfreemouse_get_flg_setting_range() {
			return hfm::is_setting_range;
		}


		EXPORT int kc2np_hansfreemouse_detect_rampaning() {
			return CursorOperator::is_rampaging;
		}


		EXPORT void kc2np_hansfreemouse_set_enable_infraredtracker(int flg) {
			Tracker::set_enable_inff(flg);
		}

		EXPORT int kc2np_hansfreemouse_get_sccess_tracking() {
			return hfm::is_tracking_sccess;
		}

	}
}

