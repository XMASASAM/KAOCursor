using KC2.DataStructs;
using KC2NativeWrapper;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Security.Cryptography.Pkcs;
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
using System.Windows.Threading;

namespace KC2.Dialogs
{
	/// <summary>
	/// MouseClickController.xaml の相互作用ロジック
	/// </summary>
	public partial class MouseClickController : Window
	{
		//Pages.MainPage mainPage;
		Dialogs.StayProgressBar stayProgressBar;
		public List<ClickEventTag> Tags { get; set; } = new List<ClickEventTag>();

		static DispatcherTimer timer = new DispatcherTimer();
		static DispatcherTimer timer_window_min = new DispatcherTimer();
		bool flg_cursor_inside = false;
		ClickEventTag? current_tag;
		//		KC2_MouseEvent current_mouse_event;
		SaveData config;
		double ListWidth=0;
		double ListHeight = 0;
		Canvas canvas;
		List<ListViewItem> ListViewItemList = new List<ListViewItem>();
		bool IsWide = false;
		bool IsInsideMoveBorder = false;
		bool IsWindowMove = false;
		Vec2d WindowOffset = new Vec2d();
		public MouseClickController()
		{

			InitializeComponent();
			this.DataContext = this;


			for(int i = 0; i < ClickEventTag.AllTags.Count;i++){
				Tags.Add(ClickEventTag.AllTags[i]);
			//	if (3 < i) break;
			}

	//		this.mainPage = main;
			timer.Interval = new TimeSpan(0, 0, 0, 0, 1);
			timer.Tick += new EventHandler(Update);
			timer.Start();
			config = ((App)Application.Current).SaveData;

			
			KC2HandsFreeMouse.SetEnableClick(config.IsEnableClick);

			TagListView.SelectedIndex = 0;

			timer_window_min.Tick+= new EventHandler(SetWindowMin);
			timer_window_min.Interval = new TimeSpan(0, 0, 0, 0, 500);
		}

	

		void Update(object? sender, EventArgs e)
		{
			if(IsWindowMove){
				Win32Mouse.GetCursorPos(out var pos);
				Left = pos.X - WindowOffset.X;
				Top = pos.Y - WindowOffset.Y;
			}
		}

		void SetWindowMin(object? sender, EventArgs e)
		{
			if (0 <= TagListView.SelectedIndex && IsWide)
			{
				SetWindowWidth(ListViewItemList[TagListView.SelectedIndex].ActualWidth,true);
				double d = Canvas.GetLeft(ListViewItemList[TagListView.SelectedIndex]);
				foreach (var i in ListViewItemList)
				{
					SetChangeLeftAnime(i, Canvas.GetLeft(i) - d, 100);
					//Canvas.SetLeft(i, Canvas.GetLeft(i) - d);
				}
				IsWide = false;
			}
			timer_window_min.Stop();

		}

		public void SetChangeWidthAnime(double width, int milliseconds)
		{
			DoubleAnimation dx = new DoubleAnimation();
			//target_pos = target;
			dx.From = Width;
			dx.To = width;
			dx.Duration = new Duration(TimeSpan.FromMilliseconds(milliseconds));


			Storyboard.SetTarget(dx, this);
			Storyboard.SetTargetProperty(dx, new PropertyPath(Window.WidthProperty));

			Storyboard sb = new Storyboard();
			sb.Children.Add(dx);
			sb.FillBehavior = FillBehavior.Stop;
			sb.Completed += (s, e) => {
				Width = width;
			};
			sb.Begin();
		}

		public void SetChangeLeftAnime(UIElement ui,double next_left, int milliseconds)
		{
			DoubleAnimation dx = new DoubleAnimation();
			//target_pos = target;
			dx.From = Canvas.GetLeft(ui);
			dx.To = next_left;
			dx.Duration = new Duration(TimeSpan.FromMilliseconds(milliseconds));


			Storyboard.SetTarget(dx, ui);
			Storyboard.SetTargetProperty(dx, new PropertyPath(Canvas.LeftProperty));

			Storyboard sb = new Storyboard();
			sb.Children.Add(dx);
			sb.FillBehavior = FillBehavior.Stop;
			sb.Completed += (s, e) => {
				Canvas.SetLeft(ui, next_left);
			};
			sb.Begin();
		}

		private void Window_Loaded(object sender, RoutedEventArgs e)
		{

			stayProgressBar = new Dialogs.StayProgressBar(this);
			stayProgressBar.Show();
		}

		private void Window_Closed(object sender, EventArgs e)
		{
			stayProgressBar.Close();
		}


