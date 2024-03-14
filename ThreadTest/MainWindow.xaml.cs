using KC2NativeWrapper;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace ThreadTest
{
	/// <summary>
	/// Interaction logic for MainWindow.xaml
	/// </summary>
	public partial class MainWindow : Window
	{
		public List<DeviceInfo> DeviceList{  get; set; }
		public List<VideoInfo> VideoInfoList{ get; set; } = new List<VideoInfo>();
		VideoCaptureManager? cap = null;
		public MainWindow()
		{
			InitializeComponent();
			KC2NativeWrapper.KC2DeviceInformation.Init();
			DeviceList = KC2DeviceInformation.DeviceList;
		}

		private void DeviceListView_SelectionChanged(object sender, SelectionChangedEventArgs e)
		{
			if(0<=DeviceListView.SelectedIndex){
				VideoInfoList.Clear();
				var l = KC2DeviceInformation.GetVideoInfoList((DeviceInfo)DeviceListView.SelectedItem);
				foreach(var i in l){
					VideoInfoList.Add(i);
				}
				VideoInfoListView.Items.Refresh();
			}
		}

		private void VideoInfoListView_SelectionChanged(object sender, SelectionChangedEventArgs e)
		{
			if(0<=DeviceListView.SelectedIndex) {
				if(0<=VideoInfoListView.SelectedIndex) {
					var dinfo = (DeviceInfo)DeviceListView.SelectedItem;
					var vinfo = (VideoInfo)VideoInfoListView.SelectedItem;
					if(cap!=null){
						cap.Release();
					}
					cap = new VideoCaptureManager(dinfo, vinfo);
					ImageView.Source = cap.GetWritableBitmap();
				}
			}
		}

		private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
		{
			if(cap!=null){
				cap.Release();
			}
		}
	}
}