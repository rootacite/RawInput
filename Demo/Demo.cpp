#include <Windows.h>
#include <Gdiplus.h>

//����Dll�еĽӿں�����ָ������
typedef void(*pfnRawRegister) (HWND);
typedef int(*pfnGetDeviceCount) ();
typedef int(*pfnGetData)(int,int);

//�����ӿں���ָ��

pfnRawRegister RawRegister = NULL;
pfnGetDeviceCount GetDeviceCount = NULL;
pfnGetData GetData = NULL;

//����Dll���

HMODULE hDll = NULL;

/// <summary>
/// ��ĳ��λ�û�����
/// </summary>
/// <param name="hDc">�豸������</param>
/// <param name="p">����λ��</param>
void DrawPoint(HDC hDc, POINT p, COLORREF c)
{
	auto hBrush = CreateSolidBrush(c);
	RECT rc{ p.x - 5,p.y - 5,p.x + 5,p.y + 5 };
	FillRect(hDc, &rc, hBrush);

	DeleteObject(hBrush);
}

//WindowProc����������Ϣ
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msgID, WPARAM wParam, LPARAM lParam)
{
	switch (msgID)
	{
	case WM_CREATE: //�ڴ��ڱ�����ʱ�����ö�ʱ����ÿ0.05s����һ��
		SetTimer(hWnd, 1, 50, NULL);
		break;
	case WM_TIMER:  //��ʱ����Ϣ
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

//Init������ʼ������
HWND Init(HINSTANCE hInst)
{
	//ע����������
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

	// ���ڴ��д���һ������
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

	//��ȡDll�еĽӿں�����ַ
	hDll = LoadLibrary(L"RawInput.dll");
	GetData = (pfnGetData)GetProcAddress(hDll, "GetData");
	GetDeviceCount = (pfnGetDeviceCount)GetProcAddress(hDll, "GetDeviceCount");
	RawRegister = (pfnRawRegister)GetProcAddress(hDll, "RawRegister");

	HWND hWnd = Init(hinstance);

	RawRegister(hWnd);

	//�����ڵ���Ϣѭ��
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
	//.......................
}
