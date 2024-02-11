using KC2.DataStructs;
using System.Configuration;
using System.Data;
using System.Windows;

namespace KC2
{
	/// <summary>
	/// Interaction logic for App.xaml
	/// </summary>
	public partial class App : Application
	{
		public SaveData SaveData { get; private set; }
		SaveData PreSaveData;


		public void SetSaveData(SaveData data){
			SaveData = data;
		}

		private void Application_Startup(object sender, StartupEventArgs e)
		{
			SaveData = SaveData.Load();
			PreSaveData = SaveData;
		}

		private void Application_Exit(object sender, ExitEventArgs e)
		{
			if(!SaveData.Equals(PreSaveData)) {
				SaveData.Save();
			}
		}
	}

}
