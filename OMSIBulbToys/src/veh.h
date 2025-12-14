#pragma once
#include "../core/bulbtoys.h"

class VEH
{
	PVOID handler = nullptr;
	
	VEH();
	~VEH();

	static LONG CALLBACK ExceptionHandler(EXCEPTION_POINTERS* exception_pointers);
};