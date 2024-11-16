#pragma once
#include <string>

#include "../core/bulbtoys.h"

// No offset macro, for easier debugging
#define NO_OF(id) 0x0//0x00DEAD00 + 0x01000000 * id

namespace OMSI
{
	// String that will be used in the version check. Length is automatically calculated at compile time
	constexpr const char* const check_str = ":3";
	constexpr int check_str_len = std::char_traits<char>::length(check_str);

	inline uintptr_t TheApplication = NO_OF(0);
	inline uintptr_t TheDirect3DDevice9 = NO_OF(1);
	inline uintptr_t TheDirectInputCl = NO_OF(2);	

	namespace v2_3_004
	{
		inline const char* Check()
		{
			if (!strncmp(reinterpret_cast<char*>(0x7C9780), check_str, check_str_len))
			{
				TheApplication = 0x85F3D4;
				TheDirect3DDevice9 = 0x8627D0;
				TheDirectInputCl = 0x862EE8;

				return "2.3.004 - Latest Steam version";
			}

			return nullptr;
		}
	}

	namespace v2_2_032
	{
		inline const char* Check()
		{
			if (!strncmp(reinterpret_cast<char*>(0x7C9668), check_str, check_str_len))
			{
				//return "2.2.032 - Tram patch";
				return nullptr;
			}

			return nullptr;
		}
	}
}
#undef NO_OF