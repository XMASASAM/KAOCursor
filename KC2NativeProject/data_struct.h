#pragma once



struct FrameInfo {
	int width=0;
	int height=0;
	int buffsize=0;
	int stride=0;
	unsigned char* ptr = nullptr;
};

struct CaptureProperty {
	int width=-1;
	int height=-1;
	int flag_fps_option=1;
	int angle=0;
	int hflip=0;

	int get_actual_width() {
		if(angle==90 || angle==270)return height;
		return width;
	}

	int get_actual_height() {
		if (angle == 90 || angle == 270)return width;
		return height;
	}
};