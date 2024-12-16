#include "omsi.h"

static class v2_3_004 : public Game
{
	virtual const char* Check() override final
	{
		if (!strncmp(reinterpret_cast<char*>(0x7C9780), ":3", 2))
		{
			return "2.3.004 - Latest Steam version";
		}

		return nullptr;
	}

	virtual LPVOID BulbToys_GetD3DDevice9() override final { return Read<LPVOID>(0x8627D0); }

	virtual uintptr_t BulbToys_GetDirectInputCl() override final { return Read<uintptr_t>(0x862EE8); }
} _;