using KC2.DataStructs;
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

namespace KC2
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            MainFrame.Navigate(new Pages.MainPage());
            //KC2DeviceInformation.Init();
            //var 
        }

		private void Window_Closed(object sender, EventArgs e)
		{
            VideoCaptureDevice.AllActiveDeviceRelease();

		}
	}
}