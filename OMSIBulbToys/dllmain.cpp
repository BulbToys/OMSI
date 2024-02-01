#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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
	MessageBoxA(0, "PluginStart", "OMSI Bulb Toys", MB_OK);
}

void __stdcall AccessSystemVariable(unsigned short index, float* value, bool* write)
{
	static bool seen = false;

	if (!seen)
	{
		MessageBoxA(0, "AccessSystemVariable", "OMSI Bulb Toys", MB_OK);
		seen = true;
	}
}

void __stdcall AccessStringVariable(unsigned short index, wchar_t* value, bool* write)
{
	static bool seen = false;

	if (!seen)
	{
		MessageBoxA(0, "AccessStringVariable", "OMSI Bulb Toys", MB_OK);
		seen = true;
	}
}

void __stdcall AccessVariable(unsigned short index, float* value, bool* write)
{
	static bool seen = false;

	if (!seen)
	{
		MessageBoxA(0, "AccessVariable", "OMSI Bulb Toys", MB_OK);
		seen = true;
	}
}

void __stdcall AccessTrigger(unsigned short index, bool* active)
{
	static bool seen = false;

	if (!seen)
	{
		MessageBoxA(0, "AccessTrigger", "OMSI Bulb Toys", MB_OK);
		seen = true;
	}
}

void __stdcall PluginFinalize()
{
	MessageBoxA(0, "PluginFinalize", "OMSI Bulb Toys", MB_OK);
}
