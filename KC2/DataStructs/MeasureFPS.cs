using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KC2.DataStructs
{
	public class MeasureFPS
	{
		int count = 0;
		int ans;
		Stopwatch stopwatch = new Stopwatch();
		public MeasureFPS() { stopwatch.Start(); }

		public void Count()
		{
			if (stopwatch.ElapsedMilliseconds >= 1000)
			{
				ans = count;
				count = 0;
				stopwatch.Restart();
			}
			count++;

		}

		public int GetFPS()
		{
			return ans;
		}

	}
}
