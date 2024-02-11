using KC2.DataStructs;
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
		public MainPage()
		{
			InitializeComponent();
			//CaptureView.StartCapture(0);
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
				App ss = (App)App.Current;
				SaveData data = ss.SaveData;
				if(!string.IsNullOrEmpty(data.DeviceName)){
					int index =  KC2DeviceInformation.GetIndex(data.DeviceName, data.DevicePath);
					if(index!=-1){
						CaptureProperty prop = new CaptureProperty {
							Width = data.DeviceSize.X,
							Height = data.DeviceSize.Y
						};
						prop.FlagFpsOption = 1;
						ChangeCaptureView(index,prop);
						CaptureView.Angle = data.CameraAngle;
						CaptureView.ScaleX = data.CameraScaleX;
						CaptureView.ScaleY = data.CameraScaleY;
					}

				}
			}else{
				CaptureView.StopCapture();
				//CaptureView.ReleaseCapture();
			}

		}


		public void ChangeCaptureView(int index, CaptureProperty prop){
			CaptureView.StartCapture(index,prop);
		}
		public void ChangeCaptureView(int index)
		{
			CaptureProperty prop = new CaptureProperty{ Width=640,Height=480,FlagFpsOption=1};
			ChangeCaptureView(index,prop);
		}

		public void ReleaseCaptureView(){
			CaptureView.ReleaseCapture();
		}

	}
}
