#include"kc2.h"
#include<opencv2/opencv.hpp>

namespace kc2 {
	using namespace std;
	using namespace cv;

	enum YuNetProperty {
		YuNet_X, YuNet_Y,
		YuNet_WIDTH, YuNet_HEIGHT,
		YuNet_RIGHT_EYE_X, YuNet_RIGHT_EYE_Y,
		YuNet_LEFT_EYE_X, YuNet_LEFT_EYE_Y,
		YuNet_NOSE_X, YuNet_NOSE_Y,
		YuNet_RIGHT_MOUSE_X, YuNet_RIGHT_MOUSE_Y,
		YuNet_LEFT_MOUSE_X, YuNet_LEFT_MOUSE_Y,
		YuNet_FACE_SCORE
	};

	bool find_main_face(cv::Mat& face, cv::Rect* main_face_rect) {
		int max_area = 0;
		for (int i = 0; i < face.rows; i++) {
			int w = face.at<float>(i, YuNet_WIDTH);
			int h = face.at<float>(i, YuNet_HEIGHT);
			int current_area = w * h;
			if (max_area < current_area) {
				max_area = current_area;
				main_face_rect->x = face.at<float>(i, YuNet_X);
				main_face_rect->y = face.at<float>(i, YuNet_Y);
				main_face_rect->width = w;
				main_face_rect->height = h;
			}
		}
		return face.rows;
	}


	cv::Rect rect_range(cv::Rect rect, cv::Rect range) {
		int left = rect.x;
		int right = rect.x + rect.width;
		int top = rect.y;
		int bottom = rect.y + rect.height;
		if (left < range.x)left = 0;
		if (range.width <= right)right = range.width - 1;
		if (top < range.y)top = 0;
		if (range.height <= bottom)bottom = range.height - 1;
		return { left,top,right - left , bottom - top };
	}



	cv::Rect rect_resize(cv::Rect rect, float s) {
		int cx = rect.x + (rect.width) / 2;
		int cy = rect.y + (rect.height) / 2;
		int w = rect.width * s;
		int h = rect.height * s;
		int x = cx - w / 2;
		int y = cy - h / 2;
		return { x,y,w,h };
	}


class Tracker {
	private:
		const string model_path = "C:\\Users\\MaMaM\\source\\repos\\KC2\\KC2NativeProject\\face_detection_yunet_2023mar.onnx";

		int cw;
		int ch;
		Ptr<FaceDetectorYN> yn_ptr;
		bool is_first=true;
		Rect trect;
		vector<Point2f> cors;
		Mat crop;
		Point2d p{0,0};
		Point2d del;
		Point2d acc;
		Rect pre_trect;
		Rect clip_rect;
		int max_points;
		bool get_face_rect(Mat cmat,Rect* ans) {
			Mat face;
//			Rect ans;
			(*yn_ptr).detect(cmat, face);
			if(!face.rows)return false;
			find_main_face(face, ans);
			*ans = rect_resize(*ans,.9);
			return true;
		}

		void init(Mat mat,Mat gmat) {
			
			get_face_rect(mat,&trect);
			trect = rect_range(trect, { 0,0,gmat.cols,gmat.rows });

			pre_trect = trect;
			crop = Mat(gmat, trect);
			goodFeaturesToTrack(crop, cors, max_points, 0.1, 10);
			p.x = trect.x;
			p.y = trect.y;
		}

		double IoU(Rect a,Rect b,Rect*u) {
			
			u->x = max(a.x, b.x);
			u->y = max(a.y,b.y);
			int ri = min(a.x + a.width, b.x + b.width);
			int bo = min(a.y + a.height,b.y+b.height);
			u->width =max(0, ri - u->x+1);
			u->height =max(0, bo - u->y+1);

			return (u->area()) / (double)(a.area());
		}

	public:
	Tracker(int cap_width,int cap_height,int max_p):max_points(max_p) {
		cw = cap_width;
		ch = cap_height;
		yn_ptr = cv::FaceDetectorYN::create(model_path, "",{cw,ch});
	}

	void change(int cap_width, int cap_height, int max_p) {
		cw = cap_width;
		ch = cap_height;
		(*yn_ptr).~FaceDetectorYN();
		yn_ptr = cv::FaceDetectorYN::create(model_path, "", { cw,ch });
		is_first = true;
		trect={0,0,0,0};
		cors.clear();
		p = { 0,0 };
		del = {0,0};
		acc = {0,0};
		pre_trect = {0,0,0,0};
		clip_rect = {0,0,0,0};
		max_points = max_p;
	}

