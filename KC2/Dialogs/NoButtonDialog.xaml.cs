using KC2.DataStructs;
using KC2NativeWrapper;
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
using System.Windows.Media.Animation;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace KC2.Dialogs
{
	/// <summary>
	/// NoButtonDialog.xaml の相互作用ロジック
	/// </summary>
	public partial class NoButtonDialog : Window
	{
		public bool IsMoving{ get;private set; }
		Vec2d target_pos;
		public NoButtonDialog()
		{
			InitializeComponent();
		}

		public void SetImage(BitmapImage img)
		{
			Image1.Source = img;
		}
		public void SetText(string text)
		{
			Text1.Text = text;
		}

		private void Window_Loaded(object sender, RoutedEventArgs e)
		{
			var p = Win32Mouse.GetDialogPosNearMouse((int)ActualWidth, (int)ActualHeight);
			this.Left = p.X;
			this.Top = p.Y;
		}

		public void SetMoveAnime(Vec2d target, int milliseconds)
		{
			IsMoving = true;
			DoubleAnimation dx = new DoubleAnimation();
			target_pos = target;
			dx.From = this.Left;
			dx.To = target.X;
			dx.Duration = new Duration(TimeSpan.FromMilliseconds(milliseconds));

			DoubleAnimation dy = new DoubleAnimation();
			dy.From = this.Top;
			dy.To = target.Y;
			dy.Duration = new Duration(TimeSpan.FromMilliseconds(milliseconds));


			Storyboard.SetTarget(dx, this);
			Storyboard.SetTargetProperty(dx, new PropertyPath(Window.LeftProperty));

			Storyboard.SetTarget(dy, this);
			Storyboard.SetTargetProperty(dy, new PropertyPath(Window.TopProperty));

			Storyboard sb = new Storyboard();
			sb.Children.Add(dx);
			sb.Children.Add(dy);
			sb.FillBehavior = FillBehavior.Stop;
			sb.Completed += new EventHandler(CompletedMove);
			sb.Begin();

		}

		void CompletedMove(object? sender, EventArgs e)
		{
			Left = target_pos.X;
			Top = target_pos.Y;
			IsMoving = false;
		}
	}
}
