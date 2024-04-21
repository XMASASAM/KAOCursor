using KC2.Pages;
using KC2NativeWrapper;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Threading;

namespace KC2.DataStructs
{
	internal class ConfigCameraProcess
	{
		SaveData config;
		ComboBox DeviceComboBox;
		ComboBox ResolutionComboBox;
		ComboBox TransposeComboBox;
		ComboBox TurnComboBox;
		MainPage mainPage;
		public Action? LoadedCaptureDevice { get; set; } = null;
		public Action? CloseCaptureDevice { get; set; } = null;
		public List<int> AngleList { get; private set; } = new List<int>() { 0, 90, 180, 270 };

		public List<DeviceInfo> DeviceList = new List<DeviceInfo>();
		public List<VideoInfo> ResolutionList = new List<VideoInfo>();

		public ConfigCameraProcess(MainPage mainPage, ComboBox select_camera, ComboBox select_screensize, ComboBox select_rotation, ComboBox select_hflip, Controls.CaptureDeviceView deviceView)
		{
			select_camera.SelectionChanged += ComboBox_SelectionChanged;
			select_screensize.SelectionChanged += ResolutionComboBox_SelectionChanged;
			select_rotation.SelectionChanged += TransposeComboBox_SelectionChanged;
			select_hflip.SelectionChanged += TurnComboBox_SelectionChanged;



			config = ((App)Application.Current).SaveData;
			DeviceComboBox = select_camera;
			ResolutionComboBox = select_screensize;
			TransposeComboBox = select_rotation;
			TurnComboBox = select_hflip;
			this.mainPage = mainPage;

			int index = KC2DeviceInformation.GetIndex(config.DeviceName, config.DevicePath);
			if (0 <= index)
			{
				DeviceComboBox.SelectedIndex = index;
				//	CameraPreviewView.StartCapture(index);
			}
			KC2DeviceInformation.Init();
			DeviceList = KC2DeviceInformation.DeviceList;


			select_camera.ItemsSource = DeviceList;
			select_screensize.ItemsSource = ResolutionList;
			select_rotation.ItemsSource = AngleList;
			select_hflip.ItemsSource = new List<string>{"OFF", "ON"};
		}
		public void Loaded(){
			ApplyResolution();

			TransposeComboBox.SelectedIndex = AngleList.IndexOf(config.CaptureProperty.Angle);
			TurnComboBox.SelectedIndex = config.CaptureProperty.HorizonFlip;
		}

		public void SaveConfig(){
			App app = (App)Application.Current;
			app.SetSaveData(config);
		}

		bool CheckHFM()
		{
			if (CloseCaptureDevice != null) CloseCaptureDevice.Invoke();

			if (KC2HandsFreeMouse.IsActive)
			{
				string messageBoxText = "ハンズフリーマウスの起動中にカメラ設定の変更はできません。";
				string caption = "メッセージ";
				MessageBoxButton button = MessageBoxButton.OK;
				MessageBoxImage icon = MessageBoxImage.Warning;
				MessageBoxResult result;

				result = MessageBox.Show(messageBoxText, caption, button, icon, MessageBoxResult.Yes);
				return true;
			}
			return false;
		}
		void ShowLoadOverLay()
		{
			mainPage.ShowLoadOverLay();
		}
		void CloseLoadOverLay()
		{
			mainPage.CloseLoadOverLay();
		}

		void OpenDevice(int index,in CaptureProperty prop){
			//if (CloseCaptureDevice != null) CloseCaptureDevice.Invoke();

			CaptureDevice.Open(index, prop);
			if (LoadedCaptureDevice != null) LoadedCaptureDevice.Invoke();
		}

		void ApplyResolution()
		{
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
					if (CaptureDevice.GetCurrentIndex() != index)
					{

						if (CheckHFM())
						{
							index = KC2DeviceInformation.GetIndex(config.DeviceName, config.DevicePath);
							if (0 <= index)
							{
								DeviceComboBox.SelectedIndex = index;
								//	CameraPreviewView.StartCapture(index);
							}
							e.Handled = true;
							return;
						}
						ShowLoadOverLay();

					}

					DispatcherTimer open_timer = new DispatcherTimer();

					Thread open_thread = new Thread(new ThreadStart(() => {
						if (CaptureDevice.GetCurrentIndex() != index)
						{
							OpenDevice(index,config.CaptureProperty);
						//	CaptureDevice.Open(index, config.CaptureProperty);

						}
						open_timer.Start();
					}));
					open_timer.Interval = new TimeSpan(0);
					open_timer.Tick += new EventHandler((es, ee) => {

						CloseLoadOverLay();

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

						foreach (var i in dic)
						{
							ResolutionList.Add(i.Value);
						}
						ResolutionComboBox.Items.Refresh();
						config.DeviceName = dinfo.Name;
						config.DevicePath = dinfo.Path;

						ApplyResolution();
						open_timer.Stop();
					});
					open_thread.Start();

				}
			}
		}

		private void ResolutionComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
		{
			ComboBox cb = sender as ComboBox;



			if (cb != null)
			{
				if (cb.SelectedIndex == -1) return;
				VideoInfo vi = (VideoInfo)cb.SelectedItem;

				if (vi.Width == config.CaptureProperty.Width && vi.Height == config.CaptureProperty.Height)
				{
					return;
				}

				if (CheckHFM())
				{
					e.Handled = true;
					ApplyResolution();
					return;
				}

				CaptureProperty prop = config.CaptureProperty;//new CaptureProperty {
															  //	Width = vi.Width, Height = vi.Height, FlagFpsOption = 1
															  //};
				prop.Width = vi.Width;
				prop.Height = vi.Height;

				config.CaptureProperty = prop;


				ShowLoadOverLay();
				DispatcherTimer open_timer = new DispatcherTimer();
				int dindex = DeviceComboBox.SelectedIndex;
				Thread open_thread = new Thread(new ThreadStart(() => {

					OpenDevice(dindex, prop);

					//CaptureDevice.Open(dindex, prop);

					open_timer.Start();
				}));
				open_timer.Tick += new EventHandler((s, e) =>
				{
					CloseLoadOverLay();
					open_timer.Stop();
				});
				open_timer.Interval = new TimeSpan(0);
				open_thread.Start();
			}
		}


		private void TransposeComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
		{

			int d = (int)(TransposeComboBox.SelectedItem);
			var prop = config.CaptureProperty;

			if (prop.Angle == d) return;

			if (CheckHFM())
			{
				e.Handled = true;

				TransposeComboBox.SelectedIndex = AngleList.IndexOf(config.CaptureProperty.Angle);

				return;
			}

			prop.Angle = d;
			config.CaptureProperty = prop;
			//CameraPreviewView.GetVideoCaptureDevice().ChangeFlip(prop.Angle,prop.HorizonFlip);
			CaptureDevice.ChangeFlip(prop.Angle, prop.HorizonFlip);
			if (LoadedCaptureDevice != null) LoadedCaptureDevice.Invoke();

		}

		private void TurnComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
		{

			int d = TurnComboBox.SelectedIndex;
			var prop = config.CaptureProperty;

			if (prop.HorizonFlip == d) return;

			if (CheckHFM())
			{
				e.Handled = true;

				TurnComboBox.SelectedIndex = config.CaptureProperty.HorizonFlip;
				return;
			}

			prop.HorizonFlip = d;
			config.CaptureProperty = prop;
			CaptureDevice.ChangeFlip(prop.Angle, prop.HorizonFlip);
			if (LoadedCaptureDevice != null) LoadedCaptureDevice.Invoke();

		}


	}
}
