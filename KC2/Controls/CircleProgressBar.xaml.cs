using KC2.Dialogs;
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
	/// CircleProgressBar.xaml の相互作用ロジック
	/// </summary>
	public partial class CircleProgressBar : UserControl
	{

		public static readonly DependencyProperty StartPointProperty =
	DependencyProperty.Register(nameof(StartPoint), typeof(System.Windows.Point), typeof(CircleProgressBar), new PropertyMetadata(new System.Windows.Point(0, 0)));
		public System.Windows.Point StartPoint
		{
			get { return (System.Windows.Point)GetValue(StartPointProperty); }
			set { SetValue(StartPointProperty, value); }
		}

		public static readonly DependencyProperty EndPointProperty =
	DependencyProperty.Register(nameof(EndPoint), typeof(System.Windows.Point), typeof(CircleProgressBar), new PropertyMetadata(new System.Windows.Point(0, 0)));
		public System.Windows.Point EndPoint
		{
			get { return (System.Windows.Point)GetValue(EndPointProperty); }
			set { SetValue(EndPointProperty, value); }
		}


		public static readonly DependencyProperty BStartPointProperty =
	DependencyProperty.Register(nameof(BStartPoint), typeof(System.Windows.Point), typeof(CircleProgressBar), new PropertyMetadata(new System.Windows.Point(0, 0)));
		public System.Windows.Point BStartPoint
		{
			get { return (System.Windows.Point)GetValue(BStartPointProperty); }
			set { SetValue(BStartPointProperty, value); }
		}

		public static readonly DependencyProperty BEndPointProperty =
	DependencyProperty.Register(nameof(BEndPoint), typeof(System.Windows.Point), typeof(CircleProgressBar), new PropertyMetadata(new System.Windows.Point(0, 0)));
		public System.Windows.Point BEndPoint
		{
			get { return (System.Windows.Point)GetValue(BEndPointProperty); }
			set { SetValue(BEndPointProperty, value); }
		}


		public static readonly DependencyProperty IsLargeArcProperty =
DependencyProperty.Register(nameof(IsLargeArc), typeof(bool), typeof(CircleProgressBar), new PropertyMetadata(false));
		public bool IsLargeArc
		{
			get { return (bool)GetValue(IsLargeArcProperty); }
			set { SetValue(IsLargeArcProperty, value); }
		}

		public static readonly DependencyProperty BIsLargeArcProperty =
DependencyProperty.Register(nameof(BIsLargeArc), typeof(bool), typeof(CircleProgressBar), new PropertyMetadata(false));
		public bool BIsLargeArc
		{
			get { return (bool)GetValue(BIsLargeArcProperty); }
			set { SetValue(BIsLargeArcProperty, value); }
		}

		public static readonly DependencyProperty CColorProperty =
DependencyProperty.Register(nameof(CColor), typeof(SolidColorBrush), typeof(CircleProgressBar), new PropertyMetadata(new SolidColorBrush(Colors.Red)));
		public SolidColorBrush CColor
		{
			get { return (SolidColorBrush)GetValue(CColorProperty); }
			set { SetValue(CColorProperty, value); }
		}

		public System.Windows.Size CircleSize
		{
			get; set;
		}
		public System.Windows.Size ArcSize
		{
			get; set;
		}

		public double CircleThickness
		{
			get; set;
		}

		public double BCircleThickness
		{
			get; set;
		}

		public double current_par=-1;

		public CircleProgressBar()
		{

			init();

			InitializeComponent();
			this.DataContext = this;
		}

		System.Windows.Point GetPoint(System.Windows.Point cp, double r, double deg)
		{
			double rad = Math.PI * deg / 180.0;
			double x = cp.X + r * Math.Sin(rad);
			double y = cp.Y - r * Math.Cos(rad);
			return new System.Windows.Point(x, y);
		}

		void SetDegree(double deg)
		{
			System.Windows.Point cp = new System.Windows.Point(ArcSize.Width, ArcSize.Height);
			double r = ArcSize.Width;

			BEndPoint = GetPoint(cp, r, deg);
			if (180 <= deg) BIsLargeArc = true;
			else BIsLargeArc = false;


			EndPoint = GetPoint(cp, r, deg);
			if (180 <= deg) IsLargeArc = true;
			else IsLargeArc = false;
		}

		void init()
		{
			CircleSize = new System.Windows.Size(100, 100);
			ArcSize = new System.Windows.Size(CircleSize.Width * .5, CircleSize.Height * .5);
			CircleThickness = 18;
			BCircleThickness = 24;
			System.Windows.Point cp = new System.Windows.Point(ArcSize.Width, ArcSize.Height);
			double r = ArcSize.Width;
			StartPoint = GetPoint(cp, r, 0);
			BStartPoint = GetPoint(cp, r, 0);
			SetDegree(0);


		}
		
		public void SetProgress(double par){
			if (current_par == par) return;
			SetDegree(359.99 * par);
			current_par = par;
		}

	}
}
