#pragma once
#include<opencv2/opencv.hpp>

namespace kc2{

class Tracker {
private:
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
	bool is_op_active=false;
	bool get_face_rect(cv::Mat cmat, cv::Rect* ans,cv::Point2d*cp);

	bool init(cv::Mat mat, cv::Mat gmat);

	double IoU(cv::Rect a, cv::Rect b, cv::Rect* u);

public:
	Tracker(int cap_width, int cap_height, int max_p);

	void change(int cap_width, int cap_height, int max_p);

	bool input(cv::Mat mat);

	cv::Point2d get_point();

	cv::Point2d get_delta();


	cv::Point2d get_acceleration();

	cv::Rect get_bbox();

	void draw_ui(cv::Mat mat);

	void release();

};

}