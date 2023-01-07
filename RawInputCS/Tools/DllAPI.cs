using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace RawInputCS.Tools
{
    static public class DllAPI
    {
        [DllImport("RawInput.dll")]
        extern public static void Test();
        [DllImport("RawInput.dll")]
        extern public static void RawRegister(IntPtr hWnd);
        [DllImport("RawInput.dll")]
        extern public static int GetDeviceCount();
        [DllImport("RawInput.dll")]
        extern public static int GetData(int Index, int XY);
    }
}
