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
		public Vec2I DeviceSize{ get; set; }

		public double CameraAngle { get; set; }
		public double CameraScaleX { get; set; }
		public double CameraScaleY { get; set; }

		public void SetDefault(){
			DeviceName = string.Empty;
			DevicePath = string.Empty;
			DeviceSize = new Vec2I(0,0);

			CameraAngle = 0;
			CameraScaleX = 1;
			CameraScaleY = 1;

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
