#include "../../core/bulbtoys.h"
#include "../omsi.h"

namespace sixth_sense
{
	char inline_separator[4] = "|";
	int frequency_ms = 100;

	namespace world
	{
		bool date_time = true;
		bool weather = true;

		bool colors = true;
	}

	namespace vehicle
	{
		bool basic_info = true;
		bool pedals = true;
		bool pedals_progress_bars = true;

		bool pedals_mouse = true;
		int pedals_mouse_y_offset = -30;

		bool weather = true;
		bool passengers = true;

		bool colors = true;
	}

	namespace driver
	{
		bool timetable = true;
		bool bus_stops = true;
		bool comfort = true;
		bool rating = true;

		bool colors = true;
	}

	class SixthSensePanel : public IPanel
	{
		virtual bool Draw() override final
		{
			if (ImGui::BulbToys_Menu("Sixth Sense"))
			{
				ImGui::Text("Inline separator:");
				ImGui::InputText("##InlineSeparator", inline_separator, IM_ARRAYSIZE(inline_separator));

				/*
				ImGui::Text("Update frequency [ms]:");
				if (ImGui::InputInt("##UpdateFrequencyMS", &frequency_ms))
				{
					if (frequency_ms < 0) frequency_ms = 0;
				}
				*/

				ImGui::Separator();

				ImGui::Text("World info: (");
				ImGui::SameLine();
				ImGui::Checkbox("##WorldColors", &world::colors);
				ImGui::SameLine();
				ImGui::Text("colors )");

				ImGui::Checkbox("Date/Time", &world::date_time);
				ImGui::Checkbox("Weather##World", &world::weather);

				ImGui::Separator();

				ImGui::Text("Vehicle info: (");
				ImGui::SameLine();
				ImGui::Checkbox("##VehicleColors", &vehicle::colors);
				ImGui::SameLine();
				ImGui::Text("colors )");

				ImGui::Checkbox("Basic info", &vehicle::basic_info);

				ImGui::Checkbox("Pedals (", &vehicle::pedals);
				ImGui::SameLine();
				ImGui::Checkbox("##PedalsProgressBars", &vehicle::pedals_progress_bars);
				ImGui::SameLine();
				ImGui::Text("progress bars )");

				ImGui::Checkbox("Mouse control pedals", &vehicle::pedals_mouse);
				ImGui::Text("Height (Y) offset:");
				ImGui::InputInt("##MousePedalsYOffset", &vehicle::pedals_mouse_y_offset);

				ImGui::Checkbox("Weather##Vehicle", &vehicle::weather);
				ImGui::Checkbox("Passengers", &vehicle::passengers);

				ImGui::Separator();

				ImGui::Text("Driver info: (");
				ImGui::SameLine();
				ImGui::Checkbox("##DriverColors", &driver::colors);
				ImGui::SameLine();
				ImGui::Text("colors )");

				ImGui::Checkbox("Timetable", &driver::timetable);
				ImGui::Checkbox("Bus stops", &driver::bus_stops);
				ImGui::Checkbox("Comfort", &driver::comfort);
				ImGui::Checkbox("Rating", &driver::rating);
			}

			return true;
		}
	};

