using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Controls.Primitives;

namespace KC2NativeWrapper
{


	[StructLayout(LayoutKind.Sequential)]
	public struct VideoInfo
	{
		public int Width { get; set; }
		public int Height { get; set; }
		public int Format { get; set; }
		public double FPS { get; set; }

	}
	[StructLayout(LayoutKind.Sequential)]
	public struct DeviceInfo
	{
		IntPtr _Name;
		IntPtr _Path;
		public IntPtr VideoInfoList;
		public int VideoInfoListSize;

		public string Name
		{
			get
			{
				if (_Name == IntPtr.Zero) return "";
				return Marshal.PtrToStringAuto(_Name);
			}
		}
		public string Path
		{
			get
			{
				if (_Path == IntPtr.Zero) return "";
				return Marshal.PtrToStringAuto(_Path);
			}
		}

		public VideoInfo GetVideoInfo(int index){
			return KC2DeviceInformation.kc2np_video_info_list_get_video_info(VideoInfoList, index);
		}

	}
	public static class KC2DeviceInformation
	{
		const string DLLNAME = "KC2NativeProject.dll";

		[DllImport(DLLNAME)]
		internal static extern int kc2np_init_device_info_list(out IntPtr device_list);

		[DllImport(DLLNAME)]
		internal static extern DeviceInfo kc2np_device_info_list_get_device_info(IntPtr devic_list, int index);

		[DllImport(DLLNAME)]
		internal static extern VideoInfo kc2np_video_info_list_get_video_info(IntPtr video_info_list, int index);

		static bool is_read = false;
		static IntPtr device_list_ptr;
		static List<DeviceInfo> device_list = new List<DeviceInfo>();
		static public List<DeviceInfo> DeviceList{ get { return device_list; } set { } }
		static int list_size;
		static public void Init(){
			if(is_read){
				return;
			}

			list_size = kc2np_init_device_info_list(out device_list_ptr);
			for (int i = 0; i < list_size; i++){
				DeviceInfo info = kc2np_device_info_list_get_device_info(device_list_ptr, i);
				device_list.Add(info);
			}
			is_read = true;
		}

		static public int GetIndex(string name , string path){
			Init();
			for (int i = 0; i < list_size; i++) {
				var info = device_list[i];
				if (info.Name.Equals(name) && info.Path.Equals(path)){
					return i;
				}
			}
			return -1;
		}

	}
}