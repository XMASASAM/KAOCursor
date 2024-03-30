using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace KC2NativeWrapper
{
	public class Win32Mouse
	{
		enum Win32MONITOR{
			DEFAULTTONULL = 0,      
			DEFAULTTOPRIMARY = 1,    
			DEFAULTTONEAREST = 2   
		}
		[DllImport("User32.dll")]
		public static extern bool GetCursorPos(out Win32POINT lppoint);

		[DllImport("User32.dll")]
		public static extern IntPtr MonitorFromPoint(Win32POINT p,uint flg);

		[DllImport("User32.dll")]
		public static extern bool GetMonitorInfoA(IntPtr ptr,ref Win32MONITORINFO lpm);



		[StructLayout(LayoutKind.Sequential)]
		public struct Win32POINT
		{
			public int X { get; set; }
			public int Y { get; set; }
		}

		[StructLayout(LayoutKind.Sequential)]
		public struct Win32RECT
		{
			public int left { get; set; }
			public int top { get; set; }
			public int right { get; set; }
			public int bottom { get; set; }
		}

		[StructLayout(LayoutKind.Sequential)]
		public struct Win32MONITORINFO
		{
			public uint cbSize;
			public Win32RECT rcMonitor;
			public Win32RECT rcWork;
			public uint dwFlags;
		}

		public static (Win32RECT worck,Win32RECT all) GetCurrentScreenRect(int screen_x, int screen_y)
		{ 
			IntPtr ptr = MonitorFromPoint(new Win32POINT {X= screen_x,Y= screen_y }, (uint)Win32MONITOR.DEFAULTTOPRIMARY);
			Win32MONITORINFO lpm = new Win32MONITORINFO();
			lpm.cbSize = (uint)Marshal.SizeOf(typeof(Win32MONITORINFO));
			GetMonitorInfoA(ptr, ref lpm);
			return (lpm.rcWork, lpm.rcMonitor);
		}


		public static (int X,int Y) GetDialogPosNearMouse(int dialog_width,int dialog_height){
			GetCursorPos(out Win32POINT cp);
			var r = GetCurrentScreenRect(cp.X, cp.Y).worck;

			int w = dialog_width;
			int h = dialog_height;
			int x = cp.X - w / 2;
			int y = cp.Y - h / 2;

			if (x < r.left) x = r.left;
			if (y < r.top) y = r.top;
			if (r.right < x + w) x = r.right - w;
			if (r.bottom < y + h) y = r.bottom - h;
			return (x,y);
		}




	}
}
