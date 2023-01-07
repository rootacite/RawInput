// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "Helper.hpp"

#include <vector>
#include <map>

using namespace std;

map<HANDLE, POINT> Points; //保存鼠标设备的句柄和该鼠标的位置

HWND m_hWnd = NULL;
HHOOK hHook = NULL;
HMODULE hModule = NULL;
LRESULT CALLBACK GetMessageProc(int nCode, WPARAM wParam, LPARAM lParam);

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        ::hModule = hModule;
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

DLLAPI void Test()
{
    MessageBoxA(0, "Test", "Info", 0);
}

/// <summary>
///  注册设备，因为只有一个设备类型，所以不管多少个鼠标，组测一个就够了
/// </summary>
/// <param name="hWnd">主窗口的句柄</param>
/// <returns></returns>
DLLAPI void RawRegister(HWND hWnd)
{
    m_hWnd = hWnd;
    RAWINPUTDEVICE Rid[1];

    Rid[0].usUsagePage = 0x01;  //设备类型为鼠标       
    Rid[0].usUsage = 0x02;               
    Rid[0].dwFlags = RIDEV_INPUTSINK; //RIDEV_INPUTSINK参数让程序可以后台运行
    Rid[0].hwndTarget = hWnd;   

  //  Rid[1].usUsagePage = 0x01;      
  //  Rid[1].usUsage = 0x02;             
  //  Rid[1].dwFlags = RIDEV_INPUTSINK;  
   // Rid[1].hwndTarget = hWnd;

    if (RegisterRawInputDevices(Rid, 1, sizeof(Rid[0])) == FALSE)
    {
        int Sk = GetLastError();
        MsgBox("Failed");
    }

    hHook = SetWindowsHookEx(WH_GETMESSAGE, GetMessageProc, hModule, 0);
}

/// <summary>
/// 获取当前检测到的设备数量
/// </summary>
/// <returns>设备数量</returns>
DLLAPI int GetDeviceCount() 
{
    return Points.size();
}

/// <summary>
/// 获取某个鼠标设备的X轴或Y轴
/// </summary>
/// <param name="Index">鼠标设备的下标</param>
/// <param name="XY">true为X轴，false为Y轴</param>
/// <returns></returns>
DLLAPI int GetData(int Index, int XY)
{
    int t = 0;
    for (auto i : Points)
    {
        if (Index != t) { t++; continue; }
        if (XY)
        {
            return i.second.x;
        }
        else {
            return i.second.y;
        }
        t++;
    }

    return 0;
}

LRESULT CALLBACK GetMessageProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    LPMSG pMsg = (LPMSG)lParam;
    if (pMsg->hwnd != m_hWnd) return CallNextHookEx(hHook, nCode, wParam, lParam);

    switch (pMsg->message)
    {
        // print out the values that I need
    case WM_INPUT: { //只处理WM_INPUT消息
        UINT dataSize;
        GetRawInputData(reinterpret_cast<HRAWINPUT>(pMsg->lParam),
            RID_INPUT, NULL, &dataSize, sizeof(RAWINPUTHEADER));
        //Need to populate data size first
        //获取数据的大小      
        if (dataSize > 0) //如果数据不为空则处理数据
        {
            std::vector<BYTE> rawdata(dataSize);
            //vector容器避免内存泄漏，懒得释放指针

            //获取数据
            if (GetRawInputData(reinterpret_cast<HRAWINPUT>(pMsg->lParam), RID_INPUT, rawdata.data(), &dataSize, sizeof(RAWINPUTHEADER)) == dataSize)
            {
                RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(rawdata.data());

                if (raw->header.dwType == RIM_TYPEMOUSE) //如果接收到的是鼠标消息
                {
                    if (Points.count(raw->header.hDevice) == 0)  //如果map中不存在该鼠标句柄，则说明是一个新的鼠标，将其插入map中
                        Points.insert({ raw->header.hDevice, { 0,0 } });


                    POINT new_point = { Points[raw->header.hDevice].x,Points[raw->header.hDevice].y };
                    new_point.x += raw->data.mouse.lLastX;
                    new_point.y += raw->data.mouse.lLastY;
                    //将map中保存的鼠标位置，和本次消息中获取到的鼠标位移相加，然后更新map中保存的值
                    Points[raw->header.hDevice] = new_point;

                }
            }
        }
        return 0;
    }
    }

    return CallNextHookEx(hHook, nCode, wParam, lParam);
}