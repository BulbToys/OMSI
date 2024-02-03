#pragma once

#include <string>

// No offset macro
#define NO_OF(id) 0x0//0x00DEAD00 + 0x01000000 * id
namespace Offsets
{
	// String that will be used in the version check. Length is automatically calculated at compile time
	constexpr const char* const str = ":3";
	constexpr int len = std::char_traits<char>::length(str);

	namespace v2_3_004
	{
		inline const char* Check()
		{
			if (!strncmp(reinterpret_cast<char*>(0x7C9780), str, len))
			{
				return "2.3.004 - Latest Steam version";
			}

			return nullptr;
		}
	}

	namespace v2_2_032
	{
		inline const char* Check()
		{
			if (!strncmp(reinterpret_cast<char*>(0x7C9668), str, len))
			{
				return "2.2.032 - Tram patch";
			}

			return nullptr;
		}
	}
}
#undef NO_OF