	void input(Mat mat) {
		Mat gmat;
		cvtColor(mat, gmat, COLOR_BGR2GRAY);


		if (is_first) {
			is_first = false;
			init(mat,gmat);
		}

		Mat pre_crop = crop;

		trect.x = max(0, min(trect.x, gmat.cols - trect.width));
		trect.y = max(0, min(trect.y, gmat.rows - trect.height));
		clip_rect = trect;
		crop = Mat(gmat,trect);



		std::vector<cv::Point2f> cors2;
		vector<Point2f> tcors;
		vector<uchar> status;
		vector<float> err;
		auto pre_p = p;
		auto pre_del = del;
		del={0,0};

		if(cors.size()){
			calcOpticalFlowPyrLK(pre_crop, crop, cors, cors2, status, err);

			for (int i = 0; i < cors.size(); i++) {
				if (status[i]) {
					tcors.push_back(cors2[i]);
					del.x += cors2[i].x - cors[i].x;
					del.y+=cors2[i].y - cors[i].y;
				}
			}

			cors = tcors;
			del.x /= tcors.size();
			del.y /= tcors.size();

			del.x += trect.x - pre_trect.x;
			del.y += trect.y - pre_trect.y;

			p.x += del.x;
			p.y += del.y;
		}

		acc = del - pre_del;

		if (tcors.size() < 6) {
			Rect frect; 
			if(get_face_rect(mat,&frect)){
				frect = rect_range(frect,{0,0,gmat.cols,gmat.rows});
				Rect urect;
				double iou = IoU(frect,trect,&urect);

				Rect crect = urect;
				bool is_flush=false;
				if (iou <= .4) {
					crect = frect;
					is_flush = true;
				}
			
				Mat crop2(gmat,crect);
				cors.clear();
				goodFeaturesToTrack(crop2, cors, max_points, 0.1, 10);

				if (is_flush) {
					trect = crect;
					p.x = crect.x;
					p.y = crect.y;
					crop = crop2;
					del={0,0};
				}else{
					for (auto& i : cors) {
						i.x += trect.x - crect.x;
						i.y+=trect.y - crect.y;
					}
			
					p = pre_p;
				}
			}

		}


		pre_trect = trect;


		trect.x=p.x;
		trect.y=p.y;

	}

	Point2d get_point() {
		auto ans = p;
		ans.x += trect.width * .5;
		ans.y += trect.height*.5;
		return ans;
	}

	Point2d get_delta() {
		return del;
	}


	Point2d get_acceleration() {
		return acc;
	}

	Rect get_bbox() {
		return trect;
	}

	void draw_ui(Mat mat) {
		for (auto i : cors) {
			i.x += clip_rect.x;
			i.y += clip_rect.y;
			cv::circle(mat, i, 3, cv::Scalar(0, 0, 255), -1);
		}
		rectangle(mat, clip_rect, { 0,0,255 }, 2);

	}


};


class CursorOperator {
private:
	Point2d pos_cursor_screen{0,0};
	POINT pos_send_screen;
	Point2d del_lowpass_sensor;
public:

	CursorOperator() {
		
	}

	void input(Point2d pos_sensor,Point2d del_sensor,Point2d acc_sensor,int64_t interval) {
		//cout << interval << endl;
		double ts =  interval / 1e3;
		
		//Point2d del_v_sencor = del_sensor / ts;
		//printf("del:: x:%lf y:%lf  ", del_sensor.x, del_sensor.y);

		//printf("del_v:: x:%lf y:%lf interval:%d\n",del_v_sencor.x , del_v_sencor.y,interval);


		POINT pos_current_screen;
		GetCursorPos(&pos_current_screen);
		
		if (pos_send_screen.x != pos_current_screen.x || pos_send_screen.y != pos_current_screen.y) {
			pos_cursor_screen.x = pos_current_screen.x;
			pos_cursor_screen.y = pos_current_screen.y;
			del_lowpass_sensor=del_sensor;//{0,0};
		}

		del_lowpass_sensor = del_lowpass_sensor*.4 + del_sensor*.6;
		double pow2_del = acc_sensor.x * acc_sensor.x + acc_sensor.y * acc_sensor.y;
		double k_del = min(10.0,max(5.0,pow2_del));

		auto pos_add = del_lowpass_sensor * k_del;

		if ( abs(pos_add.x) < 1) {
			pos_add.x=0;
		}

		if (abs(pos_add.y) < 1) {
			pos_add.y = 0;
		}

		pos_add*=3;

		pos_add.x = -pos_add.x;

		pos_cursor_screen+= pos_add;


		pos_send_screen.x = pos_cursor_screen.x;
		pos_send_screen.y = pos_cursor_screen.y;

		POINT pos_del;
		pos_del.x = pos_send_screen.x - pos_current_screen.x;
		pos_del.y = pos_send_screen.y - pos_current_screen.y;

		mouse_event(MOUSEEVENTF_MOVE,pos_del.x,pos_del.y,NULL,NULL);
		
	}




};




EXPORT void test_track() {
	Mat mat;
	CursorOperator cuop;
	VideoCapture cap(0);
	cap.read(mat);

	Tracker tr(mat.size().width , mat.size().height,50);
	kc2::timer time;
	time.start();
	int64_t pre_time = time.elapsed();
	while (1) {
		cap.read(mat);
		tr.input(mat);
		auto ctime = time.elapsed();
		cuop.input(tr.get_point(),tr.get_delta(),tr.get_acceleration(),ctime - pre_time);
		pre_time = ctime;

		cv::blur(mat, mat, { 100,100 });
		tr.draw_ui(mat);
		imshow("qq",mat);
		waitKey(1);
	}

	cap.release();
}




}

