using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace KC2NativeWrapper
{
	public class Win32Mouse
	{
		[DllImport("User32.dll")]
		public static extern bool GetCursorPos(out Win32POINT lppoint);
		[StructLayout(LayoutKind.Sequential)]
		public struct Win32POINT
		{
			public int X { get; set; }
			public int Y { get; set; }
		}
	}
}
