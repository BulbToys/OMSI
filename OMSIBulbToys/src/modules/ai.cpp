#include "../../core/bulbtoys.h"
#include "../omsi.h"

#include <random>

namespace ai
{
	namespace passenger
	{
		char status[1024] = { 0 };

		float max_chance = 80.0f;
		float min_chance = 40.0f;

		float short_trip = 5.0f;
		float short_modifier = 40.0f;

		float younger_than = 16.0f;
		float young_modifier = 10.0f;

		float older_than = 55.0f;
		float old_modifier = 20.0f;

		float abs_random_modifier = 10.0f;

		int PreferSitting(uintptr_t* eligibles_ptr, uintptr_t human_seat_nr_bus)
		{
			// if there's 0 or 1 seat left, don't bother
			auto len = OMSI->BulbToys_ListLength(*eligibles_ptr);
			if (len < 2)
			{
				return len;
			}

			// offset from &human->SeatNrBus back to base human pointer
			auto human_inst = human_seat_nr_bus - 0x610;

			auto eligibles = reinterpret_cast<Game::Tfreeseats*>(*eligibles_ptr);

			std::vector<Game::Tfreeseats> new_eligible_seats;
			std::vector<Game::Tfreeseats> new_eligible_stands;

			int total_seats = 0;
			std::vector<uintptr_t> checked_vehicles;

			for (int i = 0; i < len; i++)
			{
				auto eligible = eligibles[i];

				auto vehicle = eligible.vehicle;
				if (!vehicle)
				{
					continue;
				}

				auto road_vehicle = Read<uintptr_t>(vehicle + 0x710);
				if (!road_vehicle)
				{
					continue;
				}

				auto passenger_cabin = Read<uintptr_t>(road_vehicle + 0x190);
				if (!passenger_cabin)
				{
					continue;
				}

				auto seats = Read<uintptr_t>(passenger_cabin + 0x4);

				// if we haven't checked this vehicle yet (each articulated part is its own vehicle)
				if (checked_vehicles.end() == std::find(checked_vehicles.begin(), checked_vehicles.end(), vehicle))
				{
					// don't check this vehicle again
					checked_vehicles.push_back(vehicle);

					for (int j = 0; j < OMSI->BulbToys_ListLength(seats); j++)
					{
						auto seat = reinterpret_cast<Game::TSeat*>(seats + j * sizeof(Game::TSeat));

						if (seat->height > 0.0f)
						{
							// add to total count of seats in entire bus
							total_seats++;
						}
					}
				}

				auto index = eligible.index;
				if (!OMSI->BulbToys_BoundCheck(seats, index))
				{
					continue;
				}

				auto seat = reinterpret_cast<Game::TSeat*>(seats + index * sizeof(Game::TSeat));

				if (seat->height > 0.0f)
				{
					new_eligible_seats.push_back(eligible);
				}
				else
				{
					new_eligible_stands.push_back(eligible);
				}
			}

			auto free_seats = new_eligible_seats.size();
			auto free_stands = new_eligible_stands.size();
			if (free_seats == 0 || free_stands == 0)
			{
				// don't bother calculating sitting chance, we don't have a choice anyways (forced to sit or stand)
				return len;
			}

			// scale initial sitting chance based on the amount of free seats in the bus
			float sit_chance = min_chance + ((max_chance - min_chance) * ((float)free_seats / (float)total_seats));

			// apply modifier based on estimated trip length, IN SECONDS
			auto est_travel_time = NAN;

			// get primary vehicle
			auto vehicle = Read<uintptr_t>(checked_vehicles[0] + 0x4D0);
			auto target_stop_name = Read<char*>(human_inst + 0x5F4);
			if (vehicle && target_stop_name && strlen(target_stop_name))
			{
				auto ttman = reinterpret_cast<Game::TTimeTableMan*>(OMSI->BulbToys_GetTimeTableManager());
				if (ttman && ttman->Trips)
				{
					auto trip = Read<int>(vehicle + 0x66C);
					if (OMSI->BulbToys_BoundCheck(reinterpret_cast<uintptr_t>(ttman->Trips), trip))
					{
						auto profile = Read<int>(vehicle + 0x67C);
						if (ttman->Trips[trip].profiles && OMSI->BulbToys_BoundCheck(reinterpret_cast<uintptr_t>(ttman->Trips[trip].profiles), profile))
						{
							auto bus_stops = ttman->Trips[trip].busstops;
							if (bus_stops)
							{
								auto stop_index = Read<int>(vehicle + 0x680);

								for (int i = stop_index + 1; i < OMSI->BulbToys_ListLength(reinterpret_cast<uintptr_t>(bus_stops)); i++)
								{
									auto bus_stop = bus_stops[i];

									auto stop_name_wide = bus_stop.name;
									if (!stop_name_wide)
									{
										continue;
									}

									auto stop_name_len = ::WideStringToString(stop_name_wide, -1);
									if (!stop_name_len)
									{
										continue;
									}

									char* stop_name = new char[stop_name_len];
									::WideStringToString(stop_name_wide, stop_name_len, stop_name, stop_name_len);

									if (!strncmp(stop_name, target_stop_name, stop_name_len))
									{
										est_travel_time = ttman->Trips[trip].profiles[profile].stop_times[i].arr_time - ttman->Trips[trip].profiles[profile].stop_times[stop_index].arr_time;
										delete[] stop_name;
										break;
									}

									delete[] stop_name;
								}
							}
						}
					}
				}
			}

			if (!isnan(est_travel_time))
			{
				float short_trip_secs = short_trip * 60.0f;

				float travel_time_leftover = short_trip_secs - est_travel_time;
				if (travel_time_leftover > 0.0f)
				{
					sit_chance -= short_modifier * (travel_time_leftover / short_trip_secs);
				}
			}

			// apply modifier based on age
			auto human = Read<uintptr_t>(human_inst + 0x5B0);
			if (human)
			{
				auto age = Read<float>(human + 0x268);
				if (age < younger_than)
				{
					sit_chance += young_modifier;
				}
				else if (age > older_than)
				{
					sit_chance += old_modifier;
				}
			}

			// apply absolute randomness modifier
			sit_chance = OMSI->RandomCentered(abs_random_modifier, sit_chance);
			if (sit_chance > 100.0f)
			{
				sit_chance = 100.0f;
			}
			else if (sit_chance < 0.0f)
			{
				sit_chance = 0.0f;
			}

			auto sit_factor = sit_chance / 100.0f;

			float reserved_seats = free_seats;
			float reserved_stands = (free_seats / sit_factor) - reserved_seats;

			// estimated too many stands, can't fulfill promise, so calculate reserved stands first instead (infinity is okay here)
			if (reserved_stands > (float)free_stands)
			{
				reserved_stands = free_stands;
				reserved_seats = (reserved_stands / (1.0f - sit_factor)) - reserved_stands;
			}

			std::random_device rd;
			std::mt19937 rng(rd());
			std::vector<Game::Tfreeseats> new_eligibles;
			std::sample(new_eligible_seats.begin(), new_eligible_seats.end(), std::back_inserter(new_eligibles), (int)round(reserved_seats), rng);
			std::sample(new_eligible_stands.begin(), new_eligible_stands.end(), std::back_inserter(new_eligibles), (int)round(reserved_stands), rng);

			auto new_len = new_eligibles.size();
			std::copy(new_eligibles.begin(), new_eligibles.end(), eligibles);
			OMSI->DynArraySetLength(eligibles_ptr, OMSI->BulbToys_GetEligibleSeatsRTTIAddress(), 1, new_len);

			MYPRINTF(
				status,
				1024,
				"- Length: %d\n"
				"- Human: %08X\n"
				"- Age: %.02f\n"
				"- Est. travel time: %.02f min\n"
				"- Checked vehicles: %u\n"
				"- Free seats: %u\n"
				"- Free stands: %u\n"
				"- Precalc sit chance: %.02f\n"
				"- Calculated sit chance: %.02f\n"
				"- Reserved seats: %.02f\n"
				"- Reserved stands: %.02f\n"
				"- New length: %u",
				len,
				human,
				human? Read<float>(human + 0x268) : NAN,
				est_travel_time / 60.0f,
				checked_vehicles.size(),
				free_seats,
				free_stands,
				min_chance + ((max_chance - min_chance) * ((float)free_seats / (float)total_seats)),
				sit_chance,
				reserved_seats,
				reserved_stands,
				new_len);

			return new_len;
		}

