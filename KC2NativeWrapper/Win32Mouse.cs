using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
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
		static extern bool GetCursorPos(out Win32POINT lppoint);

		[DllImport("User32.dll")]
		public static extern IntPtr MonitorFromPoint(Win32POINT p,uint flg);

		[DllImport("User32.dll")]
		public static extern bool GetMonitorInfoA(IntPtr ptr,ref Win32MONITORINFO lpm);

		[DllImport("User32.dll")]
		public static extern uint GetDpiForSystem();


		[DllImport("Shcore.dll")]
		public static extern int GetDpiForMonitor(IntPtr hmonitor,uint dpiType,out uint dpiX,out uint dpiY);
		[DllImport("User32.dll")]
		public static extern bool ScreenToClient(IntPtr hWnd,ref Win32POINT lpPoint);
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

		static int kakeru(int v,double k){
			return (int)(v*k);
		}

		static Win32RECT kakeru(Win32RECT v, double k){
			v.left = kakeru(v.left,k);
			v.top = kakeru(v.top,k);
			v.right = kakeru(v.right,k);
			v.bottom = kakeru(v.bottom,k);
			return v;
		}

		public static (Win32RECT worck,Win32RECT all, (double x, double y) dpi_k) GetCurrentScreenRect(int screen_x, int screen_y,bool on_monitor=false)
		{ 
			IntPtr ptr = MonitorFromPoint(new Win32POINT {X= screen_x,Y= screen_y }, (uint)Win32MONITOR.DEFAULTTOPRIMARY);
			Win32MONITORINFO lpm = new Win32MONITORINFO();
			lpm.cbSize = (uint)Marshal.SizeOf(typeof(Win32MONITORINFO));
			GetMonitorInfoA(ptr, ref lpm);

			(double x, double y) kdp;
			if(GetDpiForMonitor(ptr,0,out uint dx,out uint dy)==0){
				kdp.x = 96.0/dx;
				kdp.y = 96.0/dy;
			}else{
				kdp.x = 1.0;
				kdp.y = 1.0;
			}
			if(on_monitor){
				return (lpm.rcWork , lpm.rcMonitor , kdp);
			}
			return (kakeru( lpm.rcWork , kdp.x),kakeru( lpm.rcMonitor , kdp.y) , kdp);
		}

		public static (int X,int Y) GetCursorPosOnScreen(){
			GetCursorPos(out Win32POINT cp);
			var m = GetCurrentScreenRect(cp.X, cp.Y);

			//IntPtr ptr = MonitorFromPoint(new Win32POINT { X = , Y = screen_y }, (uint)Win32MONITOR.DEFAULTTOPRIMARY);
			return (kakeru(cp.X , m.dpi_k.x), kakeru(cp.Y,m.dpi_k.y));
		}
		public static (int X, int Y) GetCursorPosOnMonitor()
		{
			GetCursorPos(out Win32POINT cp);
			return (cp.X,cp.Y);
		}
		public static (int X,int Y) GetDialogPosNearMouse(int dialog_width,int dialog_height){
			GetCursorPos(out Win32POINT cpt);
			var m = GetCurrentScreenRect(cpt.X, cpt.Y);
			var r = m.worck;
			var cp = GetCursorPosOnScreen();
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
