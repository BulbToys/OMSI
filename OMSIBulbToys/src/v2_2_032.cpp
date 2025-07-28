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

	virtual uintptr_t BulbToys_GetTextureManager1() override final { ASSERT(!"TODO"); return 0; }
	virtual uintptr_t BulbToys_GetTextureManager2() override final { ASSERT(!"TODO"); return 0; }

	virtual HRESULT D3DXSaveTextureToFile(LPCWSTR pDestFile, DWORD DestFormat, void* pSrcTexture, void* pSrcPalette) override final
	{
		ASSERT(!"TODO"); return 0x8876086C;
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
		ASSERT(!"TODO"); return 0x8876086C;
	}

	virtual void BulbToys_ForceMipLevelsPatch(bool unpatch, void* func) override final { ASSERT(!"TODO"); }

	virtual void BulbToys_ForceOnDepot(bool on_depot) override final { ASSERT(!"TODO"); }

	virtual uintptr_t BulbToys_GetMyVehicle() override final { ASSERT(!"TODO"); return 0; }

	virtual void BulbToys_SetTextureMaxSize(uint16_t size) override final { ASSERT(!"TODO"); }
	virtual void BulbToys_SetTextureUnloadFrames(uint16_t frames) override final { ASSERT(!"TODO"); }
} _;