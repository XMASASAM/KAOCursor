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
		static extern FrameInfo kc2np_F4MVideoCapture_get_current_frame(IntPtr vc,int is_draw_hfm_ui);

		[DllImport(DLLNAME)]
		static extern void kc2np_F4MVideoCapture_release_capture(ref IntPtr vc);

		[DllImport(DLLNAME)]
		static extern CaptureProperty kc2np_F4MVideoCapture_get_CaptureProperty(IntPtr vc);
		[DllImport(DLLNAME)]
		static extern int kc2np_F4MVideoCapture_get_fps(IntPtr vc);

		[DllImport(DLLNAME)]
		static extern void kc2np_F4MVideoCapture_change_flip(IntPtr vc, int angle, int hflip);

		internal IntPtr cap = IntPtr.Zero;
		CaptureProperty prop;
		int index = -1;
		public bool IsOpened
		{
			get { return cap != IntPtr.Zero; }
		}
		public F4MVideoCapture(int capture_index, CaptureProperty prop)
		{
			if (capture_index == -1) return;
			cap = kc2np_get_F4MVideoCapture(capture_index, prop);
			
			if (cap == IntPtr.Zero) return;
			this.prop = prop;
			index = capture_index;
		}

		public FrameInfo GetCurrentFrameInfo()
		{
			return kc2np_F4MVideoCapture_get_current_frame(cap,1);
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

		public void ChangeFlip(int angle, int hflip)
		{
			kc2np_F4MVideoCapture_change_flip(cap, angle, hflip);
			prop.HorizonFlip = hflip;
			prop.Angle = angle;
		}


		public string GetID()
		{
			var di = KC2DeviceInformation.DeviceList[index];
			return $"{di.Name + di.Path}{prop.Width}{prop.Height}{prop.Angle}";
		}

	}
}
