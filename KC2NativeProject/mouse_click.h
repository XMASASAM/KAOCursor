#pragma once
#include<Windows.h>
#include<thread>

namespace kc2{
namespace CursorClickEvent {
	using namespace std;

	bool flg_drag_press = false;
	bool flg_wheel_is_active = false;
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
			w = 12;
		}
		if (e & KC2_MouseEvent_WheelDown) {
			w=-12;
		}
		mouse_event(flg, 0, 0, w, 0);
	}


	void await_click(KC2_MouseEvent e) {
		mouse_down(e);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		mouse_up(e);
	}

	void await_double_click(KC2_MouseEvent e) {
		await_click(e);
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		await_click(e);
	}


	void await_wheel(KC2_MouseEvent e) {
		while (flg_wheel_is_active) {
			mouse_wheel(e);
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
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




}

}