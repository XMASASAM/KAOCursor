using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KC2.DataStructs
{
    class Rect2I
    {
        public Vec2I Point;
        public Vec2I Size;
        public Rect2I(Vec2I point,Vec2I size) { 
            Point = point;
            Size = size;
        }
    }
    public struct Vec2I:IComparable<Vec2I>
    {
        public int X { get; set; }
        public int Y { get; set; }
        public Vec2I(int x, int y)
        {
            this.X = x;
            this.Y = y;
        }
		public int CompareTo(Vec2I o)
		{
            int a = Y.CompareTo(o.Y);
            if(a!=0)return a;
            return X.CompareTo(o.X);
        }
	}

	public struct Vec2d : IComparable<Vec2d>
	{
		public double X { get; set; }
		public double Y { get; set; }
		public Vec2d(double x, double y)
		{
			this.X = x;
			this.Y = y;
		}
		public int CompareTo(Vec2d o)
		{
			int a = Y.CompareTo(o.Y);
			if (a != 0) return a;
			return X.CompareTo(o.X);
		}
	}
}
