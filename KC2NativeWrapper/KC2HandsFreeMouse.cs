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

		KC2_MouseEvent_Bashing     =  0b00100000000,
		KC2_MouseEvent_LeftBashing =  0b00100000001,
		KC2_MouseEvent_RightBasing =  0b00100000010,
		KC2_MouseEvent_MiddleBasing = 0b00100000100,
		KC2_MouseEvent_Stop         = 0b01000000000,
		KC2_MouseEvent_ShowMenu     = 0b10000000000

	}

	[StructLayout(LayoutKind.Sequential)]
	public struct HansFreeMouseProperty
	{
		public double LowPathRate{ get; set; }
		public double DetectStayPixelThresholdPow2 { get; set; }
		public int DetectStayMillisecondTimeThreshold { get; set; }
		public double CursorMovePixelThreshold { get; set; }
		public double CursorMoveMultiplier { get; set; }
		public double CursorMoveXFactor { get; set; }
		public double CursorMoveYFactor { get; set; }
		public int DetectCursorStayMillisecondTimeThreshold { get; set; }
		public int NumTrackPoint { get; set; }
		public int FlagDrawUILayout { get; set; }
		public int MouseClickHoldMillisecondTime { get; set; }
		public int MouseDoubleClickUnPressMillisecondTime { get; set; }
		public int MouseWheelScrollAmount { get; set; }
		public int MouseWheelScrollIntervalMillisecondTime { get; set; }

		public HansFreeMouseProperty(){
			SetDefault();
		}

		public void SetDefault(){
			LowPathRate = .9;
			DetectStayPixelThresholdPow2 = 4.0;
			DetectStayMillisecondTimeThreshold = 1000;
			CursorMovePixelThreshold = .4;
			CursorMoveMultiplier = 8;
			CursorMoveXFactor = 1.0;
			CursorMoveYFactor = 1.2;
			DetectCursorStayMillisecondTimeThreshold = 500;
			NumTrackPoint = 30;
			FlagDrawUILayout = 1;
			MouseClickHoldMillisecondTime = 10;
			MouseDoubleClickUnPressMillisecondTime = 50;
			MouseWheelScrollAmount = 12;
			MouseWheelScrollIntervalMillisecondTime = 5;
		}


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
		[DllImport(DLLNAME)]
		static extern void kc2np_hansfreemouse_set_propery(ref HansFreeMouseProperty prop);

		[DllImport(DLLNAME)]
		static extern int kc2np_hansfreemouse_get_mouse_click_event();

		[DllImport(DLLNAME)]
		static extern void kc2np_hansfreemouse_set_click_allowed(int enable);

		[DllImport(DLLNAME)]

		static extern void kc2np_hansfreemouse_call_se();
		[DllImport(DLLNAME)]

		static extern void kc2np_hansfreemouse_set_enable_se(int ok);

		[DllImport(DLLNAME)]
		static extern void kc2np_hansfreemouse_clear_range_point();

		[DllImport(DLLNAME)]
		static extern void kc2np_hansfreemouse_set_flg_setting_range(int flg);

		[DllImport(DLLNAME)]
		static extern int kc2np_hansfreemouse_detect_rampaning();
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

		public static void ClearRangePoint(){
			kc2np_hansfreemouse_clear_range_point();
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

		public static KC2_MouseEvent GetMouseClickEvent(){
			return (KC2_MouseEvent)kc2np_hansfreemouse_get_mouse_click_event();
		}

		public static void SetPropety(HansFreeMouseProperty prop){
			kc2np_hansfreemouse_set_propery(ref prop);
		}

		public static void SetClickAllowed(int enable){
			kc2np_hansfreemouse_set_click_allowed(enable);
		}
		public static void PlaySE(){
			kc2np_hansfreemouse_call_se();
		}

		public static void SetEnablePlaySE(int ok){
			kc2np_hansfreemouse_set_enable_se(ok);
		}

		public static void SetFlagSettingRange(int flg){
			kc2np_hansfreemouse_set_flg_setting_range(flg);
		}

		public static bool DetectRampaging(){
			return kc2np_hansfreemouse_detect_rampaning()==1;
		}

	}
}
