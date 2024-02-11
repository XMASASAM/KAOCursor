using KC2.DataStructs;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;

namespace KC2.Controls
{
	/// <summary>
	/// VideoCaptureView.xaml の相互作用ロジック
	/// </summary>
	public partial class VideoCaptureView : UserControl
	{
		public static readonly DependencyProperty CroppedFrameProperty =
			DependencyProperty.Register(nameof(CroppedFrame),typeof(WriteableBitmap),typeof(VideoCaptureView),new PropertyMetadata(null));
		public WriteableBitmap CroppedFrame
		{
			get { return (WriteableBitmap)GetValue(CroppedFrameProperty); }
			set { SetValue(CroppedFrameProperty, value); }
		}

		public static readonly DependencyProperty FPSDeviceProperty =
			DependencyProperty.Register(nameof(FPSDevice), typeof(int), typeof(VideoCaptureView), new PropertyMetadata(0));
		public int FPSDevice
		{
			get { return (int)GetValue(FPSDeviceProperty); }
			private set { SetValue(FPSDeviceProperty, value); }
		}

		public static readonly DependencyProperty FPSDisplayProperty =
			DependencyProperty.Register(nameof(FPSDisplay), typeof(int), typeof(VideoCaptureView), new PropertyMetadata(0));
		public int FPSDisplay
		{
			get { return (int)GetValue(FPSDisplayProperty); }
			private set { SetValue(FPSDisplayProperty, value); }
		}

		public static readonly DependencyProperty AngleProperty =
			DependencyProperty.Register(nameof(Angle), typeof(double), typeof(VideoCaptureView), new PropertyMetadata(0.0));
		public double Angle
		{
			get { return (double)GetValue(AngleProperty); }
			set { SetValue(AngleProperty, value); }
		}

		public static readonly DependencyProperty ScaleXProperty =
			DependencyProperty.Register(nameof(ScaleX), typeof(double), typeof(VideoCaptureView), new PropertyMetadata(1.0));
		public double ScaleX
		{
			get { return (double)GetValue(ScaleXProperty); }
			set { SetValue(ScaleXProperty, value); }
		}

		public static readonly DependencyProperty ScaleYProperty =
			DependencyProperty.Register(nameof(ScaleY), typeof(double), typeof(VideoCaptureView), new PropertyMetadata(1.0));
		public double ScaleY
		{
			get { return (double)GetValue(ScaleYProperty); }
			set { SetValue(ScaleYProperty, value); }
		}


		VideoCaptureDevice? device = null;
		//WriteableBitmap? CroppedFrame;
		Int32Rect CropRect;
		DispatcherTimer timer = new DispatcherTimer();
		Dictionary<Vec2I, WriteableBitmap> wmap = new Dictionary<Vec2I, WriteableBitmap>();
		MeasureFPS mfps = new MeasureFPS();
		public VideoCaptureView()
		{
			InitializeComponent();
			timer.Interval = new TimeSpan(0, 0, 0, 0, 10);
			timer.Tick += new EventHandler(Update);
		
		}

		public void StartCapture(int capture_index, KC2NativeWrapper.CaptureProperty prop)
		{
			if(device != null)
			{
				timer.Start();
				if (device.CaptureIndex == capture_index) return;
				timer.Stop();
				VideoCaptureDevice.TryReleaseDevice(ref device);
			}

			device =  VideoCaptureDevice.GetDevice(capture_index, prop);

			CropRect = new Int32Rect(0, 0, device.FrameSize.X, device.FrameSize.Y);

			device.Start();
			timer.Start();
			
		}
		public void StartCapture(int capture_index){
			KC2NativeWrapper.CaptureProperty prop;
			prop.Width = 0;
			prop.Height=0;
			prop.FlagFpsOption = 1;
			StartCapture(capture_index, prop);
		}

		public void StopCapture(){
			if(device != null){
				timer.Stop();
			}
		}
		public void ReStartCapture()
		{
			if (device != null)
			{
				timer.Start();
			}
		}
		public void ReleaseCapture(){
			if(device != null){
				timer.Stop();
				VideoCaptureDevice.TryReleaseDevice(ref device);
			}
		}

		void Update(object? sender, EventArgs e)
		{
			var f = device.GetCurrentFrame(out bool is_new_frame);
			if (!f.HaveFrameData) return;
			//if (!is_new_frame) return;

			//Vec2I fsize = new(f.Width, f.Height);
			Vec2I csize = new(CropRect.Width, CropRect.Height);

			if (!wmap.ContainsKey(csize))
			{
				wmap.Add(csize, new WriteableBitmap(csize.X, csize.Y, 96, 96, PixelFormats.Bgr24, null));
			}
			if (CroppedFrame != wmap[csize])
			{
				CroppedFrame = wmap[csize];
			}
			wmap[csize].WritePixels(CropRect, f.ptr, f.BuffSize, f.Stride, 0, 0);

			//if(device.CheckHaveNewFrame())
			mfps.Count();


			FPSDevice = device.FPS;
			FPSDisplay = mfps.GetFPS();
		}

	}
}
