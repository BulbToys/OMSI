#include "../../core/bulbtoys.h"
#include "../omsi.h"

namespace ai
{
	namespace passenger
	{
		float max_chance = 80.0f;
		float min_chance = 40.0f;

		float short_trip = 5.0f;
		float short_modifier = 40.0f;

		float younger_than = 16.0f;
		float young_modifier = 10.0f;

		float older_than = 55.0f;
		float old_modifier = 20.0f;

		float abs_random_modifier = 10.0f;

		int PreferSitting(uintptr_t* eligible_seats_ptr, uintptr_t human_seat_nr_bus)
		{
			auto human = human_seat_nr_bus - 0x610;

			auto len = OMSI->BulbToys_ListLength(*eligible_seats_ptr);
			auto eligible_seats = reinterpret_cast<Game::Tfreeseats*>(*eligible_seats_ptr);

			std::vector<Game::Tfreeseats> new_eligible_seats(&eligible_seats[0], &eligible_seats[len - 1]);

			// sort by distance to entrance todo?

			int free_seats = 0;
			int total_seats = 0;
			std::vector<uintptr_t> checked_vehicles;

			for (const auto& it : new_eligible_seats)
			{
				auto vehicle = it.vehicle;
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
				if (checked_vehicles.end() != std::find(checked_vehicles.begin(), checked_vehicles.end(), vehicle))
				{
					checked_vehicles.push_back(vehicle);

					for (int i = 0; i < OMSI->BulbToys_ListLength(seats); i++)
					{
						auto seat = reinterpret_cast<Game::TSeat*>(seats + i * sizeof(Game::TSeat));

						if (seat->height > 0.0f)
						{
							total_seats++;
						}
					}
				}

				auto index = it.index;
				if (!OMSI->BulbToys_BoundCheck(seats, index))
				{
					continue;
				}

				auto seat = reinterpret_cast<Game::TSeat*>(seats + index * sizeof(Game::TSeat));

				if (seat->height > 0.0f)
				{
					free_seats++;
				}
			}

			if (free_seats)
			{
				float sit_chance = min_chance + ((max_chance - min_chance) * (free_seats / total_seats));

				// trip length modifier

				auto age = Read<float>(human + 0x268);
				if (age < younger_than)
				{
					sit_chance += young_modifier;
				}
				else if (age > older_than)
				{
					sit_chance += old_modifier;
				}

				sit_chance = OMSI->RandomCentered(abs_random_modifier, sit_chance);

				if (sit_chance > 100.0f)
				{
					sit_chance = 100.0f;
				}
				else if (sit_chance < 0.0f)
				{
					sit_chance = 0.0f;
				}

				// determine seat(s) based on chance
			}

			int new_len = new_eligible_seats.size();
			if (new_len > 0 && new_len < len)
			{
				std::copy(new_eligible_seats.begin(), new_eligible_seats.end(), eligible_seats);
				OMSI->DynArraySetLength(eligible_seats_ptr, OMSI->BulbToys_GetEligibleSeatsRTTIAddress(), 1, new_len);
				return new_len;
			}

			return len;
		}

		void __declspec(naked) PreferSittingHook()
		{
			__asm
			{
				push  [ebp - 8]
				push  eax
				call  PreferSitting
				add   esp, 4
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
				if (ImGui::Checkbox("Passengers prefer to sit", &prefer_sitting))
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

				ImGui::Separator();
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