#pragma once
#include <string>

#include "../core/bulbtoys.h"

inline class Game
{
	static inline Game* first = nullptr;
	Game* next = nullptr;
public:
	Game()
	{
		if (!Game::first)
		{
			Game::first = this;
		}
		else
		{
			auto game = Game::first;
			while (game->next)
			{
				game = game->next;
			}
			game->next = this;
		}
	}

	static const char* Init()
	{
		auto game = Game::first;
		while (game)
		{
			auto version = game->Check();
			if (version)
			{
				OMSI = game;
				return version;
			}
			game = game->next;
		}
		return nullptr;
	}

	virtual const char* Check() = 0;

	/* ===== S T R U C T S ===== */

	struct TTextureItem
	{
		uint16_t size_x;
		uint16_t size_y;
		double mem;
		int datasize;
		bool dataready;
		void* Texture_ID3DT9;
		void* oldTexture_ID3DT9;
		char* path;
		char* justfilename;
		char* loadpath;
		char loaded;
		char load_request;
		bool managed;
		unsigned int failed;
		uint16_t used;
		uint16_t used_highres;
		bool threadloading;
		bool hasspecials;
		bool no_unload;
		bool onlyalpha;
		int NightMap;
		int WinterSnowMap;
		int WinterSnowfallMap;
		int FallMap;
		int SpringMap;
		int WinterMap;
		int SummerDryMap;
		int SurfMap;
		bool moisture;
		bool puddles;
		bool moisture_ic;
		bool puddles_ic;
		char surface;
		char surface_ic;
		bool terrainmapping;
		bool terrainmapping_alpha;
	};

	/* ===== C O N S T A N T S ===== */

	virtual LPVOID BulbToys_GetD3DDevice9() = 0;
	virtual uintptr_t BulbToys_GetDirectInputCl() = 0;

	virtual uintptr_t BulbToys_GetTextureManager1() = 0;
	virtual uintptr_t BulbToys_GetTextureManager2() = 0;

	/* ===== G A M E   F U N C S ===== */

	virtual HRESULT D3DXSaveTextureToFile(LPCWSTR pDestFile, DWORD DestFormat, void* pSrcTexture, void* pSrcPalette) = 0;

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
	) = 0;

	/* ===== H E L P E R   F U N C S ===== */

	virtual void BulbToys_ForceMipLevelsPatch(bool unpatch, void* func) = 0;

	virtual void BulbToys_ForceOnDepot(bool on_depot) = 0;

	virtual uintptr_t BulbToys_GetMyVehicle() = 0;

	virtual inline int BulbToys_ListLength(uintptr_t list) final { return Read<int>(list - 4); }

	virtual void BulbToys_SetTextureMaxSize(uint16_t size) = 0;
	virtual void BulbToys_SetTextureUnloadFrames(uint16_t frames) = 0;
} *OMSI;