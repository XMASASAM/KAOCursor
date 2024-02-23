using KC2.Controls;
using KC2NativeWrapper;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media.Imaging;
using System.Windows;
using System.Windows.Threading;
using System.Windows.Media;

namespace KC2.DataStructs
{
	public static class CaptureDevice
	{

		static int cap_index;
		static bool is_active_capture = false;
		public static CaptureProperty Property{ get; private set; }
		static F4MVideoCapture? cap;
		static DispatcherTimer timer = new DispatcherTimer();
		public static WriteableBitmap? CurrentFrame { get; private set; } = null;
		static Dictionary<Vec2I, WriteableBitmap> wmap = new Dictionary<Vec2I, WriteableBitmap>();

		static MeasureFPS mfps = new MeasureFPS();

		public static bool IsOpened
		{
			get
			{
				if (cap == null) return false;
				return cap.IsOpened;
			}
		}

		static CaptureDevice() {
			timer.Interval = new TimeSpan(0, 0, 0, 0, 10);
			timer.Tick += new EventHandler(Update);
			
		}

		public static bool Open(int capture_index, CaptureProperty property)
		{

			cap_index = capture_index;

			if(cap!=null && cap.IsOpened){
				cap.ReleaseCapture();
			}
			cap = new F4MVideoCapture(cap_index, property);

			if(!cap.IsOpened)return false;

			Property = cap.GetCaptureProperty();
			timer.Start();
			return true;
		}


		public static void ReleaseCapture(){
			if (cap==null || !cap.IsOpened) return;
			cap.ReleaseCapture();
		}

		static void Update(object? sender, EventArgs e)
		{
			var f = cap.GetCurrentFrameInfo();
			if (!f.HaveFrameData) return;

			Vec2I fsize = new(f.Width, f.Height);
			//Vec2I csize = new(CropRect.Width, CropRect.Height);

			if (!wmap.ContainsKey(fsize))
			{
				wmap.Add(fsize, new WriteableBitmap(fsize.X, fsize.Y, 96, 96, PixelFormats.Bgr24, null));
			}
			if (CurrentFrame != wmap[fsize])
			{
				CurrentFrame = wmap[fsize];
			}

			wmap[fsize].WritePixels(new Int32Rect(0, 0, f.Width, f.Height), f.ptr, f.BuffSize, f.Stride, 0, 0);

			mfps.Count();

		}

		static public void ChangeFlip(int angle, int hflip)
		{
			if (cap == null || !cap.IsOpened) return;

			cap.ChangeFlip(angle, hflip);
			var prop = Property;
			prop.Angle = angle;
			prop.HorizonFlip = hflip;
			Property = prop;
		}

		public static int GetDisplayFPS(){
			return mfps.GetFPS();
		}

		public static int GetCaptureFPS(){
			if (cap == null || !cap.IsOpened) return 0;
			return cap.GetFPS();
		}

		public static int GetCurrentIndex(){
			return cap_index;
		}

		public static bool StartHandsFreeMouse(int is_range_setting = 0){
			if (cap == null || !cap.IsOpened) return false ;
			return KC2HandsFreeMouse.StartHansFreeMouse(cap, 50, is_range_setting);
		}

		public static void ReleaseHandsFreeMouse(){
			KC2HandsFreeMouse.ReleaseHansFreeMouse();
		}

		public static bool HandsFreeMouseIsActive(){
			return KC2HandsFreeMouse.IsActive;
		}

	}
}
