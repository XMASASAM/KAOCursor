#include"tracker.h"
namespace kc2 {

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

	struct YuNetLandMark
	{
		Point2d pos;
		Point2d size;
		Point2d right_eye;
		Point2d left_eye;
		Point2d nose;
		Point2d right_mouse;
		Point2d left_mouse;
		double score;

		YuNetLandMark(Mat face) {
			pos.x = face.at<float>(YuNet_X);
			pos.y = face.at<float>(YuNet_Y);
			size.x = face.at<float>(YuNet_WIDTH);
			size.y = face.at<float>(YuNet_HEIGHT);
			right_eye.x = face.at<float>(YuNet_RIGHT_EYE_X);
			right_eye.y = face.at<float>(YuNet_RIGHT_EYE_Y);
			left_eye.x = face.at<float>(YuNet_LEFT_EYE_X);
			left_eye.y = face.at<float>(YuNet_LEFT_EYE_Y);
			nose.x = face.at<float>(YuNet_NOSE_X);
			nose.y = face.at<float>(YuNet_NOSE_Y);
			right_mouse.x = face.at<float>(YuNet_RIGHT_MOUSE_X);
			right_mouse.y = face.at<float>(YuNet_RIGHT_MOUSE_Y);
			left_mouse.x = face.at<float>(YuNet_LEFT_MOUSE_X);
			left_mouse.y = face.at<float>(YuNet_LEFT_MOUSE_Y);
			score = face.at<float>(YuNet_FACE_SCORE);
		}

	};

	bool find_main_face(Mat faces, Mat& dst) {
		if (!faces.rows)return false;
		int max_area = 0;
		int ans = 0;
		for (int i = 0; i < faces.rows; i++) {
			int w = faces.at<float>(i, YuNet_WIDTH);
			int h = faces.at<float>(i, YuNet_HEIGHT);
			int current_area = w * h;
			if (max_area < current_area) {
				max_area = current_area;
				ans = i;
			}
		}
		dst = faces.row(ans);
		return true;
	}

	bool detect_safe_face(Mat mat, const YuNetLandMark& mk) {

		auto v1 = mk.nose - mk.right_mouse;
		auto v2 = mk.left_mouse - mk.right_mouse;
		auto bias_m = v1.dot(v2) / (v2.dot(v2));

		auto v3 = mk.nose - mk.right_eye;
		auto v4 = mk.left_eye - mk.right_eye;
		auto bias_e = v3.dot(v4) / (v4.dot(v4));

		auto bias = bias_e + bias_m;

		bool is_out = false;
		if (mk.pos.x < 0 || mk.pos.y < 0)is_out = true;
		if (mat.cols <= mk.pos.x + mk.size.x || mat.rows <= mk.pos.y + mk.size.y)is_out = true;

		//putText(mat, to_string(bias), { 0,100 }, 0, 1, { 0,0,255 });

		if (bias <= 0.8 || 1.2 <= bias) return false;
		if (is_out)return false;

		return true;
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

kc2::Tracker_Face::Tracker_Face(int cap_width, int cap_height, int max_p) {
	cw = cap_width;
	ch = cap_height;
	yn_ptr = cv::FaceDetectorYN::create(model_path, "", { cw,ch });
	cas.load("C:\\Users\\MaMaM\\Documents\\Programs\\C++\\cascades\\haarcascade_mcs_nose.xml");
}

bool Tracker_Face::input(Mat mat) {
	Mat gmat;
	cvtColor(mat, gmat, COLOR_BGR2GRAY);
	//Rect face;
	bool track_ok = false;
	if (is_op_active && cors.size()) {
		std::vector<cv::Point2f> cors2;
		vector<Point2f> tcors;
		vector<uchar> status;
		vector<float> err;

		auto pre_crop = crop;

		crop = Mat(gmat, clip_rect);
		trect = clip_rect;
		calcOpticalFlowPyrLK(pre_crop, crop, cors, cors2, status, err);
		auto del_prev = del;
		del = { 0,0 };
		for (int i = 0; i < cors.size(); i++) {
			if (status[i]) {
				tcors.push_back(cors2[i]);
				del.x += cors2[i].x - cors[i].x;
				del.y += cors2[i].y - cors[i].y;
			}
		}
		cors = tcors;

		if (cors.size()) {
			del.x /= tcors.size();
			del.y /= tcors.size();

			del.x += clip_rect.x - clip_p_prev.x;
			del.y += clip_rect.y - clip_p_prev.y;
			track_ok |= true;

			acc = del - del_prev;

		}
		else {
			del = { 0,0 };
			acc = { 0,0 };
		}

		clip_p_prev.x = clip_rect.x;
		clip_p_prev.y = clip_rect.y;

		p += del;

		clip_rect.x = p.x;
		clip_rect.y = p.y;


		clip_rect.x = max(0, min(clip_rect.x, gmat.cols - clip_rect.width));
		clip_rect.y = max(0, min(clip_rect.y, gmat.rows - clip_rect.height));

	}


	if (cors_threshold <= cors.size())return track_ok;

	Mat fs;
	yn_ptr->detect(mat, fs);
	Mat f;
	if (find_main_face(fs, f)) {
		YuNetLandMark mk(f);
		//auto face_pos_prev = face_pos;
		face_pos = (mk.left_eye + mk.right_eye) * .5;

		bool ok = detect_safe_face(mat, mk);
		if (ok) {
			Rect grect;
			grect.x = mk.pos.x;
			grect.y = mk.pos.y;
			double mouse_y = max(mk.left_mouse.y, mk.right_mouse.y);

			grect.width = mk.size.x;
			grect.height = mouse_y - mk.pos.y;

			grect = rect_resize(grect, 0.9);

			Mat crop2(gmat, grect);
			goodFeaturesToTrack(crop2, cors, max_points, 0.03, 10);

			clip_rect = rect_resize(grect, 1.11);

			crop = Mat(gmat, clip_rect);

			for (auto& i : cors) {
				i.x = (i.x + grect.x) - clip_rect.x;
				i.y = (i.y + grect.y) - clip_rect.y;
			}

			p.x = clip_rect.x;
			p.y = clip_rect.y;
			p_prev = p;
			clip_p_prev.x = clip_rect.x;
			clip_p_prev.y = clip_rect.y;
			is_op_active = true;
			cors_threshold = max(2, (int)cors.size() / 4);
			center_point = face_pos - p;
			trect = clip_rect;
		}
	}
	return track_ok;
}


Point2d Tracker_Face::get_point() {
	Point2d ans = p;
	ans.x += center_point.x;
	ans.y += center_point.y;
	return ans;
}

Point2d Tracker_Face::get_delta() {
	return del;

}

void Tracker_Face::draw_ui(Mat mat) {
	for (auto i : cors) {
		i.x += trect.x;
		i.y += trect.y;
		cv::circle(mat, i, 3, cv::Scalar(0, 0, 255), -1);
	}

	cv::circle(mat, get_point(), 3, cv::Scalar(0, 255, 0), -1);


	rectangle(mat, trect, { 0,0,255 }, 2);
}

void Tracker_Face::release() {
	cors.clear();
	yn_ptr.release();
	crop.release();
}





}