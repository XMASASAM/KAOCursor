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
	/// SetumeiPage.xaml の相互作用ロジック
	/// </summary>
	public partial class SetumeiPage : Page
	{
		MainPage mainPage;
		public SetumeiPage()
		{
			InitializeComponent();
		}

		public void SetMainPage(MainPage mainPage)
		{
			this.mainPage = mainPage;

		}

		private void Button_Click(object sender, RoutedEventArgs e)
		{
			App app = (App)Application.Current;

			var config = app.SaveData;
			config.NeedTutorial = false;
			app.SetSaveData(config);

			NavigationService.Navigate(mainPage);

		}
	}
}
