#include"tracker.h"

namespace kc2 {

	namespace TB {
		int min_int(int x,int y) {
			if(x <= y)return x;
			return y;
		}
		int max_int(int x,int y) {
			if(x <=y)return y;
			return x;
		}
		void inside_rect(Mat mat,Rect* tr) {
			int mx = min_int(tr->x, mat.cols - tr->width);
			tr->x = max_int(0, mx);
			tr->y = max(0, min(tr->y, mat.rows - tr->height));

		}
    }

double Tracker_Blob::get_max_area(Mat bmat) {
	double ans=0;
	for (int y = 0; y < bmat.rows; y++) {
		int lx = -1;
		int rx = -1;
		for (int x = 0; x < bmat.cols; x++) {
			if (bmat.data[y * bmat.step + x]) {
				if (lx == -1) {
					lx = x;
				}
				rx = max(rx, x);
			}
		}
		ans += rx - lx + 1;
	}
	return ans;
}

bool Tracker_Blob::find_seals(Mat mat,vector<Seal>* seal_list) {
	Mat gmat;
	cv::cvtColor(mat, gmat, ColorConversionCodes::COLOR_BGR2GRAY);
	cv::blur(gmat, gmat, { 3,3 });
	Mat bmat;
	threshold_prev = cv::threshold(gmat, bmat, 0, 255, cv::ThresholdTypes::THRESH_OTSU);

	vector<vector<Point>> contours;
	cv::findContours(bmat, contours, RETR_TREE, CHAIN_APPROX_SIMPLE);
	int ans_index = -1;
	double ans_result = 0;
	int count_index = 0;
	for (const auto& i : contours) {
		auto rect = cv::boundingRect(i);
		Mat im(bmat, rect);
		auto mome = cv::moments(im, true);

		Point2d center = { mome.m10 / mome.m00 +rect.x, mome.m01 / mome.m00 + rect.y};
		vector<Point> hull;
		cv::convexHull(i, hull);
		auto il = cv::arcLength(i, true);
		auto hl = cv::arcLength(hull, true);
		//{
		//{
		if (0.85 * il < hl && hl < 1.176 * il) {
			if (0.5 * rect.height < rect.width && rect.width < 2 * rect.height) {
				//auto max_area = abs(cv::contourArea(i,true));
				//auto real_area =abs( mome.m00);
				auto real_area = abs(mome.m00);
				auto max_area = get_max_area(im);


				if (10.0 * (mat.cols * mat.rows / (360.0 * 240.0)) < max_area)
					//					if (abs(max_area - real_area) < 3.0) {
					if (abs(max_area - real_area) < 3) {
						//	Mat bmat255;
						//	cv::cvtColor(bmat,bmat255,ColorConversionCodes::B)
					//	TB::inside_rect(gmat,&rect);
						Mat rectgmat(gmat, rect);
						auto ave = cv::mean(rectgmat, im)[0];
						//if (150 < ave) {
						

					//	printf("index:%d max_area:%lf real_area:%lf ave:%lf\n", count_index, max_area, real_area, ave);

						/*					for (const auto& j : i) {
												cv::circle(mat, j, 1, { 0,0,255 }, -1);
											}
											cv::rectangle(mat, rect, { 0,255,0 }, 2);

											for (const auto& j : hull) {
												cv::circle(mat, j, 1, { 255,100,155 }, -1);
											}
											cv::circle(mat, center, 1, { 0,255,255 }, -1);*/

					//	if (ans_result < ave) {
						ans_index = count_index;
						ans_result = ave;
						seal_list->push_back({
							rect,
							center,
							ave,
							cv::contourArea(i)
						});
					//	}
						//}
					}
			}
		}
		count_index++;
	}
	return seal_list->size()!=0;
	//	auto rect = cv::boundingRect(contours[ans_index]);
	//	cv::rectangle(mat, rect, { 0,255,0 }, 2);
	//	printf("ans_index:%d ave:%lf\n", ans_index, ans_result);
	//	cout << "rect::" << rect << endl;
}

bool Tracker_Blob::update_seal(Mat mat,Rect&crop_r2 ,Seal* s) {
	Seal sp = *s;
	Seal ns;
//	cout << "------------------------" << endl;
//	cout << "prev_seal:" << sp.rect << " center:" << sp.center << " bright:" << sp.brightness << " area:" << sp.area << endl;
//	cout << "------------------------" << endl;
	double min_score = DBL_MAX;
	Rect crop_r = crop_r2;
	TB::inside_rect(mat, &crop_r);

	Mat crop(mat,crop_r);
	Mat gmat;
	cv::cvtColor(crop, gmat, ColorConversionCodes::COLOR_BGR2GRAY);
	cv::blur(gmat, gmat, { 3,3 });
	Mat bmat;
	//cv::threshold(gmat, bmat, 0, 255, cv::ThresholdTypes::THRESH_OTSU);
	cv::threshold(gmat,bmat,threshold_prev,255,THRESH_BINARY);
	//cv::imshow("eee",bmat);
	//cv::waitKey(1);
	cout << "threshold::" << threshold_prev << endl;
	vector<vector<Point>> contours;
	cv::findContours(bmat, contours, RETR_TREE, CHAIN_APPROX_SIMPLE);
	//cv::findContours(bmat, contours, RETR_TREE, CHAIN_APPROX_NONE);
	bool is_found=false;
	for (const auto& i : contours) {
		auto rect = cv::boundingRect(i);
		Mat im(bmat, rect);
		//auto mome = cv::moments(im, true);

		Mat rectgmat(gmat, rect);
		auto ave = cv::mean(rectgmat, im)[0];
		//auto real_area = abs(mome.m00);
		auto con_area = cv::contourArea(i);
		cout << "--------------------------" << endl;
		//double da = con_area - sp.area;
		if(0.5 * con_area < sp.area && sp.area < 2 * con_area){
			if(0.7 * ave < sp.brightness && sp.brightness < 1.42 * ave){
			//da *= da;

				Point2d center{ 0,0 }; //= { mome.m10 / mome.m00, mome.m01 / mome.m00};
				for (auto& j : i) {
					center.x += j.x;
					center.y += j.y;
				}
				center.x /= i.size();
				center.y /= i.size();
				center.x += crop_r.x;
				center.y += crop_r.y;

				auto dis = center - (sp.center + del);
				double dp = dis.x * dis.x + dis.y * dis.y;
				double score = (dp + 0.0001);// * (dv + 0.0001);



				cout << "seal:" << rect << " center:" << center << " bright:" << ave << " area:" << con_area << endl;

				cout << "score:" << score << endl;
				if (score < min_score) {
					Mat im(bmat, rect);
					auto mome = cv::moments(im, true);
					Point2d center2 = { mome.m10 / mome.m00, mome.m01 / mome.m00};
					center2.x += crop_r.x + rect.x;
					center2.y += crop_r.y + rect.y;

					min_score = score;
					is_found = true;

				//	Rect rrr = rect_resize(rect, 3);
				//	TB::inside_rect(gmat,&rrr);
				//	Mat thgmat(gmat,rrr);

				//	cv::imshow("eee2", thgmat);
					//Mat nul;
					threshold_prev = ave-30;//max(cv::threshold(thgmat, nul, 0, 255, THRESH_OTSU) , ave-30);

					rect.x += crop_r.x;
					rect.y += crop_r.y;

				//	ns = { rect,center,ave,cv::contourArea(i) };
					ns = { rect,center2,ave,cv::contourArea(i) };
				}
			}
		}
		//cv::waitKey(1);

	}
	
	if (is_found) {
		*s = ns;

		del = (ns.center - sp.center) * .6 + del*.4;
		
	}
	
	return is_found;
}



bool Tracker_Blob::input(Mat mat) {
	bool is_found = false;
	if (is_first) {
		is_tracking = false;
		vector<Seal> sl;
		is_found = find_seals(mat, &sl);

		if (!is_found) {
			return false;
		}

		double maxb=0;
		for (auto& i : sl) {
			if (maxb < i.brightness) {
				target_seal = i;
				maxb = i.brightness;
			}
		}

		crop_rect = rect_resize(target_seal.rect,10);
		is_first = false;
		return false;
	}
	//Mat cmat(mat, crop_rect);
	is_found = update_seal(mat,crop_rect,&target_seal);
	if (!is_found) {
		is_tracking = false;
		is_first = true;
		return false;
	}
	//crop_rect.x += del.x;
	//crop_rect.y += del.y;
	
	
	crop_rect = rect_resize(target_seal.rect, 10);
	is_tracking = true;
	return true;
}


Point2d Tracker_Blob::get_point() {
	if(!is_tracking)return{0,0};
	return target_seal.center;
}


Point2d Tracker_Blob::get_delta() {
	if (!is_tracking)return{ 0,0 };
	cout << "blob_del:" << del << endl;

	return del;
}


void Tracker_Blob::draw_ui(Mat mat) {
	if (is_tracking) {
		cv::rectangle(mat, crop_rect, { 0,0,255 }, 2);
		cv::rectangle(mat,target_seal.rect,{0,255,0},2);
		cv::circle(mat,target_seal.center,2,{0,0,255});
	}

//	cv::imshow("w", bmat);
//	cv::imshow("wg",mat);
//	cv::waitKey(1);
}

void Tracker_Blob::release() {
}





}