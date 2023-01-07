// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "Helper.hpp"

#include <vector>
#include <map>

using namespace std;

map<HANDLE, POINT> Points;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
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

DLLAPI void RawRegister(HWND hWnd)
{
    RAWINPUTDEVICE Rid[1];

    Rid[0].usUsagePage = 0x01;        
    Rid[0].usUsage = 0x02;             
    Rid[0].dwFlags = RIDEV_INPUTSINK; 
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
}

DLLAPI LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        // print out the values that I need
    case WM_INPUT: {
        UINT dataSize;
        GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, NULL, &dataSize, sizeof(RAWINPUTHEADER)); //Need to populate data size first
        if (dataSize > 0)
        {
            std::vector<BYTE> rawdata(dataSize);

            if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, rawdata.data(), &dataSize, sizeof(RAWINPUTHEADER)) == dataSize)
            {
                RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(rawdata.data());
                if (raw->header.dwType == RIM_TYPEMOUSE)
                {
                    if (Points.count(raw->header.hDevice) == 0)
                        Points.insert({ raw->header.hDevice, { 0,0 } });

                    POINT new_point = { Points[raw->header.hDevice].x,Points[raw->header.hDevice].y };
                    new_point.x += raw->data.mouse.lLastX;
                    new_point.y += raw->data.mouse.lLastY;
                    Points[raw->header.hDevice] = new_point;

                }
            }
        }
        return 0;
    }
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

DLLAPI int GetDeviceCount()
{
    return Points.size();
}

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