		void __declspec(naked) PreferSittingHook()
		{
			__asm
			{
				push  [ebp - 8]
				push  eax
				call  PreferSitting
				add   esp, 8
				retn
			}
		}
	}

	struct AIPanel : IPanel
	{
		float fps = 0;
		float dt = 0;
		int ms = 1000;

		virtual bool Draw() override final
		{
			if (ImGui::BulbToys_Menu("AI"))
			{
				static bool prefer_sitting = false;
				if (ImGui::Checkbox("Override passenger sitting AI", &prefer_sitting))
				{
					// TODO FIXME add to omsi offsets
					auto address = OMSI->BulbToys_GetEligibleSeatsCallAddress();
					Unprotect _(address - 3, 6 + 3);

					if (prefer_sitting)
					{
						Patch<uint8_t>(address - 3, 0x8D);
						PatchCall(address, passenger::PreferSittingHook);
					}
					else
					{
						Unpatch(address);
						Unpatch(address - 3);
					}
				}

				ImGui::Text("Status:\n%s", passenger::status);

				ImGui::Separator();

				ImGui::Text("Maximum sitting chance (%%):");
				ImGui::SliderFloat("##AIPaxMaxChance", &passenger::max_chance, -100.0f, 200.0f);

				ImGui::Text("Minimum sitting chance (%%):");
				ImGui::SliderFloat("##AIPaxMinChance", &passenger::min_chance, -100.0f, 200.0f);

				ImGui::Separator();

				ImGui::Text("Stand if trip is shorter than (min):");
				ImGui::SliderFloat("##AIPaxShortTrip", &passenger::short_trip, 0.0f, 10.0f);

				ImGui::Text("Short trip standing chance modifier (%%):");
				ImGui::SliderFloat("##AIPaxShortMod", &passenger::short_modifier, 0.0f, 100.0f);

				ImGui::Separator();

				ImGui::Text("Sit if younger than (years old):");
				ImGui::SliderFloat("##AIPaxYounger", &passenger::younger_than, 0.0f, 18.0f);

				ImGui::Text("Young sitting chance modifier (%%):");
				ImGui::SliderFloat("##AIPaxYoungMod", &passenger::young_modifier, -100.0f, 100.0f);

				ImGui::Separator();

				ImGui::Text("Sit if older than (years old):");
				ImGui::SliderFloat("##AIPaxOlder", &passenger::older_than, 50.0f, 120.0f);

				ImGui::Text("Old sitting chance modifier (%%):");
				ImGui::SliderFloat("##AIPaxOldMod", &passenger::old_modifier, -100.0f, 100.0f);

				ImGui::Separator();

				ImGui::Text("Absolute random modifier (+/- |%%|):");
				ImGui::SliderFloat("##AIPaxAbsRndMod", &passenger::abs_random_modifier, 0.0f, 100.0f);
			}

			return true;
		}
	};

	IPanel* Panel(Module::DrawType dt)
	{
		if (dt == Module::DrawType::MainWindow)
		{
			return new AIPanel();
		}

		return nullptr;
	}

	void Init()
	{
		
	}

	void End()
	{
		
	}
}

MODULE(ai);