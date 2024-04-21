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
	/// FaceRangePage2.xaml の相互作用ロジック
	/// </summary>
	public partial class FaceRangePage2 : Page
	{
		MainPage mainPage;
		FaceRangeProcess rangeProcess;
		public FaceRangePage2()
		{
			InitializeComponent();
			rangeProcess = new FaceRangeProcess();
			rangeProcess.SetMainPage(mainPage);
			rangeProcess.Begin = ProcessStart;
			rangeProcess.End = ProcessEnd;
		}
		public void SetMainPage(MainPage mainPage){
			this.mainPage = mainPage;

		}

		private void Button_Click(object sender, RoutedEventArgs e)
		{
			//	rangeProcess.SaveFaceRange();
			App app = (App)Application.Current;

			var config = app.SaveData;

			if(CaptureDevice.GetID().Equals(config.FaceRangeDeviceID)){
				//var config = app.SaveData;
				//config.NeedTutorial = false;
				//app.SetSaveData(config);
				Pages.SetumeiPage p = new SetumeiPage();
				p.SetMainPage(mainPage);
				NavigationService.Navigate(p);

			}
			else
			{
				MessageBox.Show("顔の可動域を設定してください", "アラート", MessageBoxButton.OK, MessageBoxImage.Warning);
			}
		}

		private void ReturnButton_Click(object sender, RoutedEventArgs e)
		{
			NavigationService.GoBack();
		}

		private void StartButton_Click(object sender, RoutedEventArgs e)
		{
			rangeProcess.Start(this);

		}

		void ProcessStart(){

			StartButton.Opacity = .5;
			ReturnButton.Opacity = .5;
			NextButton.Opacity = .5;
		}

		void ProcessEnd(){

			StartButton.Opacity = 1.0;
			ReturnButton.Opacity = 1.0;
			NextButton.Opacity = 1.0;
		}

	}
}
