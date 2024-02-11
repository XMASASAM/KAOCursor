
using System.Runtime.InteropServices;

namespace KC2NativeWrapper
{

	[StructLayout(LayoutKind.Sequential)]
	public struct FrameInfo
	{
		public int Width;
		public int Height;
		public int BuffSize;
		public int Stride;
		public IntPtr ptr;

		public bool HaveFrameData { get { return ptr != IntPtr.Zero; } }

	}
	[StructLayout(LayoutKind.Sequential)]
	public struct CaptureProperty
	{
		public int Width;
		public int Height;
		public int FlagFpsOption;
	};
	public class KC2VideoCapture
    {
        const string DLLNAME = "KC2NativeProject.dll";
		[DllImport(DLLNAME)]
		static extern IntPtr kc2np_get_KC2VideoCapture(int capture_index,CaptureProperty prop);

		[DllImport(DLLNAME)]
		static extern FrameInfo kc2np_KC2VideoCapture_get_current_frame(IntPtr vc);

		[DllImport(DLLNAME)]
		static extern void kc2np_KC2VideoCapture_start_capture(IntPtr vc);
		[DllImport(DLLNAME)]
		static extern void kc2np_KC2VideoCapture_release_capture(ref IntPtr vc);

		[DllImport(DLLNAME)]
		static extern FrameInfo kc2np_KC2VideoCapture_get_frame_info(IntPtr vc);

		[DllImport(DLLNAME)]
		static extern int kc2np_KC2VideoCapture_get_fps(IntPtr vc);


		[DllImport(DLLNAME)]
		static extern int kc2np_KC2VideoCapture_is_opened(IntPtr vc);
		IntPtr cap;
		public KC2VideoCapture(int capture_index, CaptureProperty prop)
		{
			if (capture_index == -1) return;
			cap = kc2np_get_KC2VideoCapture(capture_index, prop);
		}

		public FrameInfo GetCurrentFrameInfo() {
			return kc2np_KC2VideoCapture_get_current_frame(cap);
		}

		public void StartCapture()
		{
			kc2np_KC2VideoCapture_start_capture(cap);
		}

		public void ReleaseCapture()
		{
			kc2np_KC2VideoCapture_release_capture(ref cap);
		}

		public void GetFrameSize(out int width,out int height)
		{
			FrameInfo frame = kc2np_KC2VideoCapture_get_frame_info(cap);
			width = frame.Width; 
			height = frame.Height;
		}

		public int GetFPS()
		{
			return kc2np_KC2VideoCapture_get_fps(cap);
		}

		public bool IsOpened { get { return kc2np_KC2VideoCapture_is_opened(cap) == 1; } }


	}

}
