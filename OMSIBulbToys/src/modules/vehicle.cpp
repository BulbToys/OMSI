#include "../../core/bulbtoys.h"
#include "../omsi.h"

namespace vehicle
{
	namespace pedals
	{
		namespace throttle
		{
			constexpr float default_value = 2.0f;

			bool override = false;
			float* address = nullptr;

			Unprotect* unprotect = nullptr;
		}

		namespace throttle_release
		{
			constexpr float default_value = 1.0f;

			bool override = false;
			float value = default_value;
		}

		namespace max_throttle
		{
			constexpr float default_value = 0.85f;

			bool override = false;
			float* address = nullptr;

			Unprotect* unprotect = nullptr;
		}

		namespace brakes
		{
			constexpr float default_value = 1.0f;

			bool override = false;
			float value = default_value;
		}

		namespace brakes_release
		{
			constexpr float default_value = 0.5f;

			bool override = false;
			float value = default_value;
		}

		namespace clutch_release
		{
			constexpr float default_value = 0.7f;

			bool override = false;
			float value = default_value;
		}
	}

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
					if (pedals::throttle::override)
					{
						pedals::throttle::unprotect = new Unprotect(pedals::throttle::address, 4);
					}
					else
					{
						*pedals::throttle::address = pedals::throttle::default_value;
						delete pedals::throttle::unprotect;
					}
				}
				ImGui::BeginDisabled(!pedals::throttle::override);
				ImGui::SliderFloat("##ThrottleSens", pedals::throttle::address, 0.0f, 2.0f);
				ImGui::EndDisabled();

				if (ImGui::Checkbox("Override throttle release sensitivity:", &pedals::throttle_release::override))
				{
					uintptr_t addy = OMSI->BulbToys_GetThrottleReleaseInstructionAddress();

					if (pedals::throttle_release::override)
					{
						PatchFLD(addy, &pedals::throttle_release::value);
					}
					else
					{
						pedals::throttle_release::value = pedals::throttle_release::default_value;
						UnpatchFLD(addy);
					}
				}
				ImGui::BeginDisabled(!pedals::throttle_release::override);
				ImGui::SliderFloat("##ThrottleReleaseSens", &pedals::throttle_release::value, 0.0f, 2.0f);
				ImGui::EndDisabled();

				if (ImGui::Checkbox("Override max throttle without [NUM+]:", &pedals::max_throttle::override))
				{
					if (pedals::max_throttle::override)
					{
						pedals::max_throttle::unprotect = new Unprotect(pedals::max_throttle::address, 4);
					}
					else
					{
						*pedals::max_throttle::address = pedals::max_throttle::default_value;
						delete pedals::max_throttle::unprotect;
					}
				}
				ImGui::BeginDisabled(!pedals::max_throttle::override);
				ImGui::SliderFloat("##MaxThrottleSens", pedals::max_throttle::address, 0.0f, 1.0f);
				ImGui::EndDisabled();

				if (ImGui::Checkbox("Override brakes sensitivity:", &pedals::brakes::override))
				{
					uintptr_t addy = OMSI->BulbToys_GetBrakesInstructionAddress();

					if (pedals::brakes::override)
					{
						PatchFLD(addy, &pedals::brakes::value);
					}
					else
					{
						pedals::brakes::value = pedals::brakes::default_value;
						UnpatchFLD(addy);
					}
				}
				ImGui::BeginDisabled(!pedals::brakes::override);
				ImGui::SliderFloat("##BrakesSens", &pedals::brakes::value, 0.0f, 2.0f);
				ImGui::EndDisabled();

				if (ImGui::Checkbox("Override brakes release sensitivity:", &pedals::brakes_release::override))
				{
					uintptr_t addy = OMSI->BulbToys_GetBrakesReleaseInstructionAddress();

					if (pedals::brakes_release::override)
					{
						PatchFLD(addy, &pedals::brakes_release::value);
					}
					else
					{
						pedals::brakes_release::value = pedals::brakes_release::default_value;
						UnpatchFLD(addy);
					}
				}
				ImGui::BeginDisabled(!pedals::brakes_release::override);
				ImGui::SliderFloat("##BrakesReleaseSens", &pedals::brakes_release::value, 0.0f, 2.0f);
				ImGui::EndDisabled();

				if (ImGui::Checkbox("Override clutch release sensitivity:", &pedals::clutch_release::override))
				{
					uintptr_t addy = OMSI->BulbToys_GetClutchReleaseInstructionAddress();

					if (pedals::clutch_release::override)
					{
						PatchFLD(addy, &pedals::clutch_release::value);
					}
					else
					{
						pedals::clutch_release::value = pedals::clutch_release::default_value;
						UnpatchFLD(addy);
					}
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
	}

	void End()
	{
		if (pedals::throttle::override)
		{
			*pedals::throttle::address = pedals::throttle::default_value;
			delete pedals::throttle::unprotect;
		}

		if (pedals::throttle_release::override)
		{
			UnpatchFLD(OMSI->BulbToys_GetThrottleReleaseInstructionAddress());
		}

		if (pedals::max_throttle::override)
		{
			*pedals::max_throttle::address = pedals::max_throttle::default_value;
			delete pedals::max_throttle::unprotect;
		}

		if (pedals::brakes::override)
		{
			UnpatchFLD(OMSI->BulbToys_GetBrakesInstructionAddress());
		}

		if (pedals::brakes_release::override)
		{
			UnpatchFLD(OMSI->BulbToys_GetBrakesReleaseInstructionAddress());
		}

		if (pedals::clutch_release::override)
		{
			UnpatchFLD(OMSI->BulbToys_GetClutchReleaseInstructionAddress());
		}
	}
}

MODULE(vehicle);