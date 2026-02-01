#include "../../core/bulbtoys.h"
#include "../omsi.h"

#include <SFML/Audio.hpp>

namespace announcer_sfml
{
	bool IsPathLegalCharacter(char c, bool ignore_slashes)
	{
		switch (c)
		{
		case '\\':
		case '/':
			return ignore_slashes;
		case '<':
		case '>':
		case ':':
		case '"':
		case '|':
		case '?':
		case '*':
			return false;
		default:
			return true;
		}
	}

	int FilterPathLegal(ImGuiInputTextCallbackData* data)
	{
		if (data->EventChar < 256 && !IsPathLegalCharacter(data->EventChar, true))
		{
			return 1;
		}
		return 0;
	}

	#define DEFAULT_OMSI_ANNOUNCEMENTS_FOLDER "Vehicles\\Announcements"

	char hof_or_map[MAX_PATH] = "(N/A)";
	char folder[MAX_PATH] = "(N/A)";
	char subfolder[MAX_PATH] = "(N/A)";

	const char* subfolder_methods[] = { "Current vehicle HOF", "Current map name", "Current map friendly-name", "Custom" };
	enum SubfolderMethod : int
	{
		HOF = 0,
		Map,
		MapFriendly,
		Custom
	};
	int subfolder_method = SubfolderMethod::HOF;

	const char* first_last_stop_methods[] = { "Both", "Neither", "Current only" };
	enum FLStopMethod : int
	{
		Both = 0,
		Neither,
		CurrentOnly
	};
	int terminus_method = FLStopMethod::Both;
	int first_stop_method = FLStopMethod::Neither;

	bool verbose = false;

	bool announce_next = false;
	bool announce_current = false;

	int current_offset = 0;
	float current_distance = 20.0f;

	sf::SoundBuffer buffer;
	sf::Sound* sound = nullptr;
	float volume = 100.0f;

	void FormatFullPath(char* dst, size_t len)
	{
		if (subfolder_method == SubfolderMethod::Custom)
		{
			MYPRINTF(dst, len, "%s\\%s", folder, subfolder);
		}
		else
		{
			MYPRINTF(dst, len, "%s\\%s", folder, hof_or_map);
		}
	}

	bool SFML_PlaySound(const char* filename, int32_t offset_ms, bool show_errors)
	{
		if (sound)
		{
			sound->stop();
			delete sound;
		}

		if (buffer.loadFromFile(filename))
		{
			sound = new sf::Sound(buffer);
			sound->setVolume(volume);
			sound->play();
			sound->setPlayingOffset(sf::milliseconds(offset_ms));
			return true;
		}

		if (show_errors)
		{
			Error("Couldn't find '%s'", filename);
		}
		return false;
	}

	struct AnnouncerSFMLPanel : IPanel
	{
		char sound_name[MAX_PATH] { 0 };

		virtual bool Draw() override final
		{
			if (ImGui::BulbToys_Menu("Announcer"))
			{
				ImGui::Text("Announcements folder:");
				ImGui::SameLine();
				if (ImGui::Button("Reset to Default##AnnFolderDef"))
				{
					MYPRINTF(folder, MAX_PATH, "%s", DEFAULT_OMSI_ANNOUNCEMENTS_FOLDER);
				}
				ImGui::InputText("##AnnouncementsFolder", folder, MAX_PATH, ImGuiInputTextFlags_CallbackCharFilter, FilterPathLegal);

				ImGui::Text("Announcements subfolder:");
				ImGui::Combo("##AnnouncementsSubfolderMethod", &subfolder_method, subfolder_methods, IM_ARRAYSIZE(subfolder_methods));
				ImGui::BeginDisabled(subfolder_method != SubfolderMethod::Custom);
				ImGui::InputText("##AnnouncementsSubfolder", (subfolder_method == SubfolderMethod::Custom) ? subfolder : hof_or_map, MAX_PATH, ImGuiInputTextFlags_CallbackCharFilter, FilterPathLegal);
				ImGui::EndDisabled();

				char full_path[MAX_PATH];
				FormatFullPath(full_path, MAX_PATH);
				ImGui::Text("Full path: %s", full_path);
				if (ImGui::Button("Copy##AnnouncerFullPath"))
				{
					CopyToClipboard<MAX_PATH>(full_path);
				}

				ImGui::Text("Volume:");
				ImGui::SliderFloat("##AnnouncerSFMLVolume", &volume, 0.0f, 100.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp);

				ImGui::Text("Append '_#terminus' for next/current stop:");
				ImGui::Combo("##AnnouncementsTerminusMethod", &terminus_method, first_last_stop_methods, IM_ARRAYSIZE(first_last_stop_methods));

				ImGui::Text("Announce first next/current stop:");
				ImGui::Combo("##AnnouncementsFirstMethod", &first_stop_method, first_last_stop_methods, IM_ARRAYSIZE(first_last_stop_methods));

				ImGui::Checkbox("Verbose errors", &verbose);

				ImGui::Separator();

				ImGui::Checkbox("Announce next stop", &announce_next);

				ImGui::Separator();

				ImGui::Checkbox("Announce current stop", &announce_current);

				ImGui::Text("Playback offset [ms]:");
				if (ImGui::InputInt("##AnnouncerPlaybackOffset", &current_offset))
				{
					if (current_offset < 0)
					{
						current_offset = 0;
					}
				}

				ImGui::Text("Distance to stop [m]:");
				ImGui::SliderFloat("##AnnouncerCurrentDistance", &current_distance, 5.0f, 100.0f);

				ImGui::Separator();

				ImGui::Text("Sound name (relative to OMSI.exe):");
				ImGui::InputText("##AnnouncerSFMLSoundName", sound_name, MAX_PATH);
				if (ImGui::Button("Play Sound"))
				{
					SFML_PlaySound(sound_name, 0, true);
				}
				ImGui::SameLine();
				if (ImGui::Button("(with delay)"))
				{
					SFML_PlaySound(sound_name, current_offset, true);
				}
			}

			return true;
		}
	};

