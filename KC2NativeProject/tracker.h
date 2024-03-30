#pragma once
#include<opencv2/opencv.hpp>
#include"kc2.h"
namespace kc2 {
	using namespace std;

	using namespace cv;
	class Tracker_Base {
	public:
		virtual bool input(Mat mat)=0;
		virtual Point2d get_point()=0;
		virtual Point2d get_delta()=0;
		virtual void draw_ui(Mat mat)=0;
		virtual void release()=0;
	};

	class Tracker_Face :public Tracker_Base {
		const std::string model_path = "C:\\Users\\MaMaM\\source\\repos\\KC2\\KC2NativeProject\\face_detection_yunet_2023mar.onnx";
		int cw;
		int ch;
		cv::Ptr<cv::FaceDetectorYN> yn_ptr;
		bool is_first = true;
		cv::Rect trect;
		std::vector<cv::Point2f> cors;
		cv::Mat crop;
		cv::Point2d p{ 0,0 };
		cv::Point2d del;
		cv::Point2d face_pos;
		cv::Point2d acc;
		cv::Point2d p_prev;
		cv::Point2i clip_p_prev;
		cv::Rect clip_rect;
		int max_points;
		cv::Point2d center_point;
		cv::CascadeClassifier cas;
		int cors_threshold = 100;
		bool is_op_active = false;
	public:
		Tracker_Face(int cap_width, int cap_height, int max_p);
		~Tracker_Face();
		bool input(Mat mat);
		Point2d get_point();
		Point2d get_delta();
		void draw_ui(Mat mat);
		void release();

	};


	class Tracker_Blob :public Tracker_Base {
	public:
		bool input(Mat mat);
		Point2d get_point();
		Point2d get_delta();
		void draw_ui(Mat mat);
		void release();
	};

namespace Tracker {
	void init(int cap_width,int cap_height,int max_p);
	bool input(Mat mat, int64_t interval);
	Point2d get_point();
	Point2d get_delta();
	bool is_active();

	void draw_ui(Mat mat);
	void release();
	bool detect_mono(Mat mat,int64_t interval);
}
}