		void SetWindowWidth(double width_on_canvas,bool anim){
			double s1 = Width - TagListView.ActualWidth;
			double w = (canvas.PointToScreen(new System.Windows.Point(width_on_canvas, 0)).X) - Left + s1 / 2 + 1;

			if(anim)
				SetChangeWidthAnime(w, 100);
			else{
				Width = w;
			}

			MaxHeight = ActualHeight;
			MinHeight = ActualHeight;
		}

		private void Window_MouseEnter(object sender, MouseEventArgs e)
		{
			if (IsWindowMove) return;

			//Window内に入られたとき.
			if (sender is Window){
				KC2HandsFreeMouse.SetEnableClick(false);
				flg_cursor_inside=true;

				Topmost = false;
				Topmost = true;
				stayProgressBar.Topmost = false;
				stayProgressBar.Topmost = true;
				timer_window_min.Stop();
			}
			else if(sender is ListBoxItem){
				ListBoxItem t = (ListBoxItem)sender;
				current_tag = (ClickEventTag)t.DataContext;

				if (!IsWide)
				{
					SetWindowWidth(ListWidth,true);
					double cl = 0;
					foreach (var i in ListViewItemList)
					{
						SetChangeLeftAnime(i, cl, 100);

						cl += i.ActualWidth;
					}
					IsWide = true;
				}
			}else if(((FrameworkElement)sender).Name.Equals("MoveBorder")){
			
				IsInsideMoveBorder = true;
			}
		}

		private void Window_MouseLeave(object sender, MouseEventArgs e)
		{
			if (IsWindowMove) return;
			//Window外に出たとき.
			if (sender is Window)
			{
				if (config.IsEnableClick)
					KC2HandsFreeMouse.SetEnableClick(true);
				flg_cursor_inside = false;
				Topmost = true;

				timer_window_min.Stop();
				timer_window_min.Start();

			}
			else if (sender is ListBoxItem)
			{

				ListBoxItem t = (ListBoxItem)sender;
				if (current_tag == (ClickEventTag)t.DataContext)
				{
					current_tag = null;
				}
			}
			else if (((FrameworkElement)sender).Name.Equals("MoveBorder"))
			{
				IsInsideMoveBorder = false;
			}
		}

		public void CallClickCursorEvent(){
			if(IsInsideMoveBorder){
				Win32Mouse.GetCursorPos(out var pos);

				IsWindowMove = !IsWindowMove;

				WindowOffset.X = pos.X - Left;
				WindowOffset.Y = pos.Y - Top;
			}else{
				IsWindowMove=false;
			}

			if (current_tag == null) return;

			int index = Tags.IndexOf(current_tag);
			TagListView.SelectedIndex = index;
		}

		public void CallMoveEvent()
		{
			Win32Mouse.GetCursorPos(out var pos);
			this.Left = pos.X;
			this.Top = pos.Y;
		}

		private void TagListView_SelectionChanged(object sender, SelectionChangedEventArgs e)
		{
			var tag = (ClickEventTag)TagListView.SelectedItem;
			KC2HandsFreeMouse.SetMouseClickEvent(tag.MouseEvent);
		}

		StackPanel stp;
		private void ListBoxItem_Loaded(object sender, RoutedEventArgs e)
		{
			var s = (ListViewItem)sender;
			Canvas.SetTop(s, 0);
			Canvas.SetLeft(s, ListWidth);
			ListWidth += s.ActualWidth;
		//	canvas.Width = ListWidth;
			//canvas.Height = s.ActualHeight;
			ListHeight = Math.Max(ListHeight, s.ActualHeight);
			ListViewItemList.Add((ListViewItem)sender);
			SetWindowWidth(ListWidth,false);
			IsWide = true;
		}

		private void TagListView_Loaded(object sender, RoutedEventArgs e)
		{
			var s = (FrameworkElement)sender;
		//	ListWidth += s.ActualWidth;
		}


		private void CanvasView_Loaded(object sender, RoutedEventArgs e)
		{
			canvas = (Canvas)sender;
			Canvas.SetTop(canvas, 0);
			Canvas.SetLeft(canvas, 0);
		}

		private void MoveBorder_Click(object sender, RoutedEventArgs e)
		{

		}

		private void MoveBorder_MouseDown(object sender, MouseButtonEventArgs e)
		{

		}

		private void MoveBorder_MouseUp(object sender, MouseButtonEventArgs e)
		{

			//Win32Mouse.GetCursorPos(out var pos);
			//WindowOffset.X = pos.X - Left;
			//WindowOffset.Y = pos.Y - Top;
		}

		private void MoveBorder_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
		{
			if (IsWindowMove) return;
			IsWindowMove = true;
			Win32Mouse.GetCursorPos(out var pos);
			WindowOffset.X = pos.X - Left;
			WindowOffset.Y = pos.Y - Top;
		}

		private void MoveBorder_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
		{
			if (!IsWindowMove) return;
			IsWindowMove = false;
		}
	}
}
