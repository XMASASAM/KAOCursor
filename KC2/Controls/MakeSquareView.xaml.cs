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

namespace KC2.Controls
{
    /// <summary>
    /// MakeSquareView.xaml の相互作用ロジック
    /// </summary>
    public partial class MakeSquareView : UserControl
    {
		bool is_first_time = true;
		double originX;
		double originY;

		public MakeSquareView()
        {
            InitializeComponent();
        }

		private void UserControl_Loaded(object sender, RoutedEventArgs e)
		{
		}
		private void canvas_SizeChanged(object sender, SizeChangedEventArgs e)
		{
			tracker.Width = canvas.ActualWidth;
			tracker.Height = canvas.ActualHeight;
		}

		void SetCanvas(UIElement ui,double x,double y){
			Canvas.SetTop(ui, y);
			Canvas.SetLeft(ui, x);
		}

		private void tracker_DragStarted(object sender, System.Windows.Controls.Primitives.DragStartedEventArgs e)
		{
			var p = Mouse.GetPosition(canvas);
			tracker.Width = 16;
			tracker.Height = 16;
			Canvas.SetTop(tracker, p.Y-8);
			Canvas.SetLeft(tracker, p.X-8);
			tracker.Background = new SolidColorBrush(Color.FromRgb(255, 0, 0));

			tracker_o.Visibility = Visibility.Visible;
			SetCanvas(tracker_o, p.X-8, p.Y-8);
			border.Visibility = Visibility.Visible;
			SetCanvas(border, p.X, p.Y);
			border.Width = 0;
			border.Height = 0;
			originX = p.X; 
			originY = p.Y;
		}



		private void tracker_DragDelta(object sender, System.Windows.Controls.Primitives.DragDeltaEventArgs e)
		{
			var p = Mouse.GetPosition(canvas);

			SetCanvas(tracker,p.X-8,p.Y-8);

			double minX = Math.Min(p.X, originX);
			double minY = Math.Min(p.Y, originY);

			double maxX = Math.Max(p.X, originX);
			double maxY = Math.Max(p.Y, originY);

			double w = maxX - minX;
			double h = maxY - minY;

			

			border.Width = w;
			border.Height = h;
			SetCanvas(border,minX,minY);

		}
	}
}
