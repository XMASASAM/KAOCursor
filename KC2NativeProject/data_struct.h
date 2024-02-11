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
};