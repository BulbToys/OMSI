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

	virtual uintptr_t BulbToys_GetWeather() override final { return Read<uintptr_t>(0x8617D0); }
	virtual uintptr_t BulbToys_GetTimeTableManager() override final { return Read<uintptr_t>(0x8614E8); }

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

	virtual uintptr_t BulbToys_GetCurrentDriver() override final
	{
		auto index = Read<int>(0x8614FC);
		auto list = Read<uintptr_t>(0x8614F8);

		if (!list)
		{
			return 0;
		}

		if (!BulbToys_BoundCheck(list, index))
		{
			return 0;
		}

		return list + index * 0x68;
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

	virtual char* BulbToys_GetLineName(int schedule_line) override final
	{
		auto tttman = BulbToys_GetTimeTableManager();
		if (!tttman)
		{
			return nullptr;
		}

		auto lines = Read<uintptr_t>(tttman + 0x18);
		if (!BulbToys_BoundCheck(lines, schedule_line))
		{
			return nullptr;
		}

		return Read<char*>(lines + schedule_line * 0x10);
	}

	virtual void BulbToys_GetTripInfo(int trip, int busstop_index, wchar_t** busstop_name, int* busstop_count) override final
	{
		auto tttman = BulbToys_GetTimeTableManager();
		if (!tttman)
		{
			return;
		}

		auto trips = Read<uintptr_t>(tttman + 0xC);
		if (!BulbToys_BoundCheck(trips, trip))
		{
			return;
		}

		// Taken from TRoadVehicleInst_ScriptCallback, case 10 (GetTTBusstopCount macro)
		auto busstops_for_trip = Read<uintptr_t>(trips + trip * 0x28 + 0x18);
		if (busstop_count)
		{
			*busstop_count = BulbToys_ListLength(busstops_for_trip) - 1;
		}

		if (!BulbToys_BoundCheck(busstops_for_trip, busstop_index))
		{
			// TODO: watching AI drive out of bounds causes this. should we care?
			return;
		}
		if (busstop_name)
		{
			// Taken from TProgMan.Render ("next Stop:" in Shift+Y overlay)
			*busstop_name = Read<wchar_t*>(busstops_for_trip + (busstop_index << 6));
		}
	}

	virtual void BulbToys_GetWorldDate(char*& day_of_week, uint8_t& day, uint8_t& month, uint16_t& year) override final
	{ 
		day_of_week = Read<char*>(0x861780);
		day = Read<uint8_t>(0x861778);
		month = Read<uint8_t>(0x86178C);
		year = Read<uint16_t>(0x861790);
	}

	virtual void BulbToys_GetWorldTime(uint8_t& hours, uint8_t& minutes, float& seconds) override final
	{ 
		hours = Read<uint8_t>(0x86176C);
		minutes = Read<uint8_t>(0x86176D);
		seconds = Read<float>(0x861770);
	}

	virtual bool BulbToys_IsInMouseControl() override final { return Read<bool>(0x861697); }
	virtual bool BulbToys_IsSimPaused() override final { return Read<bool>(0x861718); }

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

	virtual uintptr_t BulbToys_TSound_Create() override final
	{
		constexpr uintptr_t TSound_Create = 0x74FAB0;
		constexpr uintptr_t vtable = 0x74CD24;

		uintptr_t device = Read<uintptr_t>(0x8627D4);

		uintptr_t sound = 0;

		__asm
		{
			mov     ecx, device
			mov     dl, 1
			mov     eax, vtable
			call    TSound_Create
			mov     sound, eax
		}

		Write<bool>(sound + 0x3C, 0);
		Write<bool>(sound + 0x3D, 0);
		Write<bool>(sound + 0x3E, 0);
		Write<bool>(sound + 0x7D, 0);
		Write<bool>(sound + 0x4A, 0);
		Write<float>(sound + 0x6C, 1.0f);
		Write<char>(sound + 0x49, 5);

		constexpr uintptr_t DynArraySetLength = 0x40A9B0;
		constexpr uintptr_t rtti_tsound_4 = 0x74CCA0;
		uintptr_t volcurves = sound + 0x74;

		constexpr uintptr_t TBoolClass_Create = 0x7F023C;
		constexpr uintptr_t boolclass_vtable = 0x7EEB1C;

		uintptr_t boolclass = 0;

		__asm
		{
			push    0
			mov     eax, volcurves
			mov     ecx, 1
			mov     edx, rtti_tsound_4
			call    DynArraySetLength
			add     esp, 4
			mov     dl, 1
			mov     eax, boolclass_vtable
			mov     boolclass, eax
		}

		Write<uintptr_t>(sound + 0x78, boolclass);

		return sound;
	}

	virtual void BulbToys_TSound_Play(uintptr_t sound, const char* filename) override final
	{
		static Game::AnsiString<MAX_PATH> sound_name_ansi;
		sound_name_ansi.SetString("%s", filename);
		auto name = sound_name_ansi.string;

		constexpr uintptr_t TSound_SetNewFilename = 0x74FC40;
		constexpr uintptr_t TSound_Load = 0x74FF2C;
		constexpr uintptr_t TSound_Play = 0x750444;

		//uintptr_t last_cam_pos = OMSI->BulbToys_GetWeather() + 0x124;

		constexpr uintptr_t arg_6 = 0x854E80;
		constexpr uintptr_t arg_5 = 0x861695;

		__asm
		{
			mov     eax, sound
			mov     edx, name
			call    TSound_SetNewFilename
		}

		Error("block1");

		__asm
		{
			mov     eax, sound
			call    TSound_Load
		}

		Error("block2");

		__asm
		{
			mov     eax, arg_6
			push    eax
			mov     eax, arg_5
			push    eax
			push    0
			mov     eax, sound
			xor     ecx, ecx
			xor     edx, edx
			call    TSound_Play
		}
	}
} _;