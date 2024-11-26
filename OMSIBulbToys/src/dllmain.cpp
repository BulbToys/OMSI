#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "omsi.h"

#include "../core/bulbtoys.h"

#include "../core/bulbtoys/gui.h"

extern "C" __declspec(dllexport) void __stdcall PluginStart(void* aOwner);
extern "C" __declspec(dllexport) void __stdcall AccessSystemVariable(unsigned short index, float* value, bool* write);
extern "C" __declspec(dllexport) void __stdcall AccessStringVariable(unsigned short index, wchar_t* value, bool* write);
extern "C" __declspec(dllexport) void __stdcall AccessVariable(unsigned short index, float* value, bool* write);
extern "C" __declspec(dllexport) void __stdcall AccessTrigger(unsigned short index, bool* active);
extern "C" __declspec(dllexport) void __stdcall PluginFinalize();

static HMODULE dll_instance = 0;
BOOL APIENTRY DllMain(HMODULE instance, DWORD, LPVOID)
{
	dll_instance = instance;
	return TRUE;
}

void __stdcall PluginStart(void* aOwner)
{
	const char* version = nullptr;

	if (!(version = OMSI::v2_3_004::Check()) && !(version = OMSI::v2_2_032::Check()))
	{
		Error("This version of OMSI 2 is not supported.");
		return;
	}

	/*
	FILE* console;
	if (!AllocConsole())
	{
		MessageBox(0, "WARN: Console already allocated!", "OMSI Bulb Toys", MB_OK);
	}
	SetConsoleTitle("OMSI Texture Manager - Debug");

	freopen_s(&console, "CONIN$", "r", stdin);
	freopen_s(&console, "CONOUT$", "w", stdout);
	freopen_s(&console, "CONOUT$", "w", stderr);

	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);

	DWORD mode;
	GetConsoleMode(handle, &mode);
	SetConsoleMode(handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN);
	*/
}

static bool init = false;
void __stdcall AccessSystemVariable(unsigned short index, float* value, bool* write)
{
	static bool first = false;
	if (!first)
	{
		first = true;

		BulbToys::SetupParams params;
		params.instance = dll_instance;
		params.settings_file = "OMSI_BulbToys.ini";
		params.GetD3D9Device = +[]()
		{
			return Read<LPVOID>(OMSI::TheDirect3DDevice9);
		};
		params.GetDI8KeyboardDevice = +[]()
		{
			LPVOID device = nullptr;

			auto tdicl = Read<uintptr_t>(OMSI::TheDirectInputCl);
			if (tdicl)
			{
				device = Read<LPVOID>(tdicl + 0x8);
			}

			return device;
		};
		init = BulbToys::Init(params);
		if (init)
		{
			// Keyboard IO doesn't work
			new MainWindow();
		}
	}
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
	if (init)
	{
		// Hangs on EnumThreadWindows
		BulbToys::End();
	}
}
