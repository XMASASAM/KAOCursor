using KC2.DataStructs;
using KC2NativeWrapper;
using System;
using System.Collections.Generic;
using System.Diagnostics;
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

namespace KC2.Pages
{
	/// <summary>
	/// RangePage.xaml の相互作用ロジック
	/// </summary>
	public partial class RangePage : Page
	{
		MainPage mainPage;
		SaveData config;
		DispatcherTimer timer = new DispatcherTimer();
		List<Vec2d> range = new List<Vec2d>();
		Dialogs.NoButtonDialog? dialog;
		int step = 0;
		bool stay = false;
		Stopwatch stopwatch = new Stopwatch();
		Win32Mouse.Win32RECT sr;
		public RangePage(MainPage main)
		{
			InitializeComponent();
			mainPage = main;
			config = ((App)Application.Current).SaveData;


			timer.Interval = new TimeSpan(0, 0, 0, 0, 30);
			timer.Tick += new EventHandler(Update);


		}

		private void ReturnButton_Click(object sender, RoutedEventArgs e)
		{
			NavigationService.GoBack();
		}

		private void Page_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
		{
			bool visible = (bool)e.NewValue;
			if (visible)
			{

			}
			else
			{
				/*App app = (App)Application.Current;
				if (!config.Equals(app.SaveData))
				{

					app.SetSaveData(config);
				}*/
				CaptureDevice.ReleaseHandsFreeMouse();
				CameraView.Stop();//ReleaseCapture();
			}
		}

		private void StartButton_Click(object sender, RoutedEventArgs e)
		{
			CaptureDevice.ReleaseHandsFreeMouse();

			if (CaptureDevice.StartHandsFreeMouse(1)){
				var p = this.PointToScreen(new Point(0, 0));
				sr = Win32Mouse.GetCurrentScreenRect((int)p.X, (int)p.Y).all;
				KC2HandsFreeMouse.SetScreenRect(sr);
				step = 0;
				timer.Start();
				dialog = new Dialogs.NoButtonDialog();
				dialog.SetImage((BitmapImage)Application.Current.Resources["IPC_LT"]);
				dialog.SetText("画面の左上を見てください");
				dialog.ShowDialog();



			}
		}
		void Update(object? sender, EventArgs e){

			stay = KC2HandsFreeMouse.DetectStaySensor();

			Step0();
			Step1();
			Step2();
			Step3();
			Step4();
			Step5();
			Step6();
			Step7();
			Step8();
			Step9();
			Step10();
		}

		void Step0(){
			if (step==0 && dialog != null)
			{
				if (dialog.IsVisible)
				{
					stopwatch.Restart();
					step = 1;
				}
			}
		}

		void Step1(){

			if (step==1 && 1000 < stopwatch.ElapsedMilliseconds)
			{
				Vec2d t = new Vec2d();
				t.X = sr.left;//(sr.right - sr.left - dialog.ActualWidth) * .5;
				t.Y = sr.top;//(sr.bottom - sr.top - dialog.ActualHeight) * .5;
				dialog.SetMoveAnime(t, 1000);
				step = 2;
			}
		}

		void Step2(){
			if(step==2 && !dialog.IsMoving){
				stopwatch.Restart();
				step = 3;
			}
		}

		void Step3(){
			if(step==3&& !dialog.IsMoving && 1000 <stopwatch.ElapsedMilliseconds&& stay){
				var p = KC2HandsFreeMouse.GetPointSensor();
				KC2HandsFreeMouse.SetRangePoint(0, p.X, p.Y);
				dialog.SetText("画面の右上を見てください");
				dialog.SetImage((BitmapImage)Application.Current.Resources["IPC_RT"]);
				stopwatch.Restart();
				step = 4;

			}
		}

		void Step4(){
			if(step==4&& !dialog.IsMoving && 1000 < stopwatch.ElapsedMilliseconds)
			{
				Vec2d t = new Vec2d();
				t.X = sr.right - dialog.ActualWidth;
				t.Y = sr.top;
				dialog.SetMoveAnime(t, 1000);
				stopwatch.Restart();
				step = 5;
			}
		}

		void Step5()
		{
			if (step == 5 && !dialog.IsMoving && 1000 < stopwatch.ElapsedMilliseconds && stay)
			{
				var p = KC2HandsFreeMouse.GetPointSensor();
				KC2HandsFreeMouse.SetRangePoint(1, p.X, p.Y);


				dialog.SetText("画面の右下を見てください");
				dialog.SetImage((BitmapImage)Application.Current.Resources["IPC_RB"]);
				stopwatch.Restart();
				step = 6;

			}
		}

		void Step6()
		{
			if (step == 6 && !dialog.IsMoving && 1000 < stopwatch.ElapsedMilliseconds)
			{
				Vec2d t = new Vec2d();
				t.X = sr.right - dialog.ActualWidth;
				t.Y = sr.bottom - dialog.Height;
				dialog.SetMoveAnime(t, 1000);
				stopwatch.Restart();

				step = 7;
			}
		}


		void Step7()
		{
			if (step == 7 && !dialog.IsMoving && 1000 < stopwatch.ElapsedMilliseconds && stay)
			{
				var p = KC2HandsFreeMouse.GetPointSensor();
				KC2HandsFreeMouse.SetRangePoint(2, p.X, p.Y);


				dialog.SetText("画面の左下を見てください");
				dialog.SetImage((BitmapImage)Application.Current.Resources["IPC_LB"]);
				stopwatch.Restart();
				step = 8;

			}
		}

		void Step8()
		{
			if (step == 8 &&  1000 < stopwatch.ElapsedMilliseconds)
			{
				Vec2d t = new Vec2d();
				t.X = sr.left;
				t.Y = sr.bottom - dialog.Height;
				dialog.SetMoveAnime(t, 1000);
				step = 9;
			}
		}

		void Step9(){
			if(step==9 && !dialog.IsMoving && stay){
				var p = KC2HandsFreeMouse.GetPointSensor();
				KC2HandsFreeMouse.SetRangePoint(3, p.X, p.Y);

				Vec2d t = new Vec2d();
				t.X = (sr.right - sr.left - dialog.ActualWidth) * .5;
				t.Y = (sr.bottom - sr.top - dialog.ActualHeight) * .5;

				dialog.SetText("完了");
				dialog.SetImage((BitmapImage)Application.Current.Resources["IPC"]);
				dialog.SetMoveAnime(t, 500);
				step = 10;
				stopwatch.Restart();
			}
		}

		void Step10(){
			if(step==10 && !dialog.IsMoving && 1500 < stopwatch.ElapsedMilliseconds){
				dialog.Close();
			}
		}


	}
}