	struct AnnouncerSFMLOverlay : IPanel
	{
		virtual bool Draw() override final
		{
			if (sound && sound->getStatus() == sf::SoundSource::Status::Stopped)
			{
				delete sound;
				sound = nullptr;
			}

			static int prev = -1;
			static bool should_announce_current = false;

			auto vehicle = OMSI->BulbToys_GetMyVehicle();
			if (subfolder_method == SubfolderMethod::HOF)
			{
				if (!vehicle)
				{
					strncpy_s(hof_or_map, MAX_PATH, "(no_veh)", MAX_PATH - 1);
				}
				else
				{
					auto trv = Read<uintptr_t>(vehicle + 0x710);
					if (!trv)
					{
						strncpy_s(hof_or_map, MAX_PATH, "(no_trv)", MAX_PATH - 1);
					}
					else
					{
						auto myhof = Read<uintptr_t>(vehicle + 0x7C8);
						auto hoefe = Read<uintptr_t>(trv + 0x5E4);
						if (hoefe && OMSI->BulbToys_BoundCheck(hoefe, myhof))
						{
							auto hof = Read<uintptr_t>(hoefe + myhof * 4);
							if (!hof)
							{
								strncpy_s(hof_or_map, MAX_PATH, "(no_hof)", MAX_PATH - 1);
							}
							else
							{
								auto name_wide = Read<wchar_t*>(hof + 0x4);
								if (name_wide)
								{
									char name[64]{ 0 };
									WideStringToString(name_wide, 64, name, 64);
									strncpy_s(hof_or_map, MAX_PATH, name, MAX_PATH - 1);
								}
								else
								{
									strncpy_s(hof_or_map, MAX_PATH, "(no_name)", MAX_PATH - 1);
								}
							}
						}
						else
						{
							strncpy_s(hof_or_map, MAX_PATH, "(no_hoefe)", MAX_PATH - 1);
						}
					}
				}
			}
			else if (subfolder_method == SubfolderMethod::Map)
			{
				strncpy_s(hof_or_map, MAX_PATH, "(TODO)", MAX_PATH - 1);
			}
			else if (subfolder_method == SubfolderMethod::MapFriendly)
			{
				strncpy_s(hof_or_map, MAX_PATH, "(TODO)", MAX_PATH - 1);
			}

			// timetable valid
			if (vehicle && Read<bool>(vehicle + 0x65C))
			{
				auto next = Read<int>(vehicle + 0x680);
				wchar_t* next_name = nullptr;
				int stop_count = -1;
				OMSI->BulbToys_GetTripInfo(Read<int>(vehicle + 0x66C), next, &next_name, &stop_count);

				// do we have a new next stop
				if (prev != next)
				{
					prev = next;
					should_announce_current = true;

					if (next_name && announce_next && (next != 0 || (next == 0 && first_stop_method == FLStopMethod::Both)))
					{
						bool append_terminus = ((next == stop_count) && (terminus_method == FLStopMethod::Both));

						char full_path[MAX_PATH]{ 0 };
						FormatFullPath(full_path, MAX_PATH);

						char next_stop[64]{ 0 };
						WideStringToString(next_name, 64, next_stop, 64);
						for (int i = 0; i < strlen(next_stop); i++)
						{
							if (!IsPathLegalCharacter(next_stop[i], false))
							{
								next_stop[i] = ' ';
							}
						}

						char sound_name[MAX_PATH]{ 0 };
						MYPRINTF(sound_name, MAX_PATH, "%s\\%s%s.wav", full_path, next_stop, append_terminus ? "_#terminus" : "");

						SFML_PlaySound(sound_name, 0, verbose);
					}
				}

				if (announce_current && should_announce_current && Read<float>(vehicle + 0x688) < current_distance)
				{
					should_announce_current = false;

					if (next_name && (next != 0 || (next == 0 && (first_stop_method == FLStopMethod::Both || first_stop_method == FLStopMethod::CurrentOnly))))
					{
						bool append_terminus = ((next == stop_count) && ((terminus_method == FLStopMethod::Both) || (terminus_method == FLStopMethod::CurrentOnly)));

						char full_path[MAX_PATH]{ 0 };
						FormatFullPath(full_path, MAX_PATH);

						char next_stop[64]{ 0 };
						WideStringToString(next_name, 64, next_stop, 64);
						for (int i = 0; i < strlen(next_stop); i++)
						{
							if (!IsPathLegalCharacter(next_stop[i], false))
							{
								next_stop[i] = ' ';
							}
						}

						char sound_name[MAX_PATH]{ 0 };
						MYPRINTF(sound_name, MAX_PATH, "%s\\%s%s.wav", full_path, next_stop, append_terminus ? "_#terminus" : "");

						SFML_PlaySound(sound_name, current_offset, verbose);
					}
				}
			}
			else
			{
				// tt invalid
				prev = -1;
				should_announce_current = false;
			}

			return true;
		}
	};

