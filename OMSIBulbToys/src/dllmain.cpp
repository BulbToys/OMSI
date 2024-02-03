#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "offsets.h"

extern "C" __declspec(dllexport) void __stdcall PluginStart(void* aOwner);
extern "C" __declspec(dllexport) void __stdcall AccessSystemVariable(unsigned short index, float* value, bool* write);
extern "C" __declspec(dllexport) void __stdcall AccessStringVariable(unsigned short index, wchar_t* value, bool* write);
extern "C" __declspec(dllexport) void __stdcall AccessVariable(unsigned short index, float* value, bool* write);
extern "C" __declspec(dllexport) void __stdcall AccessTrigger(unsigned short index, bool* active);
extern "C" __declspec(dllexport) void __stdcall PluginFinalize();

BOOL APIENTRY DllMain(HMODULE instance, DWORD, LPVOID)
{
	return TRUE;
}

void __stdcall PluginStart(void* aOwner)
{
	const char* version = nullptr;

	if (!(version = Offsets::v2_3_004::Check()) && !(version = Offsets::v2_2_032::Check()))
	{
		MessageBox(0, "Unknown OMSI Version", "OMSI Bulb Toys", MB_OK);
		return;
	}

	MessageBox(0, version, "OMSI Bulb Toys", MB_OK);
}

void __stdcall AccessSystemVariable(unsigned short index, float* value, bool* write)
{

}

void __stdcall AccessStringVariable(unsigned short index, wchar_t* value, bool* write)
{

}

void __stdcall AccessVariable(unsigned short index, float* value, bool* write)
{

}

void __stdcall AccessTrigger(unsigned short index, bool* active)
{

}

void __stdcall PluginFinalize()
{

}
