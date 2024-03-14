using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using System.Windows;
using static System.Runtime.InteropServices.JavaScript.JSType;

namespace KC2NativeWrapper
{

	public enum KC2_MouseEvent
	{
		KC2_MouseEvent_None = 0,

		KC2_MouseEvent_Left = 0b00000001,
		KC2_MouseEvent_Right = 0b00000010,
		KC2_MouseEvent_Middle = 0b00000100,

		KC2_MouseEvent_Click = 0b00001000,
		KC2_MouseEvent_LeftClick = 0b00001001,
		KC2_MouseEvent_RightClick = 0b00001010,
		KC2_MouseEvent_MiddleClick = 0b00001100,

		KC2_MouseEvent_DoubleClick = 0b00010000,
		KC2_MouseEvent_LeftDoubleClick = 0b00010001,
		KC2_MouseEvent_RightDoubleClick = 0b00010010,
		KC2_MouseEvent_MiddleDoubleClick = 0b00010100,

		KC2_MouseEvent_Drag = 0b00100000,
		KC2_MouseEvent_LeftDrag = 0b00100001,
		KC2_MouseEvent_RightDrag = 0b00100010,
		KC2_MouseEvent_MiddleDrag = 0b00100100,

		KC2_MouseEvent_WheelUp = 0b01000000,
		KC2_MouseEvent_WheelDown = 0b10000000,
		KC2_MouseEvent_Wheel = 0b11000000,

	}
	static public class KC2HandsFreeMouse
	{
		const string DLLNAME = "KC2NativeProject.dll";
		[DllImport(DLLNAME)]
		static extern int kc2np_hansfreemouse_detect_stay_sensor();

		[DllImport(DLLNAME)]
		static extern void kc2np_hansfreemouse_get_point_sensor(ref double x, ref double y);


		[DllImport(DLLNAME)]
		static extern int kc2np_F4MVideoCapture_start_hansfreemouse(IntPtr vc, int track_points, int is_setting_range);


		[DllImport(DLLNAME)]
		static extern void kc2np_F4MVideoCapture_release_hansfreemouse();


		[DllImport(DLLNAME)]
		static extern void kc2np_hansfreemouse_set_range_point(int index, double x, double y);

		[DllImport(DLLNAME)]
		static extern void kc2np_hansfreemouse_set_screen_rect(Win32Mouse.Win32RECT rect);

		[DllImport(DLLNAME)]
		static extern double kc2np_hansfreemouse_get_cursor_click_progress(out Int64 stay_time);

		[DllImport(DLLNAME)]
		static extern void kc2np_hansfreemouse_set_enable_click(int enable_click);
		[DllImport(DLLNAME)]
		static extern int kc2np_hansfreemouse_get_enable_click();

		[DllImport(DLLNAME)]
		static extern void kc2np_hansfreemouse_set_mouse_click_event(int click_event);
		static public bool IsActive{ get;private set; }

		static public bool StartHansFreeMouse(F4MVideoCapture cap, int track_points, int is_setting_range)
		{
			IsActive = kc2np_F4MVideoCapture_start_hansfreemouse(cap.cap, track_points, is_setting_range) == 1;
			return IsActive;
		}


		static public void ReleaseHansFreeMouse()
		{
			IsActive = false;
			kc2np_F4MVideoCapture_release_hansfreemouse();
		}



		public static bool DetectStaySensor(){
			return kc2np_hansfreemouse_detect_stay_sensor() == 1;
		}

		public static (double X,double Y) GetPointSensor(){
			double x = 0;
			double y = 0;
			kc2np_hansfreemouse_get_point_sensor(ref x, ref y);
			return (x, y);
		}

		public static void SetRangePoint(int index , double x,double y){
			kc2np_hansfreemouse_set_range_point(index, x,y);
		}

		public static void SetScreenRect(Win32Mouse.Win32RECT rect){
			kc2np_hansfreemouse_set_screen_rect(rect);
		}

		public static double GetCursorClickProgress(out Int64 stay_time){
			return kc2np_hansfreemouse_get_cursor_click_progress(out stay_time);
		}

		public static void SetEnableClick(bool enable_click){
			int ok = 0;
			if (enable_click) ok = 1;
			kc2np_hansfreemouse_set_enable_click(ok);
		}

		public static bool GetEnableClick()
		{
			return kc2np_hansfreemouse_get_enable_click() == 1;
		}

		public static void SetMouseClickEvent(KC2_MouseEvent click_event){
			kc2np_hansfreemouse_set_mouse_click_event((int)click_event);
		}

	}
}
