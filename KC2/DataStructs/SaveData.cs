using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Linq;
using System.Text;
using System.IO;
using System.Text.Json;
using System.Threading.Tasks;
using KC2NativeWrapper;

namespace KC2.DataStructs
{
	public struct SaveData
	{

		public static readonly string SavePath;

		public string DeviceName{ get; set; }
		public string DevicePath{ get; set; }

		public CaptureProperty CaptureProperty{ get; set; }

		public bool IsEnableClick{ get; set; }

		public HansFreeMouseProperty HansFreeMouseProperty{ get; set; }

		public void SetDefault(){
			DeviceName = string.Empty;
			DevicePath = string.Empty;
			CaptureProperty = new CaptureProperty();
			IsEnableClick = true;
			HansFreeMouseProperty = new HansFreeMouseProperty();
		}

		static SaveData(){
			SavePath = "C:\\Users\\MaMaM\\OneDrive\\デスクトップ\\kc2_savedata\\savedata.txt";
		}

		public string ToJsonString()
		{
			return JsonSerializer.Serialize(this);
		}

		public void Save(){
			string json = ToJsonString();
			File.WriteAllText(SavePath,json);
		}

		public static SaveData Load(){
			if(File.Exists(SavePath)){
				string json = File.ReadAllText(SavePath);
				return JsonSerializer.Deserialize<SaveData>(json);

			}else{
				SaveData data = new SaveData();
				data.SetDefault();
				return data;
			}
		}


		public override bool Equals([NotNullWhen(true)] object? obj)
		{
			SaveData? o = obj as SaveData?;

			if (o == null) return false;

			return ToJsonString().Equals(o.Value.ToJsonString()); 
		}

		public override int GetHashCode() {
			return ToJsonString().GetHashCode();
		}

		public override string ToString() {
			return ToJsonString();
		}

	}
}
