using KC2NativeWrapper;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Threading;
namespace ThreadTest
{
    class VideoCaptureManager
    {
        F4MVideoCapture? cap=null;
        WriteableBitmap wb;
        Int32Rect CropRect;
        DispatcherTimer timer = new DispatcherTimer();
        bool IsActive = false;
        DeviceInfo deviceInfo;
        VideoInfo videoInfo;
        public VideoCaptureManager(DeviceInfo dinfo, VideoInfo vinfo)
        {
            IsActive = true;
            deviceInfo = dinfo;
            videoInfo = vinfo;

			InputQueue(1, this);

			CropRect = new Int32Rect(0, 0, vinfo.Width, vinfo.Height);
			wb = new WriteableBitmap(CropRect.Width, CropRect.Height, 96, 96, PixelFormats.Bgr24, null);

		}

        void StartFromQueue(){
			int index = KC2DeviceInformation.GetIndex(deviceInfo.Name, deviceInfo.Path);

			CaptureProperty prop = new CaptureProperty();
			prop.Width = videoInfo.Width;
			prop.Height = videoInfo.Height;
			cap = new F4MVideoCapture(index, prop);
		


			timer.Interval = new TimeSpan(0, 0, 0, 0, 1);
			timer.Tick += new EventHandler(Update);
			timer.Start();
		}

		public WriteableBitmap GetWritableBitmap()
		{
			return wb;
		}

		void Update(object? sender,EventArgs e){
            if (cap == null || !IsActive) return;
			var f = cap.GetCurrentFrameInfo();
			if (!f.HaveFrameData) return;
			wb.WritePixels(new Int32Rect(0, 0, f.Width, f.Height), f.ptr, f.BuffSize, f.Stride, 0, 0);

		}


        public void Release(){
            IsActive = false;
			timer.Stop();
			InputQueue(-1, this);

		}

        public void ReleaseFromQueue()
        {
            if (cap != null)
            {
                cap.ReleaseCapture();
            }
        }



		// static
		static Queue<(int flg,VideoCaptureManager cap)> queue = new Queue<(int flg, VideoCaptureManager cap)> ();
        static bool IsQueueActive = false;
        static Thread th_queue;
        static void InputQueue(int flg,VideoCaptureManager cap){
            lock (queue){
                queue.Enqueue((flg, cap));
				if (!IsQueueActive)
				{
					IsQueueActive = true;
					th_queue = new Thread(new ThreadStart(QueueOpenClose));
					th_queue.Start();
				}
			}
        }

        static void QueueOpenClose(){
			while (true)
			{
				(int flg, VideoCaptureManager cap) q;
				lock (queue)
				{
					q = queue.Dequeue();
				}

				if (0 <= q.flg)
				{
					if (q.cap.IsActive)
					{
						q.cap.StartFromQueue();
						Console.WriteLine($"VideoOpen:{queue.Count}");

					}
				}
				else
				{
					q.cap.ReleaseFromQueue();
				}

				lock (queue)
				{
					Console.WriteLine($"QueueCount:{queue.Count}");

					if (queue.Count == 0)
					{
						IsQueueActive = false;
						break;
					}
				}

			}
        }


	}
}
