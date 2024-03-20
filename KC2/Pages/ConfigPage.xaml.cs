using KC2.Controls;
using KC2.DataStructs;
using KC2NativeWrapper;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Ribbon;
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
	/// ConfigPage.xaml の相互作用ロジック
	/// </summary>
	public partial class ConfigPage : Page
	{
		public static readonly DependencyProperty DeviceListProperty =
			DependencyProperty.Register(nameof(DeviceList), typeof(List<DeviceInfo>), typeof(ConfigPage), new PropertyMetadata(null));
		public List<DeviceInfo> DeviceList
		{
			get { return (List<DeviceInfo>)GetValue(DeviceListProperty); }
			private set { SetValue(DeviceListProperty, value); }
		}

		public static readonly DependencyProperty ResolutionListProperty =
			DependencyProperty.Register(nameof(ResolutionList), typeof(List<VideoInfo>), typeof(ConfigPage), new PropertyMetadata(new List<VideoInfo>()));
		public List<VideoInfo> ResolutionList
		{
			get { return (List<VideoInfo>)GetValue(ResolutionListProperty); }
			private set { SetValue(ResolutionListProperty, value); }
		}

		public List<int> AngleList { get; private set; } = new List<int>(){0,90,180,270};


		SaveData config;
		SaveData pre_config;
		MainPage mainPage;
		public ConfigPage(MainPage mainPage)
		{
			this.mainPage = mainPage;

			InitializeComponent();
			config = ((App)Application.Current).SaveData;

			int index = KC2DeviceInformation.GetIndex(config.DeviceName, config.DevicePath);
			if (0 <= index)
			{
				DeviceComboBox.SelectedIndex = index;
			//	CameraPreviewView.StartCapture(index);
			}
			KC2DeviceInformation.Init();
			DeviceList = KC2DeviceInformation.DeviceList;
			EnableClickComboBox.SelectedIndex = Convert.ToInt32(config.IsEnableClick); //Convert.ToInt32(KC2HandsFreeMouse.GetEnableClick());

		}
		private void Page_Loaded(object sender, RoutedEventArgs e)
		{

			//LicenseTab.Margin = new Thickness(tabItemWidth.ActualWidth,0,0,0);
			
			//			ResolutionComboBox.SelectedIndex = ResolutionList.Find()
			ApplyResolution();

			TransposeComboBox.SelectedIndex = AngleList.IndexOf(config.CaptureProperty.Angle);
			TurnComboBox.SelectedIndex = config.CaptureProperty.HorizonFlip;

			HansFreeMouseProperty hp = config.HansFreeMouseProperty;
			LowPathRateView.SetCurrentValue(hp.LowPathRate, 2);
			CursorMovePixelThresholdView.SetCurrentValue(hp.CursorMovePixelThreshold, 1);
			CursorMoveMultiplierView.SetCurrentValue(hp.CursorMoveMultiplier, 1);
			CursorMoveXFactorView.SetCurrentValue(hp.CursorMoveXFactor, 1);
			CursorMoveYFactorView.SetCurrentValue(hp.CursorMoveYFactor, 1);
			DetectCursorStayMillisecondTimeThresholdView.SetCurrentValue(hp.DetectCursorStayMillisecondTimeThreshold,0);
			MouseClickHoldMillisecondTimeView.SetCurrentValue(hp.MouseClickHoldMillisecondTime, 0);
			MouseDoubleClickUnPressMillisecondTimeView.SetCurrentValue(hp.MouseDoubleClickUnPressMillisecondTime, 0);
			MouseWheelScrollAmountView.SetCurrentValue(hp.MouseWheelScrollAmount, 0);
			MouseWheelScrollIntervalMillisecondTimeView.SetCurrentValue(hp.MouseWheelScrollIntervalMillisecondTime, 0);
		}

		void LoadHansFreeMousePropertyFromUI(){
			HansFreeMouseProperty hp = config.HansFreeMouseProperty;
			hp.LowPathRate = LowPathRateView.GetCurrentValue(2);
			hp.CursorMovePixelThreshold = CursorMovePixelThresholdView.GetCurrentValue(1);
			hp.CursorMoveMultiplier = CursorMoveMultiplierView.GetCurrentValue(1);
			hp.CursorMoveXFactor = CursorMoveXFactorView.GetCurrentValue(1);
			hp.CursorMoveYFactor = CursorMoveYFactorView.GetCurrentValue(1);
			hp.DetectCursorStayMillisecondTimeThreshold = DetectCursorStayMillisecondTimeThresholdView.GetCurrentValueInt(0);
			hp.MouseClickHoldMillisecondTime = MouseClickHoldMillisecondTimeView.GetCurrentValueInt(0);
			hp.MouseDoubleClickUnPressMillisecondTime = MouseDoubleClickUnPressMillisecondTimeView.GetCurrentValueInt(0);
			hp.MouseWheelScrollAmount = MouseWheelScrollAmountView.GetCurrentValueInt(0);
			hp.MouseWheelScrollIntervalMillisecondTime = MouseWheelScrollIntervalMillisecondTimeView.GetCurrentValueInt(0);
			config.HansFreeMouseProperty = hp;
		}


		void ApplyResolution(){
			//VideoCaptureDevice? d = CameraPreviewView.GetVideoCaptureDevice();
			
			if (!CaptureDevice.IsOpened) return;
			CaptureProperty cap = CaptureDevice.Property;
			for (int i = 0; i < ResolutionList.Count(); i++)
			{
				var j = ResolutionList[i];
				if (j.Width == cap.Width && j.Height == cap.Height)
				{
					ResolutionComboBox.SelectedIndex = i;
					break;
				}
			}
		}


		private void ComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
		{
			ComboBox comboBox = sender as ComboBox;
			if (comboBox != null)
			{
				int index = comboBox.SelectedIndex;
				if (index != -1)
				{
					//mainPage.ChangeCaptureView(index,config.CaptureProperty);

					//CameraPreviewView.StartCapture(index,config.CaptureProperty);
					if(CaptureDevice.GetCurrentIndex()!=index)
						CaptureDevice.Open(index,config.CaptureProperty);

					ResolutionList.Clear();
					var dinfo = KC2DeviceInformation.DeviceList[index];
					Dictionary<Vec2I, VideoInfo> dic = new Dictionary<Vec2I, VideoInfo>();
					for (int i = 0; i < dinfo.VideoInfoListSize; i++)
					{
						var vinfo = dinfo.GetVideoInfo(i);
						Vec2I s = new(vinfo.Width, vinfo.Height);
						if (dic.ContainsKey(s))
						{
							if (vinfo.FPS < dic[s].FPS) continue;
							dic[s] = vinfo;
						}
						else
						{
							dic.Add(s, vinfo);
						}
					}

					foreach (var i in dic){
						ResolutionList.Add(i.Value);
					}
					ResolutionComboBox.Items.Refresh();
					config.DeviceName = dinfo.Name;
					config.DevicePath = dinfo.Path;

					ApplyResolution();
				}
			}
		}

		private void ResolutionComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
		{
			ComboBox cb = sender as ComboBox;

			 

			if (cb != null) {
				if (cb.SelectedIndex == -1) return;
				VideoInfo vi = (VideoInfo)cb.SelectedItem;

				if(vi.Width==config.CaptureProperty.Width && vi.Height==config.CaptureProperty.Height) {
					return;
				}

				//config.DeviceSize = new(vi.Width, vi.Height);
				
				//mainPage.ReleaseCaptureView();
				//CameraPreviewView.ReleaseCapture();

				CaptureProperty prop = config.CaptureProperty;//new CaptureProperty {
															  //	Width = vi.Width, Height = vi.Height, FlagFpsOption = 1
															  //};
				prop.Width = vi.Width;
				prop.Height = vi.Height;
				
				config.CaptureProperty = prop;


				//mainPage.ChangeCaptureView(DeviceComboBox.SelectedIndex, prop);
				//CameraPreviewView.StartCapture(DeviceComboBox.SelectedIndex,prop);
				CaptureDevice.Open(DeviceComboBox.SelectedIndex, prop);
			}
		}

		private void ReturnButton_Click(object sender, RoutedEventArgs e)
		{
			NavigationService.GoBack();
		}

		private void Page_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
		{
			bool visible = (bool)e.NewValue;
			if(visible){

			}else{
				App app = (App)Application.Current;
				LoadHansFreeMousePropertyFromUI();
				if (!config.Equals(app.SaveData))
				{
					
					app.SetSaveData(config);
				}
				//CameraPreviewView.ReleaseCapture();
				CameraPreviewView.Stop();
			}
		}

		private void TransposeComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
		{
			int d = (int)(TransposeComboBox.SelectedItem);
			var prop = config.CaptureProperty;

			if (prop.Angle == d) return;

			prop.Angle = d;
			config.CaptureProperty = prop;
			//CameraPreviewView.GetVideoCaptureDevice().ChangeFlip(prop.Angle,prop.HorizonFlip);
			CaptureDevice.ChangeFlip(prop.Angle, prop.HorizonFlip);
		}

		private void TurnComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
		{

			int d = TurnComboBox.SelectedIndex;
			var prop = config.CaptureProperty;

			if(prop.HorizonFlip == d) return;

			prop.HorizonFlip = d;
			config.CaptureProperty = prop;
			CaptureDevice.ChangeFlip(prop.Angle, prop.HorizonFlip);

			//CameraPreviewView.GetVideoCaptureDevice().ChangeFlip(prop.Angle, prop.HorizonFlip);
			/*
				if(TurnComboBox.SelectedIndex==0) {
					CameraPreviewView.ScaleX = -1;
				}
				if(TurnComboBox.SelectedIndex==1) {
					CameraPreviewView.ScaleX = 1;
				}*/
		}

		private void EnableClickComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
		{
			int d = EnableClickComboBox.SelectedIndex;
			if(d==0){//OFF
				config.IsEnableClick = false;
				KC2HandsFreeMouse.SetEnableClick(false);
				if(KC2HandsFreeMouse.IsActive){
					mainPage.CloseMouseClickController();
				}
			}
			if(d==1){//ON
				config.IsEnableClick = true;
				KC2HandsFreeMouse.SetEnableClick(true);
				if(KC2HandsFreeMouse.IsActive)mainPage.ShowMouseClickController();

			}
		}

		private void Hyperlink_Click(object sender, RoutedEventArgs e)
		{
			var s = (Hyperlink)sender;
			var destinationurl = s.NavigateUri.ToString();//"https://www.bing.com/";
			var sInfo = new System.Diagnostics.ProcessStartInfo(destinationurl)
			{
				UseShellExecute = true,
			};
			System.Diagnostics.Process.Start(sInfo);
			e.Handled = true;
		}
    }
}
