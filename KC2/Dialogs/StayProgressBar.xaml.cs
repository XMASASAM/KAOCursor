using KC2NativeWrapper;
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
using System.Windows.Shapes;
using System.Windows.Threading;

namespace KC2.Dialogs
{
	/// <summary>
	/// StayProgressBar.xaml の相互作用ロジック
	/// </summary>
	public partial class StayProgressBar : Window
	{

		DispatcherTimer timer = new DispatcherTimer();
		bool flg_first_click = true;
		bool flg_second_stay = true;
		MouseClickController Controller;
		long second_time_limit = 2000;
		long second_time_limit_offset;

		public StayProgressBar(MouseClickController controller)
		{
			InitializeComponent();
			this.DataContext = this;
			Controller = controller;

			timer.Interval = new TimeSpan(0, 0, 0, 0, 1);
			timer.Tick += new EventHandler(Update);
			timer.Start();

			var config = ((App)Application.Current).SaveData;
			if(!config.IsEnableShowCircleProgressBar){
				Visibility = Visibility.Hidden;
			}
		}
		void Update(object? sender, EventArgs e)
		{
			Win32Mouse.GetCursorPos(out var p);
			this.Left = p.X - ActualWidth * .5;
			this.Top = p.Y - ActualHeight * .5;
			var pg = KC2HandsFreeMouse.GetCursorClickProgress(out var stay_time);
			bar.SetProgress(pg);
			//SetDegree(359.9*pg);
			if (1.0 <= pg)
			{
				bar.CColor = new SolidColorBrush(Colors.Green);
				if(flg_first_click){
					Controller.CallClickCursorEvent();
					flg_first_click=false;
					second_time_limit_offset = stay_time;// + second_time_limit;
				}
			}
			else
			{
				bar.CColor = new SolidColorBrush(Colors.Red);
				flg_first_click = true;
				bar2.SetProgress(0.0);
			}



			if (second_time_limit_offset + second_time_limit < stay_time)
			{
				
				if (flg_second_stay)
				{
					bar2.SetProgress(1.0);

					Controller.CallMoveEvent(); 
					flg_second_stay = false;

				}
			}
			else
			{
				flg_second_stay = true;

				if(!flg_first_click){
					double pg2 = (stay_time - second_time_limit_offset)/(double)second_time_limit;
					bar2.SetProgress(Math.Min(pg2,1.0));

				}

			}

			Topmost = false;
			Topmost = true;
		}

	}
}
