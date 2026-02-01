#include "../../core/bulbtoys.h"
#include "../omsi.h"

namespace vehicle
{
	namespace pedals
	{
		void PatchFLD(uintptr_t addy, float* flt)
		{
			Unprotect _(addy, 6);
			Patch<uint16_t>(addy, 0x05D9);
			Patch<float*>(addy + 2, flt);
		}

		void UnpatchFLD(uintptr_t addy)
		{
			Unprotect _(addy, 6);
			Unpatch(addy + 2);
			Unpatch(addy);
		}

		namespace throttle
		{
			constexpr float default_value = 2.0f;

			bool override = false;
			float* address = nullptr;

			Unprotect* unprotect = nullptr;

			void Update()
			{
				if (override)
				{
					unprotect = new Unprotect(address, 4);
				}
				else
				{
					*address = default_value;
					delete unprotect;
				}
			}
		}

		namespace throttle_release
		{
			constexpr float default_value = 1.0f;

			bool override = false;
			float value = default_value;

			void Update()
			{
				static uintptr_t addy = OMSI->BulbToys_GetThrottleReleaseInstructionAddress();

				if (override)
				{
					PatchFLD(addy, &value);
				}
				else
				{
					value = default_value;
					UnpatchFLD(addy);
				}
			}
		}

		namespace max_throttle
		{
			constexpr float default_value = 0.85f;

			bool override = false;
			float* address = nullptr;

			Unprotect* unprotect = nullptr;

			void Update()
			{
				if (override)
				{
					unprotect = new Unprotect(address, 4);
				}
				else
				{
					*address = default_value;
					delete unprotect;
				}
			}
		}

		namespace brakes
		{
			constexpr float default_value = 1.0f;

			bool override = false;
			float value = default_value;

			void Update()
			{
				static uintptr_t addy = OMSI->BulbToys_GetBrakesInstructionAddress();

				if (override)
				{
					PatchFLD(addy, &value);
				}
				else
				{
					value = default_value;
					UnpatchFLD(addy);
				}
			}
		}

		namespace brakes_release
		{
			constexpr float default_value = 0.5f;

			bool override = false;
			float value = default_value;

			void Update()
			{
				static uintptr_t addy = OMSI->BulbToys_GetBrakesReleaseInstructionAddress();

				if (override)
				{
					PatchFLD(addy, &value);
				}
				else
				{
					value = default_value;
					UnpatchFLD(addy);
				}
			}
		}

		namespace clutch_release
		{
			constexpr float default_value = 0.7f;

			bool override = false;
			float value = default_value;

			void Update()
			{
				static uintptr_t addy = OMSI->BulbToys_GetClutchReleaseInstructionAddress();

				if (override)
				{
					PatchFLD(addy, &value);
				}
				else
				{
					value = default_value;
					UnpatchFLD(addy);
				}
			}
		}
	}

	struct VehiclePanel : IPanel
	{
		virtual bool Draw() override final
		{
			if (ImGui::BulbToys_Menu("Vehicle"))
			{
				static bool on_depot = false;
				if (ImGui::Checkbox("Force 'On Depot'", &on_depot))
				{
					OMSI->BulbToys_ForceOnDepot(on_depot);
				}

				ImGui::Separator();

				auto trvinst = OMSI->BulbToys_GetMyVehicle();
				ImGui::BulbToys_AddyLabel(trvinst, "Current");

				if (trvinst)
				{
					auto dirt = reinterpret_cast<float*>(trvinst + 0x5F8);
					ImGui::Text("Dirt");
					ImGui::SliderFloat("##VehicleDirt", dirt, 0, 1);
				}

				ImGui::Separator();

				if (ImGui::Checkbox("Override throttle sensitivity:", &pedals::throttle::override))
				{
					pedals::throttle::Update();
				}
				ImGui::BeginDisabled(!pedals::throttle::override);
				ImGui::SliderFloat("##ThrottleSens", pedals::throttle::address, 0.0f, 2.0f);
				ImGui::EndDisabled();

				if (ImGui::Checkbox("Override throttle release sensitivity:", &pedals::throttle_release::override))
				{
					pedals::throttle_release::Update();
				}
				ImGui::BeginDisabled(!pedals::throttle_release::override);
				ImGui::SliderFloat("##ThrottleReleaseSens", &pedals::throttle_release::value, 0.0f, 2.0f);
				ImGui::EndDisabled();

				if (ImGui::Checkbox("Override max throttle without [NUM+]:", &pedals::max_throttle::override))
				{
					pedals::max_throttle::Update();
				}
				ImGui::BeginDisabled(!pedals::max_throttle::override);
				ImGui::SliderFloat("##MaxThrottleSens", pedals::max_throttle::address, 0.0f, 1.0f);
				ImGui::EndDisabled();

				if (ImGui::Checkbox("Override brakes sensitivity:", &pedals::brakes::override))
				{
					pedals::brakes::Update();
				}
				ImGui::BeginDisabled(!pedals::brakes::override);
				ImGui::SliderFloat("##BrakesSens", &pedals::brakes::value, 0.0f, 2.0f);
				ImGui::EndDisabled();

				if (ImGui::Checkbox("Override brakes release sensitivity:", &pedals::brakes_release::override))
				{
					pedals::brakes_release::Update();
				}
				ImGui::BeginDisabled(!pedals::brakes_release::override);
				ImGui::SliderFloat("##BrakesReleaseSens", &pedals::brakes_release::value, 0.0f, 2.0f);
				ImGui::EndDisabled();

				if (ImGui::Checkbox("Override clutch release sensitivity:", &pedals::clutch_release::override))
				{
					pedals::clutch_release::Update();
				}
				ImGui::BeginDisabled(!pedals::clutch_release::override);
				ImGui::SliderFloat("##ClutchReleaseSens", &pedals::clutch_release::value, 0.0f, 2.0f);
				ImGui::EndDisabled();
			}

			return true;
		}
	};