	IPanel* Panel(Module::DrawType dt)
	{
		if (dt == Module::DrawType::MainWindow)
		{
			return new AnnouncerSFMLPanel();
		}
		else if (dt == Module::DrawType::Overlay)
		{
			return new AnnouncerSFMLOverlay();
		}

		return nullptr;
	}

	void Init()
	{
		Settings::String<"AnnouncerSFML", "Folder", DEFAULT_OMSI_ANNOUNCEMENTS_FOLDER, MAX_PATH> folder_setting;
		strncpy_s(folder, MAX_PATH, folder_setting.Get(), MAX_PATH - 1);

		Settings::String<"AnnouncerSFML", "Subfolder", "Grundorf", MAX_PATH> subfolder_setting;
		strncpy_s(subfolder, MAX_PATH, subfolder_setting.Get(), MAX_PATH - 1);

		Settings::String<"AnnouncerSFML", "SubfolderMethod", "HOF", MAX_PATH> subfolder_method_setting;
		auto new_subfolder_method = subfolder_method_setting.Get();
		if (!_stricmp(new_subfolder_method, "HOF"))
		{
			subfolder_method = SubfolderMethod::HOF;
		}
		else if (!_stricmp(new_subfolder_method, "Map"))
		{
			subfolder_method = SubfolderMethod::Map;
		}
		else if (!_stricmp(new_subfolder_method, "MapFriendly"))
		{
			subfolder_method = SubfolderMethod::MapFriendly;
		}
		else if (!_stricmp(new_subfolder_method, "Custom"))
		{
			subfolder_method = SubfolderMethod::Custom;
		}

		Settings::Float<"AnnouncerSFML", "Volume", 20.0f> volume_setting;
		auto new_volume = volume_setting.Get();
		if (new_volume > 0.0f && new_volume < 100.0f)
		{
			volume = new_volume;
		}

		Settings::String<"AnnouncerSFML", "TerminusMethod", "Both", MAX_PATH> terminus_method_setting;
		auto new_terminus_method = terminus_method_setting.Get();
		if (!_stricmp(new_terminus_method, "Both"))
		{
			terminus_method = FLStopMethod::Both;
		}
		else if (!_stricmp(new_terminus_method, "Neither"))
		{
			terminus_method = FLStopMethod::Neither;
		}
		else if (!_stricmp(new_terminus_method, "CurrentOnly"))
		{
			terminus_method = FLStopMethod::CurrentOnly;
		}

		Settings::String<"AnnouncerSFML", "FirstStopMethod", "Neither", MAX_PATH> first_stop_method_string;
		auto new_first_stop_method = first_stop_method_string.Get();
		if (!_stricmp(new_first_stop_method, "Both"))
		{
			first_stop_method = FLStopMethod::Both;
		}
		else if (!_stricmp(new_first_stop_method, "Neither"))
		{
			first_stop_method = FLStopMethod::Neither;
		}
		else if (!_stricmp(new_first_stop_method, "CurrentOnly"))
		{
			first_stop_method = FLStopMethod::CurrentOnly;
		}

		Settings::Bool<"AnnouncerSFML", "NextStop", false> announce_next_setting;
		announce_next = announce_next_setting.Get();

		Settings::Bool<"AnnouncerSFML", "CurrentStop", false> announce_current_setting;
		announce_current = announce_current_setting.Get();

		Settings::Int32<"AnnouncerSFML", "CurrentOffset", 0> current_offset_setting;
		auto new_current_offset = current_offset_setting.Get();
		if (new_current_offset > 0)
		{
			current_offset = new_current_offset;
		}

		Settings::Float<"AnnouncerSFML", "CurrentDistance", 20.0f> current_distance_setting;
		current_distance = current_distance_setting.Get();
	}

	void End()
	{

	}
}

MODULE(announcer_sfml);