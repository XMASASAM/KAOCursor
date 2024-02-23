using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using System.Windows;

namespace KC2NativeWrapper
{
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
		
	}
}
