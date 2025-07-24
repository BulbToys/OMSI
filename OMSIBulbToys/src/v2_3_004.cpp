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

	virtual uintptr_t BulbToys_GetTextureManager1() override final { return Read<uintptr_t>(0x861BC4); }
	virtual uintptr_t BulbToys_GetTextureManager2() override final { return Read<uintptr_t>(0x861BC8); }

	virtual void BulbToys_ForceOnDepot(bool on_depot) override final
	{
		Unprotect _(0x7D4FC5, 1);
		Write<bool>(0x7D4FC5, on_depot);
	}

	virtual uintptr_t BulbToys_GetMyVehicle() override final
	{
		constexpr uintptr_t TRVList_GetMyVehicle = 0x74A43C;

		// TRVList
		auto var = Read<uintptr_t>(0x861508);

		__asm
		{
			mov     eax, var
			call    TRVList_GetMyVehicle
			mov     var, eax
		}

		return var;
	}

	virtual HRESULT BulbToys_D3DXSaveTextureToFile(LPCWSTR pDestFile, DWORD DestFormat, void* pSrcTexture, void* pSrcPalette) override final
	{
		return reinterpret_cast<HRESULT(__cdecl*)(LPCWSTR, DWORD, void*, void*)>(0x568070)(pDestFile, DestFormat, pSrcTexture, pSrcPalette);
	}
} _;