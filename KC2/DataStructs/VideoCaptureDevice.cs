using KC2NativeWrapper;
using System;
using System.Collections.Generic;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Threading;

namespace KC2.DataStructs
{
	public class VideoCaptureDevice
	{
		static Dictionary<int,VideoCaptureDevice> ActiveDeviceList = new Dictionary<int, VideoCaptureDevice>();
		F4MVideoCapture cap;
		DispatcherTimer timer = new DispatcherTimer();
		FrameInfo current_frame;
		int ShareCount = 0;
		int capture_index;
		bool is_active_capture = false;
		public int CaptureIndex {  get { return capture_index; } }
		public Vec2I FrameSize {  get;private set; }
		public int FPS
		{
			get
			{
				//if (!cap.IsOpened) return 0;
				return cap.GetFPS();
			}
		}
		bool have_new_frame = false;
		CaptureProperty prop;
		VideoCaptureDevice(int capture_index , CaptureProperty prop) {

			this.capture_index = capture_index;
			this.prop = prop;
			cap = new F4MVideoCapture(capture_index,this.prop);
			timer.Interval = new TimeSpan(0, 0, 0, 0, 10);
			timer.Tick += new EventHandler(Update);
		   // if (!cap.IsOpened) return;
			//cap.GetFrameSize(out int w, out int h);
			prop = cap.GetCaptureProperty();
			FrameSize = new Vec2I(prop.Width,prop.Height);
		}

		//ここの処理は重い
		public static VideoCaptureDevice GetDevice(int capture_index, CaptureProperty prop)
		{
			if (!ActiveDeviceList.ContainsKey(capture_index)){
				ActiveDeviceList.Add(capture_index, new VideoCaptureDevice(capture_index, prop));
			}
			var d = ActiveDeviceList[capture_index];
			d.ShareCount++;
			return d;
		}

		//ここの処理も重い
		public static void TryReleaseDevice(ref VideoCaptureDevice? device)
		{
			device.ShareCount--;
			if (device.ShareCount <= 0)
			{
				ActiveDeviceList.Remove(device.capture_index);
				device.cap.ReleaseCapture();
			}
			device = null;
		}
		static public void AllActiveDeviceRelease(){
			foreach(var i in ActiveDeviceList){
				var d = i.Value;
				TryReleaseDevice(ref d);
			}
		}
		public void Start()
		{
			if (is_active_capture) return;
//			cap.StartCapture();
			timer.Start();
			is_active_capture = true;
		}


		void Update(object? sender, EventArgs e)
		{
			var f = cap.GetCurrentFrameInfo();
			if (!f.HaveFrameData)
			{
				have_new_frame = false;
				return;
			}

			current_frame = f;
			have_new_frame = true;
		}

		public FrameInfo GetCurrentFrame(out bool is_new_frame)
		{
			is_new_frame = have_new_frame;
			return current_frame;
		}


	}
}
