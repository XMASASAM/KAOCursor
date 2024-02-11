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

		public List<double> AngleList { get; private set; } = new List<double>(){0,90,180,270};


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
				CameraPreviewView.StartCapture(index);
				CameraPreviewView.Angle = config.CameraAngle;
				CameraPreviewView.ScaleX = config.CameraScaleX;
				CameraPreviewView.ScaleY = config.CameraScaleY;
			}
			KC2DeviceInformation.Init();
			DeviceList = KC2DeviceInformation.DeviceList;
	//		this.config = mainPage.Config;
//			KC2DeviceInformation.DeviceList[0].GetVideoInfo(0).
		}

		private void ComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
		{
			ComboBox comboBox = sender as ComboBox;
			if (comboBox != null)
			{
				int index = comboBox.SelectedIndex;
				if (index != -1)
				{
					mainPage.ChangeCaptureView(index);

					CameraPreviewView.StartCapture(index);
					ResolutionList.Clear();
					var dinfo = KC2DeviceInformation.DeviceList[index];
					Dictionary<Vec2I,VideoInfo> dc = new Dictionary<Vec2I,VideoInfo>();
					for (int i = 0; i < dinfo.VideoInfoListSize; i++)
					{
						var vinfo = dinfo.GetVideoInfo(i);
						Vec2I vs = new Vec2I(vinfo.Width , vinfo.Height);
						if(!dc.ContainsKey(vs)){
							dc.Add(vs, vinfo );
						}
						if(dc[vs].FPS < vinfo.FPS)dc[vs] = vinfo;
					}

					foreach(var i in dc){
						ResolutionList.Add(i.Value);
					}

					ResolutionComboBox.Items.Refresh();
					config.DeviceName = dinfo.Name;
					config.DevicePath = dinfo.Path;
					//config.DeviceSize = new Vec2I(-1,-1);

				}
			}
		}

		private void ResolutionComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
		{
			ComboBox cb = sender as ComboBox;
			if (cb != null) {
				if (cb.SelectedIndex == -1) return;
				VideoInfo vi = (VideoInfo)cb.SelectedItem;
				config.DeviceSize = new(vi.Width, vi.Height);
				
				mainPage.ReleaseCaptureView();
				CameraPreviewView.ReleaseCapture();
				
				CaptureProperty prop = new CaptureProperty {
					Width = vi.Width, Height = vi.Height, FlagFpsOption = 1
				};
				mainPage.ChangeCaptureView(DeviceComboBox.SelectedIndex, prop);
				CameraPreviewView.StartCapture(DeviceComboBox.SelectedIndex,prop);

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
				config.CameraAngle = CameraPreviewView.Angle;
				config.CameraScaleX = CameraPreviewView.ScaleX;
				config.CameraScaleY = CameraPreviewView.ScaleY;
				if (!config.Equals(app.SaveData))
				{
					
					app.SetSaveData(config);
				}
				CameraPreviewView.ReleaseCapture();
			}
		}

		private void TransposeComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
		{
			double d = (double)(TransposeComboBox.SelectedItem);
			CameraPreviewView.Angle = d;
        }

		private void TurnComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
		{
			if(TurnComboBox.SelectedIndex==0) {
				CameraPreviewView.ScaleX = -1;
			}
			if(TurnComboBox.SelectedIndex==1) {
				CameraPreviewView.ScaleX = 1;
			}
		}
	}
}
