using KC2NativeWrapper;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media.Imaging;

namespace KC2.DataStructs
{
	public class ClickEventTag
	{
		public string Name { get; set; }
		public KC2_MouseEvent MouseEvent { get; set; }
		public BitmapImage Icon { get; set; }

		public ClickEventTag(string name,KC2_MouseEvent mouse_event,BitmapImage icon){
			Name = name;
			MouseEvent = mouse_event;
			Icon = icon;
		}

		readonly public static List<ClickEventTag> AllTags = new List<ClickEventTag>();
		static ClickEventTag(){
			var m = Application.Current.Resources;
			AllTags.Add(new ClickEventTag("LeftClick", KC2_MouseEvent.KC2_MouseEvent_LeftClick,(BitmapImage)m["IM_LC"]));
			AllTags.Add(new ClickEventTag("RightClick", KC2_MouseEvent.KC2_MouseEvent_RightClick, (BitmapImage)m["IM_RC"]));
			AllTags.Add(new ClickEventTag("MiddleClick", KC2_MouseEvent.KC2_MouseEvent_MiddleClick, (BitmapImage)m["IM_MC"]));
			AllTags.Add(new ClickEventTag("LeftDoubleClick", KC2_MouseEvent.KC2_MouseEvent_LeftDoubleClick, (BitmapImage)m["IM_LD"]));
			AllTags.Add(new ClickEventTag("RightDoubleClick", KC2_MouseEvent.KC2_MouseEvent_RightDoubleClick, (BitmapImage)m["IM_RD"]));
			AllTags.Add(new ClickEventTag("MiddleDoubleClick", KC2_MouseEvent.KC2_MouseEvent_MiddleDoubleClick, (BitmapImage)m["IM_MD"]));
			AllTags.Add(new ClickEventTag("LeftDrag", KC2_MouseEvent.KC2_MouseEvent_LeftDrag, (BitmapImage)m["IM_LG"]));
			AllTags.Add(new ClickEventTag("RightDrag", KC2_MouseEvent.KC2_MouseEvent_RightDrag, (BitmapImage)m["IM_RG"]));
			AllTags.Add(new ClickEventTag("MiddleDrag", KC2_MouseEvent.KC2_MouseEvent_MiddleDrag, (BitmapImage)m["IM_MG"]));
			AllTags.Add(new ClickEventTag("WheelUp", KC2_MouseEvent.KC2_MouseEvent_WheelUp, (BitmapImage)m["IM_WU"]));
			AllTags.Add(new ClickEventTag("WheelDown", KC2_MouseEvent.KC2_MouseEvent_WheelDown, (BitmapImage)m["IM_WD"]));
			AllTags.Add(new ClickEventTag("Stop", KC2_MouseEvent.KC2_MouseEvent_Stop, (BitmapImage)m["IM"]));
			AllTags.Add(new ClickEventTag("Menu", KC2_MouseEvent.KC2_MouseEvent_ShowMenu, (BitmapImage)m["IPC"]));
			
		}


	}
}
