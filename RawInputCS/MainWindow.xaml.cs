using RawInputCS.Tools;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;
using Color = System.Drawing.Color;

namespace RawInputCS
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        struct Point
        {
            public int X, Y;
            public override string ToString()
            {
                return X.ToString() + "," + Y.ToString();
            }
        }

        Point[] GetPoints()
        {
            int Size = DllAPI.GetDeviceCount();
            if (Size == 0) return null;

            Point[] Points = new Point[Size];

            for (int i = 0; i < Size; i++)
            {
                Point ix = new Point();
                ix.X = DllAPI.GetData(i, 1);
                ix.Y = DllAPI.GetData(i, 0);
                Points[i] = ix;
            }

            return Points;
        }
        void Draw(Action action)
        {
            Buffer.Lock();
            action.Invoke();
            Buffer.AddDirtyRect(new Int32Rect(0, 0, 1280, 720));
            Buffer.Unlock();
        }

        static WriteableBitmap Buffer = new WriteableBitmap(1280, 720, 96, 96, PixelFormats.Bgra32, null);
        Bitmap Drawer = new Bitmap(1280, 720, Buffer.BackBufferStride, System.Drawing.Imaging.PixelFormat.Format32bppRgb, Buffer.BackBuffer);
        readonly Graphics g;
        public MainWindow()
        {
            InitializeComponent();
            g = Graphics.FromImage(Drawer);
            Loaded += (e, v) =>
            {
                Mouse.OverrideCursor = Cursors.None;
                DllAPI.RawRegister(new WindowInteropHelper(this).Handle);
                Plant.Source = Buffer;

               

                DispatcherTimer Dt = new DispatcherTimer();
                Dt.Interval = TimeSpan.FromSeconds(0.01);
                Dt.Tick += (s, d) =>
                {
                    var ps = GetPoints();
                    if (ps == null) return;

                    if(ps.Length == 2)
                    {
                        Title = ps[0].ToString() + "::" + ps[1].ToString();
                    }

                    Draw(() =>
                    {
                        g.Clear(Color.Wheat);

                        foreach(var i in ps)
                        {
                            System.Drawing.Point p = new System.Drawing.Point(640, 360);
                            p.X += i.X;
                            p.Y += i.Y;

                            using (SolidBrush sb = new SolidBrush(Color.Black))
                                g.FillEllipse(sb, new System.Drawing.Rectangle(p.X - 5, p.Y - 5, 10, 10));
                        }
                    });
                };
                Dt.Start();
            };
        }

        protected override void OnSourceInitialized(EventArgs e)
        {
            base.OnSourceInitialized(e);
            HwndSource source = PresentationSource.FromVisual(this) as HwndSource;
            source.AddHook(WndProc);
        }

        private IntPtr WndProc(IntPtr hwnd, int msg, IntPtr wParam, IntPtr lParam, ref bool handled)
        {
            return DllAPI.WindowProc(hwnd, (uint)msg, wParam, lParam);
        }
    }
}
