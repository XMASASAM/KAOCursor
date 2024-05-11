using KC2.DataStructs;
using KC2.Pages;
using KC2NativeWrapper;
using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Drawing;
using System.Linq;
using System.Media;
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
		int tag_selected_index=-1;
		//		KC2_MouseEvent current_mouse_event;
		SaveData config;
		double ListWidth = 0;
		double ListHeight = 0;
		Canvas canvas;
		List<ListViewItem> ListViewItemList = new List<ListViewItem>();
		bool IsWide = false;
		bool IsInsideMoveBorder = false;
		bool IsActualInsideMoveBorder = false;
		bool IsWindowMove = false;
		Vec2d WindowOffset = new Vec2d();
		static MediaPlayer ClickSound;
		//		static MediaPlayer ClickSound;
		MainPage mainPage;

		static MouseClickController()
		{
	//		ClickSound = new MediaPlayer();//new SoundPlayer("PC-Mouse03-06(R).wav");	
//			ClickSound.Open(new Uri("PC-Mouse03-06(R).wav"));
		}

		public MouseClickController(MainPage main)
		{

			InitializeComponent();
			this.mainPage = main;
			this.DataContext = this;
			//	Visibility = Visibility.Hidden;

			for (int i = 0; i < ClickEventTag.AllTags.Count; i++) {
				Tags.Add(ClickEventTag.AllTags[i]);
				//	if (3 < i) break;
			}
			Width = 0;
			//	SetWindowWidth(64 * Tags.Count, false);
			//		this.mainPage = main;
			timer.Interval = new TimeSpan(0, 0, 0, 0, 1);
			timer.Tick += new EventHandler(Update);
			timer.Start();
			config = ((App)Application.Current).SaveData;


			KC2HandsFreeMouse.SetEnableClick(config.IsEnableClick);

			TagListView.SelectedIndex = 0;

			timer_window_min.Tick += new EventHandler(SetWindowMin);
			timer_window_min.Interval = new TimeSpan(0, 0, 0, 0, 500);


		}

		bool DetectInsideMoveBorder()
		{
			if (!IsVisible) return false;
			var pos = Win32Mouse.GetCursorPosOnMonitor();//Win32Mouse.GetCursorPos(out var pos);
			var lt = MoveBorder.PointToScreen(new System.Windows.Point(0, 0));
			var rb = MoveBorder.PointToScreen(new System.Windows.Point(MoveBorder.ActualWidth, MoveBorder.ActualHeight));
			return (lt.X <= pos.X && pos.X < rb.X && lt.Y <= pos.Y && pos.Y < rb.Y);
		}

		void Update(object? sender, EventArgs e)
		{
			Topmost = false;
			stayProgressBar.Topmost = false;
			Topmost = true;
			stayProgressBar.Topmost = true;
			if (IsWindowMove){
				var pos = Win32Mouse.GetCursorPosOnScreen();//.GetCursorPos(out var pos);
				Left = pos.X - WindowOffset.X;
				Top = pos.Y - WindowOffset.Y;
			}
			if(DetectInsideMoveBorder()){
				Mouse.OverrideCursor = Cursors.Cross;
				IsActualInsideMoveBorder = true;
			}else{
				IsActualInsideMoveBorder = false;
				Mouse.OverrideCursor = null;
			}

			bool is_rampaging = KC2HandsFreeMouse.DetectRampaging();
			if (is_rampaging){
				mainPage.SetActiveWindow();
				mainPage.StopHansFreeMouse();
			}

		}

		void SetWindowMin(object? sender, EventArgs e)
		{
			
			if (0 <= TagListView.SelectedIndex && IsWide && IsVisible)
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

		//アニメーション.
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
			timer_window_min.Stop();
			stayProgressBar.Close();
		}


		void SetWindowWidth(double width_on_canvas,bool anim){
			double s1 = ActualWidth - TagListView.ActualWidth;
			double w =  width_on_canvas + s1 + 1;//(canvas.PointToScreen(new System.Windows.Point(width_on_canvas, 0)).X) - Left + s1 / 2 + 1;

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
				//KC2HandsFreeMouse.SetEnableClick(false);
				if (config.IsEnableClick)
				{
					KC2HandsFreeMouse.SetEnableClick(false);//.SetClickAllowed(0);
				}
				flg_cursor_inside =true;


				timer_window_min.Stop();
				Console.WriteLine("Enter_Window");

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
				Console.WriteLine("Enter_ListBox");

			}
			else if(((FrameworkElement)sender).Name.Equals("MoveBorder")){
			
				IsInsideMoveBorder = true;
				Console.WriteLine("Enter_MoveBorder");

			}else{
				Console.WriteLine("Enter_Other");

			}
		}

		//上にほかのウィンドウがあると表示されない.
		private void Window_MouseLeave(object sender, MouseEventArgs e)
		{

			if (IsWindowMove) return;
			//Window外に出たとき.
			if (sender is Window)
			{
				if (config.IsEnableClick){
					KC2HandsFreeMouse.SetEnableClick(true);//SetClickAllowed(1);
				}
				flg_cursor_inside = false;
				Topmost = true;

				timer_window_min.Stop();
				timer_window_min.Start();
				Console.WriteLine("Leave_Window");

			}
			else if (sender is ListBoxItem)
			{

				ListBoxItem t = (ListBoxItem)sender;
				if (current_tag == (ClickEventTag)t.DataContext)
				{
					current_tag = null;
				}
				Console.WriteLine("Leave_ListBox");

			}
			else if (((FrameworkElement)sender).Name.Equals("MoveBorder"))
			{
				IsInsideMoveBorder = false;
				Console.WriteLine("Leave_MoveBorder");

			}else{
				Console.WriteLine("Leave_Other");

			}
		}

		public void CallClickCursorEvent(){
			//	ClickSound.Play();
			if (IsInsideMoveBorder || IsActualInsideMoveBorder){
				var pos = Win32Mouse.GetCursorPosOnScreen();
				if (IsActualInsideMoveBorder){
					IsWindowMove = !IsWindowMove;
					WindowOffset.X = pos.X - Left;
					WindowOffset.Y = pos.Y - Top;
				}else{
					//上にほかのウィンドウがある場合通る
					if (config.IsEnableClick)
					{
						KC2HandsFreeMouse.SetEnableClick(true);//.SetClickAllowed(1);
					}
					IsInsideMoveBorder = false;
					flg_cursor_inside = false;

					Window_MouseLeave(this,null);
					current_tag = null;
				}

			}

			if(!IsActualInsideMoveBorder){
				IsWindowMove=false;
			}

			


			if (flg_cursor_inside){
				KC2HandsFreeMouse.PlaySE();
				if (current_tag == null) return;

				int index = Tags.IndexOf(current_tag);
				
				TagListView.SelectedIndex = index;
			}
		}

		//マウスコントローラーを移動させるイベント.
		public void CallMoveEvent()
		{
			var t = KC2HandsFreeMouse.GetMouseClickEvent();
			if( ((int)t&(int)KC2_MouseEvent.KC2_MouseEvent_Wheel)!=0 ){
				return;
			}
			var pos = Win32Mouse.GetCursorPosOnScreen();
			this.Left = pos.X;
			this.Top = pos.Y;
		}

		private void TagListView_SelectionChanged(object sender, SelectionChangedEventArgs e)
		{
			var tag = (ClickEventTag)TagListView.SelectedItem;
			bool ok = true;
			if(tag.MouseEvent.Equals(KC2_MouseEvent.KC2_MouseEvent_Stop)){
				mainPage.StopHansFreeMouse();
				TagListView.SelectedIndex = tag_selected_index;
				ok = false;
			}

			if(tag.MouseEvent.Equals(KC2_MouseEvent.KC2_MouseEvent_ShowMenu)){
				mainPage.SetActiveWindow();
				TagListView.SelectedIndex = tag_selected_index;
				ok = false;
			}

			if(ok){
				tag_selected_index = TagListView.SelectedIndex;
				KC2HandsFreeMouse.SetMouseClickEvent(tag.MouseEvent);
			}
		}

		StackPanel stp;
		private void ListBoxItem_Loaded(object sender, RoutedEventArgs e)
		{
			var s = (ListViewItem)sender;
			Canvas.SetTop(s, 0);
			Canvas.SetLeft(s, ListWidth);
			ListWidth += s.ActualWidth;
			var ggg = s.Width;
		//	canvas.Width = ListWidth;
			//canvas.Height = s.ActualHeight;
			ListHeight = Math.Max(ListHeight, s.ActualHeight);
			ListViewItemList.Add((ListViewItem)sender);
			IsWide = true;

			if(Tags.Count<=ListViewItemList.Count){
				//Visibility = Visibility.Visible;
				SetWindowWidth(ListWidth, false);

			}

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
			var pos = Win32Mouse.GetCursorPosOnScreen();
			WindowOffset.X = pos.X - Left;
			WindowOffset.Y = pos.Y - Top;
		}

		private void MoveBorder_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
		{
			if (!IsWindowMove) return;
			IsWindowMove = false;
		}

		private void Window_LostFocus(object sender, RoutedEventArgs e)
		{
			Console.WriteLine("LostForcus");
		}

		private void Window_Activated(object sender, EventArgs e)
		{
			Console.WriteLine("Actived");

		}
	}
}
