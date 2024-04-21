using KC2.DataStructs;
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
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace KC2.Pages
{
	/// <summary>
	/// SelectCameraPage.xaml の相互作用ロジック
	/// </summary>
	public partial class SelectCameraPage : Page
	{
		ConfigCameraProcess camera_process;
		MainPage main_page;
		public SelectCameraPage(MainPage mainPage)
		{
			InitializeComponent();
			camera_process = new ConfigCameraProcess(mainPage,c1,c2,c3,c4,cv1);
			camera_process.LoadedCaptureDevice = LoadedDevice;
			camera_process.CloseCaptureDevice = CloseDevice;
			this.main_page = mainPage;
		}

		private void Page_Loaded(object sender, RoutedEventArgs e)
		{
			camera_process.Loaded();
        }

		void LoadedDevice(){
			CaptureDevice.StartHandsFreeMouse(1);
		}

		void CloseDevice(){
			CaptureDevice.ReleaseHandsFreeMouse();
		}

		private void Button_Click(object sender, RoutedEventArgs e)
		{
			if(KC2HandsFreeMouse.GetSuccessTracking()){
				camera_process.SaveConfig();
				Pages.FaceRangePage2 p = new FaceRangePage2();
				p.SetMainPage(main_page);
				NavigationService.Navigate(p);
			}else{
				MessageBox.Show("顔を認識できないため次のステップへすすめません。","アラート",MessageBoxButton.OK,MessageBoxImage.Warning);
			}
		}

		private void Skip_Button_Click(object sender, RoutedEventArgs e)
		{
			if(MessageBox.Show("本当に初期設定をスキップしますか？","確認",MessageBoxButton.YesNo).Equals(MessageBoxResult.Yes)){

			camera_process.SaveConfig();

			KC2HandsFreeMouse.ReleaseHansFreeMouse();
			App app = (App)Application.Current;

			var config = app.SaveData;
			config.NeedTutorial = false;
			app.SetSaveData(config);

			NavigationService.Navigate(main_page);
			}

		}
	}
}
