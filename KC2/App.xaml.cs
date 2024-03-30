using KC2.DataStructs;
using KC2NativeWrapper;
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
			ApplySaveData();
		}
		void ApplySaveData(){
			KC2HandsFreeMouse.SetEnablePlaySE(Convert.ToInt16(SaveData.IsEnablePlaySE));
			KC2HandsFreeMouse.SetPropety(SaveData.HansFreeMouseProperty);

			KC2HandsFreeMouse.ClearRangePoint();
			if (SaveData.HasFaceRange)
			{
				KC2HandsFreeMouse.SetScreenRect(SaveData.ScreenRect);

				for (int i = 0; i < 4; i++)
				{
					var r = SaveData.FaceRange[i];
					KC2HandsFreeMouse.SetRangePoint(i, r.X, r.Y);
				}
			}
		}

		private void Application_Startup(object sender, StartupEventArgs e)
		{
			SaveData = SaveData.Load();
			PreSaveData = SaveData;
			ApplySaveData();

		}

		private void Application_Exit(object sender, ExitEventArgs e)
		{
			if(!SaveData.Equals(PreSaveData)) {
				SaveData.Save();
			}
		}
	}

}
