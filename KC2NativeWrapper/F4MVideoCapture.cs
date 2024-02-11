using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Controls;

namespace KC2NativeWrapper
{
	public class F4MVideoCapture
	{
		const string DLLNAME = "KC2NativeProject.dll";

		[DllImport(DLLNAME)]
		static extern IntPtr kc2np_get_F4MVideoCapture(int capture_index, CaptureProperty prop);

		[DllImport(DLLNAME)]
		static extern FrameInfo kc2np_F4MVideoCapture_get_current_frame(IntPtr vc);

		[DllImport(DLLNAME)]
		static extern void kc2np_F4MVideoCapture_release_capture(ref IntPtr vc);

		[DllImport(DLLNAME)]
		static extern CaptureProperty kc2np_F4MVideoCapture_get_CaptureProperty(IntPtr vc);
		[DllImport(DLLNAME)]
		static extern int kc2np_F4MVideoCapture_get_fps(IntPtr vc);


		IntPtr cap;
		public bool IsOpened{ get; private set; } = false;
		public F4MVideoCapture(int capture_index, CaptureProperty prop)
		{
			if (capture_index == -1) return;
			cap = kc2np_get_F4MVideoCapture(capture_index, prop);
			if (cap == IntPtr.Zero) return;
			IsOpened = true;
		}

		public FrameInfo GetCurrentFrameInfo()
		{
			return kc2np_F4MVideoCapture_get_current_frame(cap);
		}

		public void ReleaseCapture()
		{
			kc2np_F4MVideoCapture_release_capture(ref cap);
		}

		public CaptureProperty GetCaptureProperty()
		{
			return kc2np_F4MVideoCapture_get_CaptureProperty(cap);
		}
		public int GetFPS()
		{
			return kc2np_F4MVideoCapture_get_fps(cap);
		}
	}
}
