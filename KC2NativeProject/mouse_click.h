#pragma once
#include<Windows.h>
#include<thread>
#include"soundeffect.h"
//#include<mmstream.h>
namespace kc2{

struct HansFreeMouseProperty
{
	double LowPathRate=.9;
	double DetectStayPixelThresholdPow2=4.0;
	int DetectStayMillisecondTimeThreshold=1000;
	double CursorMovePixelThreshold=.4;
	double CursorMoveMultiplier=8;
	double CursorMoveXFactor=1.0;
	double CursorMoveYFactor=1.2;
	int DetectCursorStayMillisecondTimeThreshold=500;
	int NumTrackPoint=30;
	int FlagDrawUILayout=1;
	int MouseClickHoldMillisecondTime=10;
	int MouseDoubleClickUnPressMillisecondTime=50;
	int MouseWheelScrollAmount=12;
	int MouseWheelScrollIntervalMillisecondTime=5;
};

HansFreeMouseProperty hfm_prop;

namespace CursorClickEvent {
	using namespace std;

	bool flg_drag_press = false;
	bool flg_wheel_is_active = false;
	bool flg_click_allowed = true;
	enum KC2_MouseEvent {
		KC2_MouseEvent_None=0,

		KC2_MouseEvent_Left              = 0b00000001,
		KC2_MouseEvent_Right             = 0b00000010,
		KC2_MouseEvent_Middle            = 0b00000100,
		
		KC2_MouseEvent_Click             = 0b00001000,
		KC2_MouseEvent_LeftClick         = 0b00001001,
		KC2_MouseEvent_RightClick        = 0b00001010,
		KC2_MouseEvent_MiddleClick       = 0b00001100,

		KC2_MouseEvent_DoubleClick       = 0b00010000,
		KC2_MouseEvent_LeftDoubleClick   = 0b00010001,
		KC2_MouseEvent_RightDoubleClick  = 0b00010010,
		KC2_MouseEvent_MiddleDoubleClick = 0b00010100,

		KC2_MouseEvent_Drag              = 0b00100000,
		KC2_MouseEvent_LeftDrag          = 0b00100001,
		KC2_MouseEvent_RightDrag         = 0b00100010,
		KC2_MouseEvent_MiddleDrag        = 0b00100100,

		KC2_MouseEvent_WheelUp           = 0b01000000,
		KC2_MouseEvent_WheelDown         = 0b10000000,
		KC2_MouseEvent_Wheel             = 0b11000000,

	};


	void mouse_down(KC2_MouseEvent e) {
		int flg = 0;
		if (e & KC2_MouseEvent_Left) {
			flg = MOUSEEVENTF_LEFTDOWN;
		}
		if (e & KC2_MouseEvent_Right) {
			flg = MOUSEEVENTF_RIGHTDOWN;
		}
		if (e & KC2_MouseEvent_Middle) {
			flg = MOUSEEVENTF_MIDDLEDOWN;
		}
		mouse_event(flg, 0, 0, 0, 0);
	}

	void mouse_up(KC2_MouseEvent e) {
		int flg = 0;
		if (e & KC2_MouseEvent_Left) {
			flg = MOUSEEVENTF_LEFTUP;
		}
		if (e & KC2_MouseEvent_Right) {
			flg = MOUSEEVENTF_RIGHTUP;
		}
		if (e & KC2_MouseEvent_Middle) {
			flg = MOUSEEVENTF_MIDDLEUP;
		}
		mouse_event(flg, 0, 0, 0, 0);
	}

	void mouse_wheel(KC2_MouseEvent e) {
		int flg = MOUSEEVENTF_WHEEL;
		int w = 0;
		if (e & KC2_MouseEvent_WheelUp) {
			w = hfm_prop.MouseWheelScrollAmount;
		}
		if (e & KC2_MouseEvent_WheelDown) {
			w=-hfm_prop.MouseWheelScrollAmount;
		}
		mouse_event(flg, 0, 0, w, 0);
	}

	//static sound::SoundEffect se(L"pc-mouse-3.mp3", 0);

	void await_click(KC2_MouseEvent e) {

/*		thread th = thread([] {
			PlaySound(L"PC-Mouse03-06(R).wav", NULL, SND_ASYNC);
		});
		th.detach();*/
		//se.play(1);
		if(!flg_click_allowed)return;
		mouse_down(e);
		std::this_thread::sleep_for(std::chrono::milliseconds(hfm_prop.MouseClickHoldMillisecondTime));
		mouse_up(e);
	}

	void await_double_click(KC2_MouseEvent e) {
		await_click(e);
		std::this_thread::sleep_for(std::chrono::milliseconds(hfm_prop.MouseDoubleClickUnPressMillisecondTime));
		await_click(e);
	}


	void await_wheel(KC2_MouseEvent e) {
		if (!flg_click_allowed)return;

		while (flg_wheel_is_active) {
			mouse_wheel(e);
			std::this_thread::sleep_for(std::chrono::milliseconds(hfm_prop.MouseWheelScrollIntervalMillisecondTime));
		}
	}



	void trigger_click(KC2_MouseEvent e) {
		thread th;
		if (e & KC2_MouseEvent_Click) {
			th=thread(await_click,e);
			th.detach();
		}

		if (e & KC2_MouseEvent_DoubleClick) {
			th = thread(await_double_click,e);
			th.detach();
		}

		if (e & KC2_MouseEvent_Drag) {
			flg_drag_press=!flg_drag_press;
			if (flg_drag_press) {
				mouse_down(e);
			}
			else {
				mouse_up(e);
			}
		}
		else {
			flg_drag_press = false;
		}

		if (e & KC2_MouseEvent_Wheel) {
			flg_wheel_is_active=true;//!flg_wheel_is_active;
			if (flg_wheel_is_active) {
				th=thread(await_wheel,e);
				th.detach();
			}
			else {

			}
		}
		else {
			flg_wheel_is_active = false;
		}

	}

	bool flg_stay_prev = false;
	void input(KC2_MouseEvent e,bool is_stay) {
		if (is_stay && !flg_stay_prev) {
			trigger_click(e);
		}
		flg_stay_prev = is_stay;
		if(!is_stay)flg_wheel_is_active = false;
	}

	void release() {
		flg_drag_press = false;
		flg_stay_prev = false;
		flg_wheel_is_active = false;

	}

	void set_click_allowed(bool enable) {
		flg_click_allowed = enable;
	}




}

}