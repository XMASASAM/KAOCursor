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
	/// NumericUpDown.xaml の相互作用ロジック
	/// </summary>
	public partial class NumericUpDown : UserControl
	{
		public static readonly DependencyProperty MaxValueProperty =
			DependencyProperty.Register(nameof(MaxValue), typeof(int), typeof(NumericUpDown), new PropertyMetadata(int.MaxValue));
		public int MaxValue
		{
			get { return (int)GetValue(MaxValueProperty); }
			set { SetValue(MaxValueProperty, value); }
		}
		public static readonly DependencyProperty MinValueProperty =
			DependencyProperty.Register(nameof(MinValue), typeof(int), typeof(NumericUpDown), new PropertyMetadata(int.MinValue));
		public int MinValue
		{
			get { return (int)GetValue(MinValueProperty); }
			set { SetValue(MinValueProperty, value); }
		}

		public static readonly DependencyProperty ButtonWidthProperty =
			DependencyProperty.Register(nameof(ButtonWidth), typeof(GridLength), typeof(NumericUpDown), new PropertyMetadata(new GridLength(24)));
		public GridLength ButtonWidth
		{
			get { return (GridLength)GetValue(ButtonWidthProperty); }
			set { SetValue(ButtonWidthProperty, value); }
		}

		public int CurrentValue { get; private set; }

		int _DecimalPlace = 1;
		public int DecimalPlace { get { return _DecimalPlace; } set { _DecimalPlace = value; DecimalDen = (int)Math.Round(Math.Pow(10.0f, (double)value)); } }
		public int DecimalDen { get; private set; } = 10;
		public NumericUpDown()
		{
			InitializeComponent();
		}

		public void SetCurrentValue(int value)
		{
			SetValueToText(value);
		}

		public void SetDecimalPlace(int place)
		{
			DecimalPlace = place;
			DecimalDen = (int)Math.Round(Math.Pow(10.0f, (double)place));
		}


		private void TextBox_PreviewTextInput(object sender, TextCompositionEventArgs e)
		{
			string t = e.Text;
			bool is_num = new System.Text.RegularExpressions.Regex("[0-9.]+").IsMatch(t);
			e.Handled = !is_num;
		}

		int GetNumFromTextBox()
		{
			var s = textbox.Text;
			int n;
			if (string.IsNullOrEmpty(s))
			{
				n = 0;
			}
			else
			{
				n = 0;
				var ss = s.Split('.');
				if (1 <= ss.Length)
				{
					int n1 = 0, n2 = 0;
					if (s[0] == '.')
					{
						n1 = 0;
					}
					else
					{
						n1 = Convert.ToInt32(ss[0]);
					}
					if (2 <= ss.Length)
					{
						int cut = Math.Min(ss[1].Length, DecimalPlace);
						if (0 < cut)
						{
							while (ss[1].Length < DecimalPlace)
							{
								ss[1] += '0';
							}

							n2 = Convert.ToInt32(ss[1].Substring(0, cut));

						}
					}
					n1 = Math.Clamp(n1, MinValue / DecimalDen, (int)Math.Ceiling(MaxValue / (double)DecimalDen));
					n1 *= DecimalDen;
					n = n1 + n2;
				}
			}
			return n;
		}

		void SetValueToText(int value)
		{
			var s = value.ToString();

			if (1 <= DecimalPlace)
			{
				int d = DecimalPlace + 1 - Math.Min(s.Length, DecimalPlace + 1);
				s = new string('0', d) + s;
				int c = s.Length - DecimalPlace;
				s = s.Insert(c, ".");
			}
			textbox.Text = s;
		}


		private void TextBox_TextChanged(object sender, TextChangedEventArgs e)
		{
			//			int n = GetNumFromTextBox();
			//			if (CurrentValue == n) return;

			//			SetValueToText(n);		
		}

		private void textbox_IsKeyboardFocusedChanged(object sender, DependencyPropertyChangedEventArgs e)
		{
			var f = (bool)e.NewValue;
			if (!f)
			{
				int n = GetNumFromTextBox();
				CurrentValue = Math.Clamp(n, MinValue, MaxValue);
				//textbox.Text = CurrentValue.ToString();
				n = Math.Clamp(n, MinValue, MaxValue);
				SetValueToText(n);
			}
		}

		private void UpButton_Click(object sender, RoutedEventArgs e)
		{
			int n = GetNumFromTextBox();
			n = Math.Clamp(n + 1, MinValue, MaxValue);
			SetValueToText(n);

		}

		private void DownButton_Click(object sender, RoutedEventArgs e)
		{
			int n = GetNumFromTextBox();
			n = Math.Clamp(n - 1, MinValue, MaxValue);
			SetValueToText(n);

		}
	}
}
