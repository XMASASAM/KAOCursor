using KC2NativeWrapper;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Controls;
using System.Windows.Media.Imaging;
using System.Windows;
using KC2.Pages;
using System.Windows.Threading;

namespace KC2.DataStructs
{
	internal class FaceRangeProcess
	{
		MainPage mainPage;
		public SaveData config;
		DispatcherTimer timer = new DispatcherTimer();
		Vec2d[] range = new Vec2d[4];
		Vec2d[] range_prev = new Vec2d[4];
		Dialogs.NoButtonDialog? dialog;
		int step = 0;
		bool stay = false;
		Stopwatch stopwatch = new Stopwatch();
		Win32Mouse.Win32RECT sr;
		Win32Mouse.Win32RECT sr_prev;
		bool flg_hfm_active_prev = false;
		bool flg_prev_setting = false;
		bool finished = false;
		public Action? Begin { get; set; } = null;
		public Action? End { get; set; } = null;
		public FaceRangeProcess(){


			timer.Interval = new TimeSpan(0, 0, 0, 0, 30);
			timer.Tick += new EventHandler(Update);


		}
		public void SetMainPage(MainPage mainPage){
			this.mainPage = mainPage;
		}

		public void Start(FrameworkElement fe){
			config = ((App)Application.Current).SaveData;

			if (config.HasFaceRange)
			{
				range_prev = config.FaceRange;
				sr_prev = config.ScreenRect;
			}
			flg_hfm_active_prev = KC2HandsFreeMouse.IsActive;
			bool ok = false;

			if (!KC2HandsFreeMouse.IsActive){
				ok = CaptureDevice.StartHandsFreeMouse(1);
				if (!ok) return;
			}

			flg_prev_setting = KC2HandsFreeMouse.GetFlagSettingRange();
			KC2HandsFreeMouse.SetFlagSettingRange(1);
			if (Begin != null){
				Begin.Invoke();
			}
			var p = fe.PointToScreen(new Point(0, 0));
			sr = Win32Mouse.GetCurrentScreenRect((int)p.X, (int)p.Y).all;
			config.ScreenRect = sr;
			KC2HandsFreeMouse.SetScreenRect(sr);
			KC2HandsFreeMouse.ClearRangePoint();
			//ReturnButton.Opacity = .5;
			//StartButton.Opacity = .5;
			step = 0;
			timer.Start();
			dialog = new Dialogs.NoButtonDialog();
			dialog.WindowStyle = WindowStyle.ToolWindow;
			dialog.SetImage((BitmapImage)Application.Current.Resources["IPC_LT"]);
			dialog.SetText("画面の左上を見てください");
			dialog.Closing += new System.ComponentModel.CancelEventHandler(Stop);
			dialog.ShowDialog();
		}

		void Update(object? sender, EventArgs e)
		{

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

		void Step0()
		{
			if (step == 0 && dialog != null)
			{
				if (dialog.IsVisible)
				{
					stopwatch.Restart();
					step = 1;
				}
			}
		}

		void Step1()
		{

			if (step == 1 && 1000 < stopwatch.ElapsedMilliseconds)
			{
				Vec2d t = new Vec2d();
				t.X = sr.left;//(sr.right - sr.left - dialog.ActualWidth) * .5;
				t.Y = sr.top;//(sr.bottom - sr.top - dialog.ActualHeight) * .5;
				dialog.SetMoveAnime(t, 1000);
				step = 2;
			}
		}

		void Step2()
		{
			if (step == 2 && !dialog.IsMoving)
			{
				stopwatch.Restart();
				step = 3;
			}
		}

		void Step3()
		{
			if (step == 3 && !dialog.IsMoving && 1000 < stopwatch.ElapsedMilliseconds && stay)
			{
				var p = KC2HandsFreeMouse.GetPointSensor();
				range[0] = new Vec2d(p.X, p.Y);
				KC2HandsFreeMouse.SetRangePoint(0, p.X, p.Y);
				dialog.SetText("画面の右上を見てください");
				dialog.SetImage((BitmapImage)Application.Current.Resources["IPC_RT"]);
				stopwatch.Restart();
				step = 4;

			}
		}

		void Step4()
		{
			if (step == 4 && !dialog.IsMoving && 1000 < stopwatch.ElapsedMilliseconds)
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
				range[1] = new Vec2d(p.X, p.Y);
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
				range[2] = new Vec2d(p.X, p.Y);
				KC2HandsFreeMouse.SetRangePoint(2, p.X, p.Y);


				dialog.SetText("画面の左下を見てください");
				dialog.SetImage((BitmapImage)Application.Current.Resources["IPC_LB"]);
				stopwatch.Restart();
				step = 8;

			}
		}

		void Step8()
		{
			if (step == 8 && 1000 < stopwatch.ElapsedMilliseconds)
			{
				Vec2d t = new Vec2d();
				t.X = sr.left;
				t.Y = sr.bottom - dialog.Height;
				dialog.SetMoveAnime(t, 1000);
				step = 9;
			}
		}

		void Step9()
		{
			if (step == 9 && !dialog.IsMoving && stay)
			{
				var p = KC2HandsFreeMouse.GetPointSensor();
				range[3] = new Vec2d(p.X, p.Y);
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

		void Step10()
		{
			if (step == 10 && !dialog.IsMoving && 1500 < stopwatch.ElapsedMilliseconds)
			{
				ApplyFraceRange(range);
			//	ReturnButton.Opacity = 1.0;
			//	StartButton.Opacity = 1.0;
				config.FaceRangeDeviceID = CaptureDevice.GetID();
				stopwatch.Stop();
				timer.Stop();
				dialog.Close();
				finished = true;
				SaveFaceRange();
			}
		}

		void ApplyFraceRange(Vec2d[] ra)
		{
			KC2HandsFreeMouse.ClearRangePoint();
			for (int i = 0; i < 4; i++)
			{
				var r = ra[i];
				KC2HandsFreeMouse.SetRangePoint(i, r.X, r.Y);
			}
//			if (flg_hfm_active_prev)
	//		{
				KC2HandsFreeMouse.SetFlagSettingRange(flg_prev_setting?1:0);
		//	}
			config.FaceRange = ra;
			config.HasFaceRange = true;

		}
		
		public void SaveFaceRange()
		{
			/*
				if(!CaptureDevice.GetID().Equals(config.FaceRangeDeviceID)){
					return;
				}*/
			if (!finished) return;
			var app = ((App)Application.Current);

			if (!config.Equals(app.SaveData))
			{
				app.SetSaveData(config);
			}
		}

		void Stop(object? sender, CancelEventArgs e)
		{
			if (config.HasFaceRange)
			{
				ApplyFraceRange(config.FaceRange);
			}
			//ReturnButton.Opacity = 1.0;
			//StartButton.Opacity = 1.0;
			if(End!=null){
				End.Invoke();
			}
			stopwatch.Stop();
			timer.Stop();
		}
	}
}
