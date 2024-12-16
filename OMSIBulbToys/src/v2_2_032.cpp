#include "omsi.h"

static class v2_2_032 : public Game
{
	virtual const char* Check() override final
	{
		if (!strncmp(reinterpret_cast<char*>(0x7C9668), ":3", 2))
		{
			return "2.2.032 - Tram patch";
		}

		return nullptr;
	}

	virtual LPVOID BulbToys_GetD3DDevice9() override final { ASSERT(!"TODO"); return 0; }

	virtual uintptr_t BulbToys_GetDirectInputCl() override final { ASSERT(!"TODO"); return 0; }
} _;