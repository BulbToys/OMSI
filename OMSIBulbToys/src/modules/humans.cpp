#include "../../core/bulbtoys.h"
#include "../omsi.h"

namespace humans
{
	const char* vehicle_filter_methods[] = { "All, regardless of vehicle", "Only in own vehicle", "Only not in vehicle"};
	enum VehicleFilterMethod : int
	{
		All = 0,
		OwnOnly,
		NoneOnly,
	};

	struct HumansPanel : IPanel
	{
		int vehicle_filter_method = VehicleFilterMethod::All;

		bool only_in_world = false;

		virtual bool Draw() override final
		{
			if (ImGui::BulbToys_Menu("Humans"))
			{
				// TODO offset
				uintptr_t humans_list = Read<uintptr_t>(0x86172C);

				auto humans_count = OMSI->BulbToys_ListLength(humans_list);
				ImGui::Text("Count: %d", OMSI->BulbToys_ListLength(humans_list));

				ImGui::Text("Vehicle filter method:");
				ImGui::Combo("##HumansVehicleMethod", &vehicle_filter_method, vehicle_filter_methods, IM_ARRAYSIZE(vehicle_filter_methods));

				ImGui::Checkbox("Only in world", &only_in_world);

				constexpr auto column_count = 3;
				if (ImGui::BeginTable("HumansTable", column_count, ImGuiTableFlags_SizingFixedFit))
				{
					ImGui::TableNextRow();

					ImGui::TableSetColumnIndex(0);
					ImGui::Text("#");

					ImGui::TableSetColumnIndex(1);
					ImGui::Text("Human Info");

					ImGui::TableSetColumnIndex(2);
					ImGui::Text("Bus/Trip Info");

					int displayed_row = 1;
					for (int row = 0; row < humans_count; row++)
					{
						auto human_instance = Read<uintptr_t>(humans_list + row * 4);

						auto vehicle = Read<uintptr_t>(human_instance + 0x6B4);
						if (vehicle_filter_method == VehicleFilterMethod::NoneOnly)
						{
							if (vehicle)
							{
								continue;
							}
						}
						else if (vehicle_filter_method == VehicleFilterMethod::OwnOnly)
						{
							auto my_vehicle = OMSI->BulbToys_GetMyVehicle();
							if (!my_vehicle)
							{
								break;
							}

							auto my_first_vehicle = Read<uintptr_t>(my_vehicle + 0x4D0);
							if (!my_first_vehicle)
							{
								break;
							}

							if (!vehicle)
							{
								continue;
							}

							auto first_vehicle = Read<uintptr_t>(vehicle + 0x4D0);
							if (my_first_vehicle != first_vehicle)
							{
								continue;
							}
						}

						auto in_world = Read<bool>(human_instance + 0x5EF);
						if (only_in_world)
						{
							if (!in_world)
							{
								continue;
							}
						}

						ImGui::TableNextRow();

						for (int column = 0; column < column_count; column++)
						{
							ImGui::TableSetColumnIndex(column);

							if (column == 0)
							{
								ImGui::Text("/ \n| \n| \n| \n| \n| \n| \n| \n| \n| \n| \n| ");

								ImGui::Text("%d", displayed_row++);

								ImGui::Text("| \n| \n| \n| \n| \n| \n| \n| \n| \n| \n| \n\\ ");

								ImGui::Text(" ");
							}
							else if (column == 1)
							{
								auto human = Read<uintptr_t>(human_instance + 0x5B0);
								ImGui::BulbToys_AddyLabel(human, "THuman");

								if (human)
								{
									auto filename = Read<char*>(human + 0x4);
									ImGui::Text("Filename: %s", (filename ? filename : "(null)"));

									auto exists = Read<bool>(human + 0x196);
									ImGui::Text("Exists:");
									ImGui::SameLine();
									ImGui::TextColored((exists ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1)), (exists ? "True" : "False"));

									auto model = Read<char*>(human + 0x1A4);
									ImGui::Text("Model: %s", (model ? model : "(null)"));

									auto my_path = Read<char*>(human + 0x1A8);
									ImGui::Text("My Path: %s", (my_path ? my_path : "(null)"));

									auto voice = Read<char*>(human + 0x264);
									ImGui::Text("Voice: %s", (voice ? voice : "(null)"));

									ImGui::Text("Age: %.02f", Read<float>(human + 0x268));
									ImGui::Text("Height: %.02f", Read<float>(human + 0x268));
								}

								ImGui::Text("-----");

								ImGui::Text("Target: (%.02f, %.02f, %.02f)", Read<float>(human_instance + 0x5BD), Read<float>(human_instance + 0x5BD + 4), Read<float>(human_instance + 0x5BD + 8));

								ImGui::Text("In World:");
								ImGui::SameLine();
								ImGui::TextColored((in_world ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1)), (in_world ? "True" : "False"));

								ImGui::Text("Last Moved Distance: %.02f", Read<float>(human_instance + 0x644));

								char human_max_speed_str[32]{ 0 };
								MYPRINTF(human_max_speed_str, 32, "##HumanMaxSpeed%08X", human_instance);
								ImGui::Text("Max Speed:");
								ImGui::SliderFloat(human_max_speed_str, reinterpret_cast<float*>(human_instance + 0x6AC), 0.9, 1.3);

								ImGui::Text("AIMode: %u", Read<uint8_t>(human_instance + 0x6C4));
								ImGui::Text("AIModeEx: %u", Read<uint8_t>(human_instance + 0x6C5));
								ImGui::Text("AISubMode: %u", Read<uint8_t>(human_instance + 0x6C6));
							}
							else if (column == 2)
							{
								auto first_vehicle_instance = (vehicle ? Read<uintptr_t>(vehicle + 0x4D0) : 0);

								ImGui::BulbToys_AddyLabel(first_vehicle_instance, "TRVInst (first)");

								if (first_vehicle_instance)
								{
									auto first_vehicle = Read<uintptr_t>(first_vehicle_instance + 0x710);
									if (first_vehicle)
									{
										auto brand = Read<char*>(first_vehicle + 0x5D8);
										auto name = Read<char*>(first_vehicle + 0x19C);
										ImGui::Text("Name: %s %s", (brand ? brand : "(null)"), (name ? name : "(null)"));

										auto target = Read<char*>(first_vehicle_instance + 0x7BC);
										ImGui::Text("Target: %s", (target ? target : "(null)"));
									}
								}

								ImGui::Text("-----");

								auto target_station = Read<char*>(human_instance + 0x5F4);
								ImGui::Text("Target Station: %s", (target_station ? target_station : "(null)"));

								ImGui::Text("Target Station Termini Index: %d", Read<int>(human_instance + 0x5F8));

								auto pre_target_station = Read<char*>(human_instance + 0x5FC);
								ImGui::Text("Pre Target Station: %s", (pre_target_station ? pre_target_station : "(null)"));

								auto pts_reached = Read<bool>(human_instance + 0x600);
								ImGui::Text("Pre Target Station Reached:");
								ImGui::SameLine();
								ImGui::TextColored((pts_reached ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1)), (pts_reached ? "True" : "False"));

								ImGui::Text("Enter Bus At: %.02f", Read<float>(human_instance + 0x60C));
								ImGui::Text("Seat Nr. Bus: %d", Read<int>(human_instance + 0x610));
								ImGui::Text("Bus Part Offset: %d", Read<int>(human_instance + 0x614));
								ImGui::Text("Seat Nr. Station: %d", Read<int>(human_instance + 0x618));
								ImGui::Text("Ticket Type: %u", Read<uint8_t>(human_instance + 0x61C));
								ImGui::Text("My Entry/Exit: %d", Read<int>(human_instance + 0x640));
								ImGui::Text("My Bus Code: %u", Read<uint32_t>(human_instance + 0x6B8));

								auto my_station = Read<int>(human_instance + 0x6BC);
								ImGui::Text("My Station: %d", my_station);
								ImGui::Text("My Station ID Code: %u", Read<uint32_t>(human_instance + 0x6C0));

								// todo add to offsets
								auto station_mgr = Read<uintptr_t>(0x861728);

								bool is_valid = false;

								constexpr uintptr_t TStationMan_IndexValid = 0x61D9AC;

								__asm
								{
									mov   edx, my_station
									mov   eax, station_mgr
									call  TStationMan_IndexValid
									mov   is_valid, al
								}

								if (is_valid)
								{
									uintptr_t station_inst = 0;

									constexpr uintptr_t TStationMan_GetStationInst = 0x61D994;

									__asm
									{
										mov   edx, my_station
										mov   eax, station_mgr
										call  TStationMan_GetStationInst
										mov   station_inst, eax
									}

									if (station_inst)
									{
										ImGui::Text("-----");

										auto station_name = Read<char*>(station_inst + 0x14);
										auto station_name_extra = Read<char*>(station_inst + 0x10);
										ImGui::Text("My Station Name: %s (%s)", (station_name ? station_name : "(null)"), (station_name_extra ? station_name_extra : "(null)"));
									}
								}
							}
						}
					}

					ImGui::EndTable();
				}
			}

			return true;
		}
	};

	IPanel* Panel(Module::DrawType dt)
	{
		if (dt == Module::DrawType::MainWindow)
		{
			return new HumansPanel();
		}

		return nullptr;
	}
}

MODULE_PANEL_ONLY(humans);