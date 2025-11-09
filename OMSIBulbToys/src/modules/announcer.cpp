#include "../../core/bulbtoys.h"
#include "../omsi.h"

namespace announcer
{
	bool enabled = false;
	uintptr_t sound = 0;

	struct AnnouncerPanel : IPanel
	{
		virtual bool Draw() override final
		{
			if (ImGui::BulbToys_Menu("Announcer"))
			{
				ImGui::Checkbox("Enabled", &enabled);
				if (sound)
				{
					ImGui::BulbToys_AddyLabel(sound, "Sound");

					ImGui::Text("Volume:");
					ImGui::SliderFloat("##AnnouncerVolume", reinterpret_cast<float*>(sound + 0x6C), 0.0f, 1.0f);

					ImGui::Separator();

					char sound_name[MAX_PATH] { 0 };
					ImGui::Text("Sound name (relative to OMSI.exe):");
					ImGui::InputText("##AnnouncerSoundName", sound_name, MAX_PATH);
					if (ImGui::Button("Play Sound"))
					{
						char* sound_name_ansi = Game::AnsiString<MAX_PATH>(sound_name).string;

						OMSI->BulbToys_TSound_Play(sound, sound_name);
					}
				}
			}

			return true;
		}
	};

	struct AnnouncerOverlay : IPanel
	{
		virtual bool Draw() override final
		{
			if (!sound)
			{
				sound = OMSI->BulbToys_TSound_Create();
			}

			auto vehicle = OMSI->BulbToys_GetMyVehicle();
			if (enabled && vehicle && Read<bool>(vehicle + 0x65C))
			{
				auto next = Read<int>(vehicle + 0x680);
				if (next == 0)
				{
					next = 1;
				}

				static auto prev = next;
				if (prev != next)
				{
					prev = next;

					wchar_t* next_name = nullptr;
					int stop_count = -1;
					OMSI->BulbToys_GetTripInfo(Read<int>(vehicle + 0x66C), next, &next_name, &stop_count);

					char next_stop[64] { 0 };
					if (next_name)
					{
						WideStringToString(next_name, 64, next_stop, 64);
					}

					const char* hof = "Novi Sad";
					// trvinst -> myhof
					// trvinst -> trv -> (THof)hoefe[myhof] -> name

					char sound_name[MAX_PATH] { 0 };
					MYPRINTF(sound_name, MAX_PATH, "Vehicles\\Announcements\\%s\\%s%s.wav", hof, next_stop, (next == stop_count) ? "_#terminus" : "");
					OMSI->BulbToys_TSound_Play(sound, sound_name);
				}
			}

			return true;
		}
	};

	IPanel* Panel(Module::DrawType dt)
	{
		if (dt == Module::DrawType::MainWindow)
		{
			return new AnnouncerPanel();
		}
		else if (dt == Module::DrawType::Overlay)
		{
			return new AnnouncerOverlay();
		}

		return nullptr;
	}
}

MODULE_PANEL_ONLY(announcer);