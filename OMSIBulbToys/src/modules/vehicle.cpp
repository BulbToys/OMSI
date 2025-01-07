#include "../../core/bulbtoys.h"
#include "../omsi.h"

namespace vehicle
{
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
					ImGui::BulbToys_SliderFloat("Dirt", "##VehicleDirt", dirt, 0, 1);
				}
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
}

MODULE_PANEL_ONLY(vehicle);