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

	virtual HRESULT D3DXSaveTextureToFile(LPCWSTR pDestFile, DWORD DestFormat, void* pSrcTexture, void* pSrcPalette) override final
	{
		return reinterpret_cast<HRESULT(__cdecl*)(LPCWSTR, DWORD, void*, void*)>(0x568070)(pDestFile, DestFormat, pSrcTexture, pSrcPalette);
	}

	virtual HRESULT D3DXCreateTextureFromFileExA(
		void* pDevice,
		const char* pSrcFile,
		DWORD Width,
		DWORD Height,
		DWORD MipLevels,
		DWORD Usage,
		DWORD Format,
		DWORD Pool,
		DWORD Filter,
		DWORD MipFilter,
		DWORD ColorKey,
		void* pSrcInfo,
		void* pPalette,
		void* ppTexture
	) override final
	{
		using func = HRESULT(__cdecl*)(void*, const char*, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, void*, void*, void*);
		return reinterpret_cast<func>(0x568060)(pDevice, pSrcFile, Width, Height, MipLevels, Usage, Format, Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, ppTexture);
	}

	virtual void BulbToys_ForceMipLevelsPatch(bool unpatch, void* func) override final
	{
		Unprotect _(0x7BBC47, 0x7F9092 - 0x7BBC47);

		if (!unpatch)
		{
			PatchCall(0x7BBC47, func);
			PatchCall(0x7F8F9D, func);
			PatchCall(0x7F908E, func);
		}
		else
		{
			Unpatch(0x7F908E);
			Unpatch(0x7F8F9D);
			Unpatch(0x7BBC47);
		}
	}

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

	virtual void BulbToys_SetTextureMaxSize(uint16_t size) override final
	{
		Unprotect _(0x7F8D40, 0x7F8DF0 - 0x7F8D40);
		Write<uint16_t>(0x7F8D40, size);
		Write<uint16_t>(0x7F8D5C, size);
		Write<uint16_t>(0x7F8D92, size);
		Write<uint16_t>(0x7F8DAF, size);
	}

	virtual void BulbToys_SetTextureUnloadFrames(uint16_t frames) override final
	{
		Unprotect _(0x7F9E85, 2);
		Write<uint16_t>(0x7F9E85, frames);
	}
} _;