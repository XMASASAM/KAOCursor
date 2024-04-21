using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Linq;
using System.Text;
using System.IO;
using System.Text.Json;
using System.Threading.Tasks;
using KC2NativeWrapper;
using System.Windows.Interop;
using System.IO.Compression;
namespace KC2.DataStructs
{


	/// <summary>
	/// デフレートアルゴリズムを使用したデータ圧縮・解凍昨日を定義します。
	/// </summary>
	public static class DataOperation
	{
		//コピペ元->https://takap-tech.com/entry/2020/01/20/211934

		/// <summary>
		/// 文字列を圧縮しバイナリ列として返します。
		/// </summary>
		public static byte[] CompressFromStr(string message) => Compress(Encoding.UTF8.GetBytes(message));

		/// <summary>
		/// バイナリを圧縮します。
		/// </summary>
		public static byte[] Compress(byte[] src)
		{
			using (var ms = new MemoryStream())
			{
				using (var ds = new DeflateStream(ms, CompressionMode.Compress, true/*msは*/))
				{
					ds.Write(src, 0, src.Length);
				}

				// 圧縮した内容をbyte配列にして取り出す
				ms.Position = 0;
				byte[] comp = new byte[ms.Length];
				ms.Read(comp, 0, comp.Length);
				return comp;
			}
		}

		/// <summary>
		/// 圧縮データを文字列として復元します。
		/// </summary>
		public static string DecompressToStr(byte[] src) => Encoding.UTF8.GetString(Decompress(src));

		/// <summary>
		/// 圧縮済みのバイト列を解凍します。
		/// </summary>
		public static byte[] Decompress(byte[] src)
		{
			using (var ms = new MemoryStream(src))
			using (var ds = new DeflateStream(ms, CompressionMode.Decompress))
			{
				using (var dest = new MemoryStream())
				{
					ds.CopyTo(dest);

					dest.Position = 0;
					byte[] decomp = new byte[dest.Length];
					dest.Read(decomp, 0, decomp.Length);
					return decomp;
				}
			}
		}
	}
	public struct SaveData
	{

		public static readonly string SavePath;

		public string DeviceName{ get; set; }
		public string DevicePath{ get; set; }

		public CaptureProperty CaptureProperty{ get; set; }

		public bool IsEnableClick{ get; set; }
		public bool IsEnableShowCircleProgressBar{ get; set; }
		public bool IsEnablePlaySE { get; set; }
		public bool IsEnableInfraredTracker{ get; set; }

		public Win32Mouse.Win32RECT ScreenRect{ get; set; }
		public Vec2d[] FaceRange{ get; set; }
		public bool HasFaceRange{  get; set; }

		public HansFreeMouseProperty HansFreeMouseProperty{ get; set; }

		public string FaceRangeDeviceID{ get; set; }

		public bool NeedTutorial { get; set; }

		public void SetDefault(){
			DeviceName = string.Empty;
			DevicePath = string.Empty;
			CaptureProperty = new CaptureProperty();
			IsEnableClick = true;
			NeedTutorial = true;
			IsEnableShowCircleProgressBar = true;
			IsEnablePlaySE = true;
			IsEnableInfraredTracker = false;
			HansFreeMouseProperty = new HansFreeMouseProperty();
		}

		static SaveData(){
			SavePath = Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData) + "\\KAOCursor";
			if (!Directory.Exists(SavePath)) {
				Directory.CreateDirectory(SavePath);
			}
			SavePath += "\\savedata.kc2savedata";
		}

		public string ToJsonString()
		{
			return JsonSerializer.Serialize(this);
		}

		
		public bool ByteArrayToFile(string fileName, byte[] byteArray)
		{
			//コピペ元->https://stackoverflow.com/questions/6397235/write-bytes-to-file
			try
			{
				using (var fs = new FileStream(fileName, FileMode.Create, FileAccess.Write))
				{
					fs.Write(byteArray, 0, byteArray.Length);
					return true;
				}
			}
			catch (Exception ex)
			{
				Console.WriteLine("Exception caught in process: {0}", ex);
				return false;
			}
		}
		public void Save(){
			string json = ToJsonString();
			// --- ★圧縮する ---

			// 文字列を圧縮する
			byte[] compressed = DataOperation.CompressFromStr(json);
			// > 0x7B 0xDC 0xD8 0xF4 0x18... 30byte（半分に減ってる
			ByteArrayToFile(SavePath, compressed);
			//File.WriteAllText(SavePath,json);
		}

		public static SaveData Load(){
			if(File.Exists(SavePath)){
				var compressed = File.ReadAllBytes(SavePath);
				string json = DataOperation.DecompressToStr(compressed);
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
