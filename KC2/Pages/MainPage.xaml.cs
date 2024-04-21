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
using System.Windows.Threading;


namespace KC2.Pages
{
	/// <summary>
	/// MainPage.xaml の相互作用ロジック
	/// </summary>
	public partial class MainPage : Page
	{
		Dialogs.MouseClickController? MouseClickControllerDialog;
		MainWindow MainWindow;
		public MainPage()
		{
			InitializeComponent();
			//CaptureView.StartCapture(0);


		}
		public void SetMainWindow(MainWindow main){
			MainWindow = main;
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
					ShowLoadOverLay();
					DispatcherTimer t = new DispatcherTimer();
					t.Tick += (s, e) => { CloseLoadOverLay(); t.Stop(); };
					t.Interval = new TimeSpan(0);
					Thread open_thread = new Thread(new ThreadStart(() =>
					{
						CaptureDevice.Open(index, prop);

						t.Start();
					}));
					open_thread.Start();
				}

			}
		}

		private void Page_Loaded(object sender, RoutedEventArgs e)
		{
			App app = (App)App.Current;
			if (app.SaveData.NeedTutorial)
			{
				NavigationService.Navigate(new Pages.SelectCameraPage(this));
				e.Handled = true;
			}else{
				if(StartButton.Visibility == Visibility.Visible){
					if(KC2HandsFreeMouse.IsActive){
						StartHansFreeMouse();
					}
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

				StopHansFreeMouse();

			}
			else 
			{
				StartHansFreeMouse();
			}
        }

		private void RangeButton_Click(object sender, RoutedEventArgs e)
		{
			if (CaptureDevice.IsOpened)
			{
				NavigationService.Navigate(new Pages.RangePage(this));

			}else{
				string messageBoxText = "カメラを設定して下さい。";
				string caption = "メッセージ";
				MessageBoxButton button = MessageBoxButton.OK;
				MessageBoxImage icon = MessageBoxImage.Warning;
				MessageBoxResult result;

				result = MessageBox.Show(messageBoxText, caption, button, icon, MessageBoxResult.Yes);
			}
		}

		public void Close(){
			CloseMouseClickController();
		}

		public void ShowMouseClickController(){
			if (MouseClickControllerDialog == null && KC2HandsFreeMouse.IsActive)
			{
				MouseClickControllerDialog = new Dialogs.MouseClickController(this);
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
		public void StartHansFreeMouse(){
			var config = ((App)Application.Current).SaveData;

			if (CaptureDevice.IsOpened)
			{


				if (CaptureDevice.GetID().Equals(config.FaceRangeDeviceID))
				{

					StartButtonContent.Visibility = Visibility.Hidden;
					StopButtonContent.Visibility = Visibility.Visible;
					CaptureDevice.StartHandsFreeMouse();
					if (KC2HandsFreeMouse.IsActive)
					{

						//							StartButton.Content = "Stop";

						KC2HandsFreeMouse.SetEnableClick(config.IsEnableClick);
						if (config.IsEnableClick)
						{
							ShowMouseClickController();
						}
					}
				}
				else
				{
					string messageBoxText = "可動域を設定して下さい。";
					string caption = "メッセージ";
					MessageBoxButton button = MessageBoxButton.OK;
					MessageBoxImage icon = MessageBoxImage.Warning;
					MessageBoxResult result;

					result = MessageBox.Show(messageBoxText, caption, button, icon, MessageBoxResult.Yes);
					Console.WriteLine("wlwlwlwl");
				}

			}
			else
			{
				string messageBoxText = "カメラを設定して下さい。";
				string caption = "メッセージ";
				MessageBoxButton button = MessageBoxButton.OK;
				MessageBoxImage icon = MessageBoxImage.Warning;
				MessageBoxResult result;

				result = MessageBox.Show(messageBoxText, caption, button, icon, MessageBoxResult.Yes);
			}
			//if (VideoCaptureDevice.StartHandsFreeMouse(CaptureView.GetVideoCaptureDevice(),0) == 1)
			//{
			//	StartButton.Content = "Stop";
			//}

		}
		public void StopHansFreeMouse(){
			CaptureDevice.ReleaseHandsFreeMouse();
			if (MouseClickControllerDialog != null)
			{
				MouseClickControllerDialog.Close();
				MouseClickControllerDialog = null;
			}
			StartButtonContent.Visibility = Visibility.Visible;
			StopButtonContent.Visibility = Visibility.Hidden;
		}

		public void SetActiveWindow(){
			MainWindow.Topmost = true;
			MainWindow.Topmost = false;
			if(!MainWindow.WindowState.Equals(WindowState.Maximized)){
				MainWindow.WindowState = WindowState.Normal;

			}
		}


		public void ShowLoadOverLay()
		{
			MainWindow.ShowLoadOverLay();
		}
		public void CloseLoadOverLay()
		{
			 MainWindow.CloseLoadOverLay();
		}


	}
}
