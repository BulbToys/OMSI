#include "veh.h"
#include <minidumpapiset.h>

VEH::VEH()
{
	handler = AddVectoredExceptionHandler(TRUE, VEH::ExceptionHandler);
}

VEH::~VEH()
{
	RemoveVectoredExceptionHandler(handler);
}

LONG CALLBACK VEH::ExceptionHandler(EXCEPTION_POINTERS* exception_pointers)
{
	DWORD exception_thread_id = GetCurrentThreadId();

	// Set up minidump information
	MINIDUMP_EXCEPTION_INFORMATION exception_info;
	exception_info.ClientPointers = TRUE;
	exception_info.ExceptionPointers = exception_pointers;
	exception_info.ThreadId = exception_thread_id;

	// Format minidump name to BulbToys_HHMMSS.dmp
	SYSTEMTIME time;
	GetLocalTime(&time);

	// Write minidump file
	char dmp_filename[24];
	MYPRINTF(dmp_filename, 24, "BulbToys_%02d%02d%02d.dmp", time.wHour, time.wMinute, time.wSecond);

	HANDLE hProcess = GetCurrentProcess();
	HANDLE hFile = CreateFile(dmp_filename, GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	MiniDumpWriteDump(hProcess, GetCurrentProcessId(), hFile, MiniDumpWithDataSegs, &exception_info, NULL, NULL);
	CloseHandle(hFile);

	// Try to get the name of the module our address is in
	PVOID address = exception_pointers->ExceptionRecord->ExceptionAddress;

	char module_name[MAX_PATH] = { 0 };
	MEMORY_BASIC_INFORMATION memory_info;
	if (VirtualQuery(address, &memory_info, 28UL))
	{
		if (GetModuleFileName((HMODULE)memory_info.AllocationBase, module_name, MAX_PATH))
		{
			MYPRINTF(module_name, MAX_PATH, " in module %s", strrchr(module_name, '\\') + 1);
		}
	}

	DWORD code = exception_pointers->ExceptionRecord->ExceptionCode;
	Error("Exception %08X at %08X%s. Saved to %s.\n\n", code, address, module_name, dmp_filename);
	return EXCEPTION_CONTINUE_SEARCH;
}