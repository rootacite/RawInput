#include <Windows.h>
#include <Gdiplus.h>

//声明Dll中的接口函数的指针类型
typedef void(*pfnRawRegister) (HWND);
typedef int(*pfnGetDeviceCount) ();
typedef int(*pfnGetData)(int,int);

//声明接口函数指针

pfnRawRegister RawRegister = NULL;
pfnGetDeviceCount GetDeviceCount = NULL;
pfnGetData GetData = NULL;

//声明Dll句柄

HMODULE hDll = NULL;

/// <summary>
/// 在某个位置画矩形
/// </summary>
/// <param name="hDc">设备上下文</param>
/// <param name="p">中心位置</param>
void DrawPoint(HDC hDc, POINT p, COLORREF c)
{
	auto hBrush = CreateSolidBrush(c);
	RECT rc{ p.x - 5,p.y - 5,p.x + 5,p.y + 5 };
	FillRect(hDc, &rc, hBrush);

	DeleteObject(hBrush);
}

//WindowProc函数处理消息
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msgID, WPARAM wParam, LPARAM lParam)
{
	switch (msgID)
	{
	case WM_CREATE: //在窗口被创建时，设置定时器，每0.05s触发一次
		SetTimer(hWnd, 1, 50, NULL);
		break;
	case WM_TIMER:  //定时器消息
	{
		HDC hdc = GetDC(hWnd);
		RECT rc{ 0,0,1280,720 };

		auto hBrush = CreateSolidBrush(RGB(255, 255, 255));

		FillRect(hdc, &rc, hBrush);
		COLORREF Cs[] = {RGB(255,0,0),RGB(0,255,0),RGB(0,0,255) };
		
		for (int p = GetDeviceCount() - 1; p >=0 ; p--)
		{
			DrawPoint(hdc, { GetData(p,1),GetData(p,0) }, Cs[p]);
		}

		DeleteObject(hBrush);
		DeleteDC(hdc);
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0); 
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd, msgID, wParam, lParam);
}

//Init函数初始化窗口
HWND Init(HINSTANCE hInst)
{
	//注册主窗口类
	WNDCLASS wc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.hCursor = NULL;
	wc.hIcon = NULL;
	wc.hInstance = hInst;
	wc.lpfnWndProc = WindowProc;
	wc.lpszClassName = L"WinBase";
	wc.lpszMenuName = NULL;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&wc);

	// 在内存中创建一个窗口
	HWND hWnd = CreateWindow(L"WinBase",
		L"Windows",
		WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,
		100, 100, 1280, 720,
		NULL, NULL, hInst, NULL);

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	return hWnd;
}

int WINAPI wWinMain(HINSTANCE hinstance, HINSTANCE pHinstance, LPWSTR lpCmdLine, int nShowLine)
{

	//获取Dll中的接口函数地址
	hDll = LoadLibrary(L"RawInput.dll");
	GetData = (pfnGetData)GetProcAddress(hDll, "GetData");
	GetDeviceCount = (pfnGetDeviceCount)GetProcAddress(hDll, "GetDeviceCount");
	RawRegister = (pfnRawRegister)GetProcAddress(hDll, "RawRegister");

	HWND hWnd = Init(hinstance);

	RawRegister(hWnd);

	//主窗口的消息循环
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
	//.......................
}
