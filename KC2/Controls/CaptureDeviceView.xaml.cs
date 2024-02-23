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
	/// CaptureDeviceView.xaml の相互作用ロジック
	/// </summary>
	public partial class CaptureDeviceView : UserControl
	{
		public static readonly DependencyProperty CurrentFrameProperty =
			DependencyProperty.Register(nameof(CurrentFrame), typeof(WriteableBitmap), typeof(CaptureDeviceView), new PropertyMetadata(null));
		public WriteableBitmap CurrentFrame
		{
			get { return (WriteableBitmap)GetValue(CurrentFrameProperty); }
			set { SetValue(CurrentFrameProperty, value); }
		}

		public static readonly DependencyProperty CaptureFPSProperty =
		DependencyProperty.Register(nameof(CaptureFPS), typeof(int), typeof(CaptureDeviceView), new PropertyMetadata(0));
		public int CaptureFPS
		{
			get { return (int)GetValue(CaptureFPSProperty); }
			private set { SetValue(CaptureFPSProperty, value); }
		}

		public static readonly DependencyProperty DisplayFPSProperty =
			DependencyProperty.Register(nameof(DisplayFPS), typeof(int), typeof(CaptureDeviceView), new PropertyMetadata(0));
		public int DisplayFPS
		{
			get { return (int)GetValue(DisplayFPSProperty); }
			private set { SetValue(DisplayFPSProperty, value); }
		}


		DispatcherTimer timer = new DispatcherTimer();

		public CaptureDeviceView()
		{
			InitializeComponent();

			CurrentFrame = CaptureDevice.CurrentFrame;

			timer.Interval = new TimeSpan(0, 0, 0, 0, 1);
			timer.Tick += new EventHandler(Update);
			timer.Start();
		}

		void Update(object? sender, EventArgs e)
		{
			if(CurrentFrame != CaptureDevice.CurrentFrame && CaptureDevice.CurrentFrame!=null){
				CurrentFrame = CaptureDevice.CurrentFrame;
			}

			CaptureFPS = CaptureDevice.GetCaptureFPS();
			DisplayFPS = CaptureDevice.GetDisplayFPS();

		}


		public void Start(){
			timer.Start();
		}

		public void Stop(){
			timer.Stop();
		}

	}
}