	IPanel* Panel(Module::DrawType dt)
	{
		if (dt == Module::DrawType::MainWindow)
		{
			return new VehiclePanel();
		}

		return nullptr;
	}

	void Init()
	{
		pedals::throttle::address = OMSI->BulbToys_GetThrottleIncrementAddress();
		pedals::max_throttle::address = OMSI->BulbToys_GetMaxThrottleAddress();

		Settings::Bool<"Vehicles", "Pedals_Throttle_Override", false> throttle_override_setting;
		pedals::throttle::override = throttle_override_setting.Get();

		Settings::Float<"Vehicles", "Pedals_Throttle", pedals::throttle::default_value> throttle_setting;
		if (pedals::throttle::override)
		{
			pedals::throttle::Update();
			*pedals::throttle::address = throttle_setting.Get();
		}

		Settings::Bool<"Vehicles", "Pedals_ThrottleRelease_Override", false> throttle_release_override_setting;
		pedals::throttle_release::override = throttle_release_override_setting.Get();

		Settings::Float<"Vehicles", "Pedals_ThrottleRelease", pedals::throttle_release::default_value> throttle_release_setting;
		if (pedals::throttle_release::override)
		{
			pedals::throttle_release::value = throttle_release_setting.Get();
			pedals::throttle_release::Update();
		}

		Settings::Bool<"Vehicles", "Pedals_MaxThrottle_Override", false> max_throttle_override_setting;
		pedals::max_throttle::override = max_throttle_override_setting.Get();

		Settings::Float<"Vehicles", "Pedals_MaxThrottle", pedals::max_throttle::default_value> max_throttle_setting;
		if (pedals::max_throttle::override)
		{
			pedals::max_throttle::Update();
			*pedals::max_throttle::address = max_throttle_setting.Get();
		}

		Settings::Bool<"Vehicles", "Pedals_Brakes_Override", false> brakes_override_setting;
		pedals::brakes::override = brakes_override_setting.Get();

		Settings::Float<"Vehicles", "Pedals_Brakes", pedals::brakes::default_value> brakes_setting;
		if (pedals::brakes::override)
		{
			pedals::brakes::value = brakes_setting.Get();
			pedals::brakes::Update();
		}

		Settings::Bool<"Vehicles", "Pedals_BrakesRelease_Override", false> brakes_release_override_setting;
		pedals::brakes_release::override = brakes_release_override_setting.Get();

		Settings::Float<"Vehicles", "Pedals_BrakesRelease", pedals::brakes_release::default_value> brakes_release_setting;
		if (pedals::brakes_release::override)
		{
			pedals::brakes_release::value = brakes_release_setting.Get();
			pedals::brakes_release::Update();
		}

		Settings::Bool<"Vehicles", "Pedals_ClutchRelease_Override", false> clutch_release_override_setting;
		pedals::clutch_release::override = clutch_release_override_setting.Get();

		Settings::Float<"Vehicles", "Pedals_ClutchRelease", pedals::clutch_release::default_value> clutch_release_setting;
		if (pedals::clutch_release::override)
		{
			pedals::clutch_release::value = clutch_release_setting.Get();
			pedals::clutch_release::Update();
		}
	}

	void End()
	{
		if (pedals::throttle::override)
		{
			pedals::throttle::override = false;
			pedals::throttle::Update();
		}

		if (pedals::throttle_release::override)
		{
			pedals::throttle_release::override = false;
			pedals::throttle_release::Update();
		}

		if (pedals::max_throttle::override)
		{
			pedals::max_throttle::override = false;
			pedals::max_throttle::Update();
		}

		if (pedals::brakes::override)
		{
			pedals::brakes::override = false;
			pedals::brakes::Update();
		}

		if (pedals::brakes_release::override)
		{
			pedals::brakes_release::override = false;
			pedals::brakes_release::Update();
		}

		if (pedals::clutch_release::override)
		{
			pedals::clutch_release::override = false;
			pedals::clutch_release::Update();
		}
	}
}

MODULE(vehicle);