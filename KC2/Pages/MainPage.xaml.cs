using KC2.DataStructs;
using KC2.Dialogs;
using KC2NativeWrapper;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Security.Cryptography;
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

namespace KC2.Pages
{
	/// <summary>
	/// MainPage.xaml の相互作用ロジック
	/// </summary>
	public partial class MainPage : Page
	{
		Dialogs.MouseClickController? MouseClickControllerDialog;
		public MainPage()
		{
			InitializeComponent();
			//CaptureView.StartCapture(0);
			App ss = (App)App.Current;
			SaveData data = ss.SaveData;
			if (!string.IsNullOrEmpty(data.DeviceName))
			{
				int index = KC2DeviceInformation.GetIndex(data.DeviceName, data.DevicePath);
				if (index != -1)
				{
					CaptureProperty prop = data.CaptureProperty;
					prop.FlagFpsOption = 1;
					//ChangeCaptureView(index,prop);
					CaptureDevice.Open(index, prop);
				}

			}

		}



		private void ConfigButton_Click(object sender, RoutedEventArgs e)
		{
		//	CaptureView.ReleaseCapture();
			NavigationService.Navigate(new Pages.ConfigPage(this));
		}


		private void Page_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
		{
			bool visible = (bool)e.NewValue;
			if(visible){
				CaptureView.Start();
			}else{
				//CaptureView.StopCapture();
				CaptureView.Stop();
			}

		}


		public void ChangeCaptureView(int index, CaptureProperty prop){
			//CaptureView.StartCapture(index,prop);
		}
		public void ChangeCaptureView(int index)
		{
			//CaptureProperty prop = new CaptureProperty{ Width=640,Height=480,FlagFpsOption=1};
			//ChangeCaptureView(index,prop);
		}

		public void ReleaseCaptureView(){
			//CaptureView.ReleaseCapture();
		}

		private void StartButton_Click(object sender, RoutedEventArgs e)
		{
			var config = ((App)Application.Current).SaveData;

			if (KC2HandsFreeMouse.IsActive)
			{
				//VideoCaptureDevice.ReleaseHandsFreeMouse();
				CaptureDevice.ReleaseHandsFreeMouse();
				if (MouseClickControllerDialog != null)
				{
					MouseClickControllerDialog.Close();
					MouseClickControllerDialog = null;
				}
				StartButton.Content = "Start";
			}
			else
			{
				CaptureDevice.StartHandsFreeMouse();
				if(KC2HandsFreeMouse.IsActive){
					StartButton.Content = "Stop";

					KC2HandsFreeMouse.SetEnableClick(config.IsEnableClick);
					if (config.IsEnableClick)
					{
						ShowMouseClickController();
					}
				}
				//if (VideoCaptureDevice.StartHandsFreeMouse(CaptureView.GetVideoCaptureDevice(),0) == 1)
				//{
				//	StartButton.Content = "Stop";
				//}
			}
        }

		private void RangeButton_Click(object sender, RoutedEventArgs e)
		{
			NavigationService.Navigate(new Pages.RangePage(this));
		}

		public void Close(){
			CloseMouseClickController();
		}

		public void ShowMouseClickController(){
			if (MouseClickControllerDialog == null && KC2HandsFreeMouse.IsActive)
			{
				MouseClickControllerDialog = new Dialogs.MouseClickController();
				MouseClickControllerDialog.Show();
			}
		}

		public void CloseMouseClickController(){

			if (MouseClickControllerDialog != null)
			{
				MouseClickControllerDialog.Close();
				MouseClickControllerDialog = null;
			}

		}

	}
}
