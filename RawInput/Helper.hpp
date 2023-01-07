#include <Windows.h>
#include <string>

#define DLLAPI extern "C" __declspec(dllexport)

void MsgBox(std::string Str)
{
	MessageBoxA(0, Str.c_str(), "Info", 0);
}