	class SixthSenseOverlay : public IPanel
	{
		virtual bool Draw() override final
		{
			/*
			static Stopwatch timer;
			if (!timer.Running())
			{
				timer.Start();
			}

			if (timer.Elapsed() > frequency_ms * 1000)
			{
				timer.Reset();
				timer.Start();
			}
			else
			{
				return true;
			}
			*/

			auto vehicle = OMSI->BulbToys_GetMyVehicle();
			if (OMSI->BulbToys_IsInMouseControl() && vehicle && vehicle::pedals_mouse)
			{
				auto gas = Read<float>(vehicle + 0x5DC);
				auto brakes = Read<float>(vehicle + 0x5E0);
				auto io = ImGui::GetIO();

				auto screen_pos = ImGui::GetCursorScreenPos();
				ImGui::SetCursorScreenPos({ io.MousePos.x, io.MousePos.y + vehicle::pedals_mouse_y_offset });

				if (ImGui::BulbToys_Overlay_BeginTable("MouseCtrlPedals"))
				{
					ImGui::Text("%3.0f%%", (gas - brakes) * 100.0f);

					ImGui::BulbToys_Overlay_EndTable();
				}

				ImGui::SetCursorScreenPos(screen_pos);
			}

			auto weather = OMSI->BulbToys_GetWeather();
			if (world::date_time || world::weather)
			{
				if (ImGui::BulbToys_Overlay_BeginTable("WorldInfo"))
				{
					if (world::date_time)
					{
						char* day_of_week = nullptr;
						uint8_t day = -1;
						uint8_t month = -1;
						uint16_t year = -1;
						OMSI->BulbToys_GetWorldDate(day_of_week, day, month, year);

						uint8_t hours = -1;
						uint8_t minutes = -1;
						float seconds = -1;
						OMSI->BulbToys_GetWorldTime(hours, minutes, seconds);

						auto month_name = +[](int month)
						{
							switch (month)
							{
								case  1: return "Jan";
								case  2: return "Feb";
								case  3: return "Mar";
								case  4: return "Apr";
								case  5: return "May";
								case  6: return "Jun";
								case  7: return "Jul";
								case  8: return "Aug";
								case  9: return "Sep";
								case 10: return "Oct";
								case 11: return "Nov";
								case 12: return "Dec";
								default: return "???";
							}
						};

						bool paused = OMSI->BulbToys_IsSimPaused();
						if (world::colors)
						{
							ImGui::Text("%s, %02d-%s-%02d %s %02d:%02d:%02d", day_of_week ? day_of_week : "???", day, month_name(month), year % 100, inline_separator, hours, minutes, (int)seconds);
							if (paused)
							{
								ImGui::SameLine();
								ImGui::TextColored({ 1, 1, 0, 1 }, "[Paused!]");
							}
						}
						else
						{
							ImGui::Text("%s, %02d-%s-%02d %s %02d:%02d:%02d%s", day_of_week ? day_of_week : "???", day, month_name(month), year % 100, inline_separator, hours, minutes, (int)seconds, paused ? " [Paused!]" : "");
						}
					}

					if (world::weather)
					{
						auto temp = Read<float>(weather + 0x20);
						auto humidity = Read<float>(weather + 0x164) * 100.0f;
						auto brightness = Read<float>(weather + 0xB0) * 100.0f;

						ImGui::Text("Out: %.0f C, %.0f%% %s Light: %.0f%%", temp, humidity, inline_separator, brightness);
					}

					ImGui::BulbToys_Overlay_EndTable();
				}
			}

			if (vehicle && (vehicle::basic_info || vehicle::pedals || vehicle::weather || vehicle::passengers))
			{
				if (ImGui::BulbToys_Overlay_BeginTable("VehicleInfo"))
				{
					if (vehicle::basic_info)
					{
						auto ai = Read<bool>(vehicle + 0x624);
						auto speed = Read<float>(vehicle + 0x1D4) * 3.6f;
						auto fuel = Read<float>(vehicle + 0x7CC) * 100.0f;
						auto mileage = Read<double>(vehicle + 0x430);
						auto skidding = !(Read<bool>(vehicle + 0x1D8));

						if (vehicle::colors)
						{
							if (ai)
							{
								ImGui::TextColored({ 1, 1, 0, 1 }, "AI");
								ImGui::SameLine();
								ImGui::Text("%s", inline_separator);
								ImGui::SameLine();
							}
							ImGui::Text("%-3.0f km/h %s %.0f%% %s %.02lf km", speed, inline_separator, fuel, inline_separator, mileage);
							if (skidding)
							{
								ImGui::SameLine();
								ImGui::TextColored({ 1, 1, 0, 1 }, "[Skidding!]");
							}
						}
						else
						{
							if (ai)
							{
								ImGui::Text("AI %s", inline_separator);
								ImGui::SameLine();
							}
							ImGui::Text("%-3.0f km/h %s %.0f%% %s %.02lf km%s", speed, inline_separator, fuel, inline_separator, mileage, skidding ? " [Skidding!]" : "");
						}
					}

					if (vehicle::pedals)
					{
						auto gas = Read<float>(vehicle + 0x5DC);
						auto brakes = Read<float>(vehicle + 0x5E0);
						auto clutch = Read<float>(vehicle + 0x5E4);

						if (vehicle::pedals_progress_bars)
						{
							ImGui::Text("G:");
							ImGui::SameLine();
							ImGui::ProgressBar(gas, { 250, 0 });

							ImGui::Text("B:");
							ImGui::SameLine();
							ImGui::ProgressBar(brakes, { 250, 0 });

							ImGui::Text("C:");
							ImGui::SameLine();
							ImGui::ProgressBar(clutch, { 250, 0 });
						}
						else
						{
							ImGui::Text("G: %3.0f%% %s B: %3.0f%% %s C: %3.0f%%", gas * 100.f, inline_separator, brakes * 100.f, inline_separator, clutch * 100.f);
						}
					}

					if (vehicle::weather)
					{
						auto temp = Read<float>(vehicle + 0x5FC);
						auto humidity = Read<float>(vehicle + 0x600) * 100.f;
						auto brightness_inside = Read<float>(vehicle + 0x638) * 100.f;
						auto brightness_outside = Read<float>(weather + 0xB0);

						auto brightness = 100.f;
						if (brightness_inside < 0.5f)
						{
							if (brightness_outside < 0.2f)
							{
								brightness = 0.f;
							}
							else if (brightness_outside < 0.5f)
							{
								brightness -= ((1.0f / (brightness_outside)) - 2.0f) * (100.0f / 3.0f);
							}
						}

						auto temp_out = Read<float>(weather + 0x20);

						// 1.0-2.0 25-34

						// in > 25 (0%)
						// in > 34 (100%)
						// - in > out + 3 (0%)
						// - in > out + 7 (100%)
						// --- too hot

						// in < 25
						// - in > out/2 + 20 (0%)
						// - in > out/2 + 29 (100%)
						// --- too hot

						// in < 8 (0%)
						// in < 17 (100%)
						// - in < out + 5 (0%)
						// - in < out + 9 (100%)
						// --- too cold

						// in < out - 10 (0%)
						// in < out - 19 (100%)
						// --- too cold

						auto temp_comfort = 100.f;

						auto temp_comfort_penalty = 0.0f;
						auto temp_comfort_penalty_coeff = 1.0f;

						if (temp > 25.0f)
						{
							if (temp > 34.0f)
							{
								temp_comfort_penalty_coeff = 2.0f;
							}
							else
							{
								temp_comfort_penalty_coeff = 1.0f + (temp - 25.0f) / 9.0f;
							}

							if (temp > temp_out + 3.0f)
							{
								if (temp > temp_out + 7.0f)
								{
									temp_comfort_penalty -= 50.0f;
								}
								else
								{
									temp_comfort_penalty -= 50.0f * ((temp - temp_out - 3.0f) / 4.0f);
								}
							}
						}

						if (temp < 25.0f)
						{
							if (temp > temp_out / 2.0f + 20.0f)
							{
								if (temp > temp_out / 2.0f + 29.0f)
								{
									temp_comfort_penalty -= 100.0f;
								}
								else
								{
									temp_comfort_penalty -= 100.0f * ((temp - temp_out / 2.0f - 20.0f) / 9.0f);
								}
							}
						}

						if (temp < 17.0f)
						{
							if (temp < 8.0f)
							{
								temp_comfort_penalty_coeff = 2.0f;
							}
							else
							{
								temp_comfort_penalty_coeff = 1.0f + (temp - 17.0f) / -9.0f;
							}

							if (temp < temp_out + 9.0f)
							{
								if (temp < temp_out + 5.0f)
								{
									temp_comfort_penalty -= 50.0f;
								}
								else
								{
									temp_comfort_penalty -= 50.0f * ((temp - temp_out - 9.0f) / -4.0f);
								}
							}
						}

						if (temp < temp_out - 10.0f)
						{
							if (temp < temp_out - 19.0f)
							{
								temp_comfort_penalty -= 100.0f;
							}
							else
							{
								temp_comfort_penalty -= 100.0f * ((temp - temp_out - 10.0f) / -9.0f);
							}
						}

						temp_comfort += temp_comfort_penalty * temp_comfort_penalty_coeff;

						if (vehicle::colors)
						{
							ImVec4 temp_color = { 0, 1, 0, 1 }; // green
							if (temp_comfort < 50.f)
							{
								temp_color = { 1, 0, 0, 1 }; // red
							}
							else if (temp_comfort < 100.0f)
							{
								temp_color = { 1, 1, 0, 1 }; // yellow
							}

							ImVec4 brightness_color = { 0, 1, 0, 1 }; // green
							if (brightness < 50.f)
							{
								brightness_color = { 1, 0, 0, 1 }; // red
							}
							else if (brightness < 100.0f)
							{
								brightness_color = { 1, 1, 0, 1 }; // yellow
							}

							ImGui::TextColored(temp_color, "In: %.0f C, %.0f%% (Comfort: %.0f%%)", temp, humidity, temp_comfort);
							ImGui::SameLine();
							ImGui::Text("%s", inline_separator);
							ImGui::SameLine();
							ImGui::TextColored(brightness_color, "Light: %.0f%%", brightness);
						}
						else
						{
							ImGui::Text("In: %.0f C, %.0f%% (Comfort: %.0f%%) %s Light: %.0f%%", temp, humidity, temp_comfort, inline_separator, brightness);
						}
					}

					if (vehicle::passengers)
					{
						auto seat_array = Read<bool*>(vehicle + 0x6D0);
						int count = OMSI->BulbToys_ListLength(reinterpret_cast<uintptr_t>(seat_array));

						int taken = 0;
						for (int i = 0; i < count; i++)
						{
							if (seat_array[i]) taken++;
						}

						auto first_mmo = Read<uintptr_t>(vehicle + 0x4D0);
						if (Read<bool>(first_mmo + 0x4E8))
						{
							if (vehicle::colors)
							{
								ImGui::Text("Passengers: %d / %d (%d)", taken, count, count - taken);
								ImGui::SameLine();
								ImGui::TextColored({ 1, 1, 0, 1 }, "[Stop!]");
							}
							else
							{
								ImGui::Text("Passengers: %d / %d (%d) [Stop]", taken, count, count - taken);
							}
							Write<bool>(first_mmo + 0x4E8, false); // game apparently does this too lol
						}
						else
						{
							ImGui::Text("Passengers: %d / %d (%d)", taken, count, count - taken);
						}
						
					}

					ImGui::BulbToys_Overlay_EndTable();
				}
			}

			auto driver = OMSI->BulbToys_GetCurrentDriver();
			if ((vehicle && driver::timetable) || (driver && (driver::bus_stops || driver::comfort || driver::rating)))
			{
				if (ImGui::BulbToys_Overlay_BeginTable("DriverInfo"))
				{
					if (vehicle && driver::timetable)
					{
						auto terminus = Read<char*>(vehicle + 0x7BC);
						auto allexit = !strncmp(terminus, "$allexit$", 10);
						if (!terminus || allexit)
						{
							auto myhof = Read<int>(vehicle + 0x7C8);
							auto trv = Read<uintptr_t>(vehicle + 0x710);
							if (trv)
							{
								auto hoefe = Read<uintptr_t>(trv + 0x5E4);
								if (hoefe && OMSI->BulbToys_BoundCheck(hoefe, myhof))
								{
									auto hof = Read<uintptr_t>(hoefe + myhof * 4);
									if (hof)
									{
										auto targets = Read<uintptr_t>(hof + 0x14);
										auto target_index = Read<int>(vehicle + 0x7B8);
										if (targets && OMSI->BulbToys_BoundCheck(targets, target_index))
										{
											terminus = Read<char*>(targets + 0x18 * target_index + 8);
										}
									}
								}
							}
						}

						// is timetable valid?
						if (Read<bool>(vehicle + 0x65C))
						{
							auto line = OMSI->BulbToys_GetLineName(Read<int>(vehicle + 0x660));
							
							auto delay = Read<int>(vehicle + 0x6BC);
							char delay_str[16] { 0 };
							if (delay < 0)
							{
								MYPRINTF(delay_str, IM_ARRAYSIZE(delay_str), "-%01d:%02d", -delay / 60, -delay % 60);
							}
							else
							{
								MYPRINTF(delay_str, IM_ARRAYSIZE(delay_str), "+%01d:%02d", delay / 60, delay % 60);
							}

							auto next = Read<int>(vehicle + 0x680);
							wchar_t* next_name = nullptr;
							int stop_count = -1;
							OMSI->BulbToys_GetTripInfo(Read<int>(vehicle + 0x66C), next, &next_name, &stop_count);
							
							char next_stop[64] { 0 };
							if (next_name)
							{
								WideStringToString(next_name, 64, next_stop, 64);
							}

							auto next_time = (int)Read<float>(vehicle + 0x698);
							if (next_time > 86400)
							{
								next_time -= 86400;
							}
							int next_h = next_time / 3600;
							int next_m = (next_time % 3600) / 60;
							int next_s = next_time % 60;

							float distance = Read<float>(vehicle + 0x688);

							if (driver::colors)
							{
								ImVec4 delay_color = { 0, 1, 0, 1 }; // green
								if (delay > 120)
								{
									if (delay > 180)
									{
										delay_color = { 1, 0, 0, 1 }; // red

										// note: passengers complain at 5+ min delay
									}
									else
									{
										delay_color = { 1, 1, 0, 1 }; // yellow
									}
								}
								if (delay < -60)
								{
									if (delay < -120)
									{
										delay_color = { 0, 0.5, 1, 1 }; // bright blue
									}
									else
									{
										delay_color = { 0, 1, 0.75, 1 }; // turquoise
									}
								}

								ImGui::Text("%s => %s", line ? line : "???", terminus ? terminus : "???");
								if (allexit)
								{
									ImGui::SameLine();
									ImGui::TextColored({ 1, 1, 0, 1 }, "(allexit)");
								}
								ImGui::SameLine();
								ImGui::Text("%s [%d/%d] %s @ %02d:%02d:%02d", inline_separator, next + 1, stop_count + 1, next_stop, next_h, next_m, next_s);

								ImGui::SameLine();
								ImGui::TextColored(delay_color, "(%s)", delay_str);
								if (distance < 999'999.0f)
								{
									ImGui::SameLine();
									ImGui::Text("(%.0fm)", distance);
								}
							}
							else
							{
								ImGui::Text("%s => %s %s%s [%d/%d] %s @ %02d:%02d:%02d (%s)", 
									line ? line : "???",
									terminus ? terminus : "???",
									allexit ? " (allexit)" : "",
									inline_separator,
									next + 1,
									stop_count + 1,
									next_stop,
									next_h,
									next_m,
									next_s,
									delay_str);

								if (distance < 999'999.0f)
								{
									ImGui::SameLine();
									ImGui::Text("(%.0fm)", distance);
								}
							}
						}
						else
						{
							ImGui::Text("[No Timetable] => %s", terminus ? terminus : "???");
							if (allexit)
							{
								ImGui::SameLine();
								if (driver::colors)
								{
									ImGui::TextColored({ 1, 1, 0, 1 }, "(allexit)");
								}
								else
								{
									ImGui::Text("(allexit)");
								}
							}
						}
					}

					if (driver)
					{
						if (driver::bus_stops)
						{
							auto total = Read<int>(driver + 0x20);
							auto late = Read<int>(driver + 0x24);
							auto early = Read<int>(driver + 0x28);
							auto on_time = total - late - early;
							float perc = ((float)on_time / (float)total) * 100.0f;

							ImGui::Text("Bus stops: %.02f%% (%d / %d (%d)) %s L: %d %s E: %d", perc, on_time, total, late + early, inline_separator, late, inline_separator, early);
						}

						if (driver::comfort)
						{
							auto total = Read<int>(driver + 0x54);
							auto happy = Read<int>(driver + 0x48);
							float perc = ((float)happy / (float)total) * 100.0f;

							ImGui::Text("Comfort: %.02f%% (%d / %d (%d))", perc, happy, total, total - happy);
						}

						if (driver::rating)
						{
							auto rating = Read<double>(driver + 0x40);
							int grade = round(rating * 10.0L);
							auto grade_name = +[](int grade)
							{
								switch (grade)
								{
									case  0: return "excellent";
									case  1: return "good";
									case  2: return "ok";
									case  3: return "mediocre";
									case  4: return "abrasive";
									case  5: return "not nice";
									case  6: return "adventurous";
									case  7: return "bad";
									case  8: return "very bad";
									case  9: return "cruel";
									case 10: return "perilous";
									default: return "???";
								}
							};

							if (driver::colors)
							{
								ImVec4 rating_color = { 0, 1, 0, 1 }; // green
								if (grade > 6)
								{
									rating_color = { 1, 0, 0, 1 }; // red
								}
								else if (grade > 2)
								{
									rating_color = { 1, 1, 0, 1 }; // yellow
								}

								ImGui::TextColored(rating_color, "Rating: %.02f%% (%s)", (1.0L - rating) * 100.0L, grade_name(grade));
							}
							else
							{
								ImGui::Text("Rating: %.02f%% (%s)", (1.0L - rating) * 100.0L, grade_name(grade));
							}
						}
					}

					ImGui::BulbToys_Overlay_EndTable();
				}
			}

			return true;
		}
	};

	IPanel* Panel(Module::DrawType dt)
	{
		if (dt == Module::DrawType::MainWindow)
		{
			return new SixthSensePanel();
		}

		if (dt == Module::DrawType::Overlay)
		{
			return new SixthSenseOverlay();
		}

		return nullptr;
	}

	void Init()
	{
		Settings::String<"SixthSense", "InlineSeparator", "|", 4> inline_separator_setting;
		strncpy_s(inline_separator, 4, inline_separator_setting.Get(), 3);

		Settings::Bool<"SixthSense", "World_Colors", true> world_colors_setting;
		world::colors = world_colors_setting.Get();

		Settings::Bool<"SixthSense", "World_DateTime", true> world_date_time_setting;
		world::date_time = world_date_time_setting.Get();

		Settings::Bool<"SixthSense", "World_Weather", true> world_weather_setting;
		world::weather = world_weather_setting.Get();

		Settings::Bool<"SixthSense", "Vehicle_Colors", true> vehicle_colors_setting;
		vehicle::colors = vehicle_colors_setting.Get();

		Settings::Bool<"SixthSense", "Vehicle_BasicInfo", true> vehicle_basic_info_setting;
		vehicle::basic_info = vehicle_basic_info_setting.Get();

		Settings::Bool<"SixthSense", "Vehicle_Pedals", true> vehicle_pedals_setting;
		vehicle::pedals = vehicle_pedals_setting.Get();

		Settings::Bool<"SixthSense", "Vehicle_Pedals_ProgressBars", true> vehicle_pedals_pb_setting;
		vehicle::pedals_progress_bars = vehicle_pedals_pb_setting.Get();

		Settings::Bool<"SixthSense", "Vehicle_Pedals_MouseControl", true> vehicle_pedals_mouse_setting;
		vehicle::pedals_mouse = vehicle_pedals_mouse_setting.Get();

		Settings::Int32<"SixthSense", "Vehicle_Pedals_MouseControl_HeightOffset", -30> vehicle_pedals_mouse_y_offset;
		vehicle::pedals_mouse_y_offset = vehicle_pedals_mouse_y_offset.Get();

		Settings::Bool<"SixthSense", "Vehicle_Weather", true> vehicle_weather_setting;
		vehicle::weather = vehicle_weather_setting.Get();
		
		Settings::Bool<"SixthSense", "Vehicle_Passengers", true> vehicle_passengers_setting;
		vehicle::passengers = vehicle_passengers_setting.Get();

		Settings::Bool<"SixthSense", "Driver_Colors", true> driver_colors_setting;
		driver::colors = driver_colors_setting.Get();

		Settings::Bool<"SixthSense", "Driver_Timetable", true> driver_timetable_setting;
		driver::timetable = driver_timetable_setting.Get();

		Settings::Bool<"SixthSense", "Driver_BusStops", true> driver_bus_stops_setting;
		driver::bus_stops = driver_bus_stops_setting.Get();

		Settings::Bool<"SixthSense", "Driver_Comfort", true> driver_comfort_setting;
		driver::comfort = driver_comfort_setting.Get();

		Settings::Bool<"SixthSense", "Driver_Rating", true> driver_rating_setting;
		driver::rating = driver_rating_setting.Get();
	}

	void End()
	{

	}
}

MODULE(sixth_sense);