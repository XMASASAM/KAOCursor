#include"tracker.h"

namespace kc2 {


bool Tracker_Blob::input(Mat mat) {
return false;
}


Point2d Tracker_Blob::get_point() {
return{0,0};
}


Point2d Tracker_Blob::get_delta() {
	return{ 0,0 };
}


void Tracker_Blob::draw_ui(Mat mat) {
	Mat gmat;
	cv::cvtColor(mat,gmat,ColorConversionCodes::COLOR_BGR2GRAY);
	Mat bmat;
	cv::threshold(gmat, bmat, 0, 255, cv::ThresholdTypes::THRESH_OTSU);
	cv::imshow("w",bmat);
	cv::waitKey(1);
}

void Tracker_Blob::release() {
}





}