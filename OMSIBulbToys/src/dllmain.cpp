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
