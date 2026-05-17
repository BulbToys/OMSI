#include "../../core/bulbtoys.h"
#include "../omsi.h"

namespace timetable
{
	struct TimetablePanel : IPanel
	{
		int current_trip = 0;
		int current_profile = 0;
		int current_busstop = 0;
		int current_line = 0;
		int current_tour = 0;
		int current_tour_entry = 0;

		float indent_w = 50.0f;

		virtual bool Draw() override final
		{
			if (ImGui::BulbToys_Menu("Timetable"))
			{
				auto ttman = reinterpret_cast<Game::TTimeTableMan*>(OMSI->BulbToys_GetTimeTableManager());

				if (ttman)
				{
					ImGui::Text("invalid:");
					ImGui::SameLine();
					ImGui::TextColored((ttman->invalid ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1)), (ttman->invalid ? "True" : "False"));

					auto trips = ttman->Trips;
					if (trips)
					{
						auto trips_len = OMSI->BulbToys_ListLength(reinterpret_cast<uintptr_t>(trips));
						ImGui::Text("Trips: (%d)", trips_len);
						auto trips_names = new const char* [trips_len + 1];
						trips_names[0] = "(select)";
						for (int i = 0; i < trips_len; i++)
						{
							if (trips[i].filename)
							{
								trips_names[i + 1] = trips[i].filename;
							}
							else
							{
								trips_names[i + 1] = "(null)";
							}
						}
						if (ImGui::Combo("##TTTrips", &current_trip, trips_names, trips_len + 1))
						{
							current_profile = 0;
							current_busstop = 0;
						}
						delete[] trips_names;
					}
					else
					{
						ImGui::Text("Trips: (0)");
					}

					if (current_trip > 0)
					{
						ImGui::Indent(indent_w);
						auto trip = trips[current_trip - 1];

						ImGui::Text("chrono_origin: %d", trip.chrono_origin);
						ImGui::Text("target: %s", trip.target? trip.target : "(null)");
						ImGui::Text("linie: %s", trip.linie?trip.linie:"(null)");

						ImGui::Text("trainrev:");
						ImGui::SameLine();
						ImGui::TextColored((trip.trainrev ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1)), (trip.trainrev ? "True" : "False"));

						ImGui::Text("invalide: %u", trip.invalide);

						if (trip.profiles)
						{
							auto profiles_len = OMSI->BulbToys_ListLength(reinterpret_cast<uintptr_t>(trip.profiles));
							ImGui::Text("Profiles: (%d)", profiles_len);
							auto profiles_names = new const char* [profiles_len + 1];
							profiles_names[0] = "(select)";
							for (int i = 0; i < profiles_len; i++)
							{
								if (trip.profiles[i].name)
								{
									profiles_names[i + 1] = trip.profiles[i].name;
								}
								else
								{
									profiles_names[i + 1] = "(null)";
								}
							}
							ImGui::Combo("##TTTProfiles", &current_profile, profiles_names, profiles_len + 1);
							delete[] profiles_names;
						}
						else
						{
							ImGui::Text("Profiles: (0)");
						}

						if (current_profile > 0)
						{
							ImGui::Indent(indent_w);
							auto profile = trip.profiles[current_profile - 1];

							ImGui::Text("time_all: %.02f", profile.time_all);

							auto stop_times_len = OMSI->BulbToys_ListLength(reinterpret_cast<uintptr_t>(profile.stop_times));
							ImGui::Text("stop_times: %d", stop_times_len);

							ImGui::Indent(indent_w);
							for (int i = 0; i < stop_times_len; i++)
							{
								auto stop_time = profile.stop_times[i];

								ImGui::Text("%d. arr_time: %.02f", i + 1, stop_time.arr_time);
								ImGui::Text("%d. dep_time: %.02f", i + 1, stop_time.dep_time);

								ImGui::Text("%d. arr_time_man:", i + 1);
								ImGui::SameLine();
								ImGui::TextColored((stop_time.arr_time_man ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1)), (stop_time.arr_time_man ? "True" : "False"));

								ImGui::Text("%d. dep_time_man:", i + 1);
								ImGui::SameLine();
								ImGui::TextColored((stop_time.dep_time_man ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1)), (stop_time.dep_time_man ? "True" : "False"));

								ImGui::Text("%d. stopping: %u", i + 1, stop_time.stopping);

								ImGui::Text("=====");
							}
							ImGui::Unindent(indent_w);

							ImGui::BulbToys_AddyLabel(reinterpret_cast<uintptr_t>(profile.TrackEntryTime), "TrackEntryTime");

							ImGui::Text("servicetrip:");
							ImGui::SameLine();
							ImGui::TextColored((profile.servicetrip ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1)), (profile.servicetrip ? "True" : "False"));

							ImGui::Unindent(indent_w);
						}

						if (trip.busstops)
						{
							auto busstops_len = OMSI->BulbToys_ListLength(reinterpret_cast<uintptr_t>(trip.busstops));
							ImGui::Text("Busstops: (%d)", busstops_len);
							auto busstops_names = new char* [busstops_len + 1];
							busstops_names[0] = new char[9];
							MYPRINTF(busstops_names[0], 9, "%s", "(select)");
							for (int i = 0; i < busstops_len; i++)
							{
								if (trip.busstops[i].name)
								{
									auto len = WideStringToString(trip.busstops[i].name, -1);
									busstops_names[i + 1] = new char[len];
									WideStringToString(trip.busstops[i].name, len, busstops_names[i + 1], len);
								}
								else
								{
									busstops_names[i + 1] = new char[7];
									MYPRINTF(busstops_names[i + 1], 7, "%s", "(null)");
								}
							}
							ImGui::Combo("##TTTBusstops", &current_busstop, busstops_names, busstops_len + 1);
							for (int i = 0; i < busstops_len + 1; i++)
							{
								delete[] busstops_names[i];
							}
							delete[] busstops_names;
						}
						else
						{
							ImGui::Text("Busstops: (0)");
						}

						if (current_busstop > 0)
						{
							ImGui::Indent(indent_w);
							auto busstop = trip.busstops[current_busstop - 1];

							if (busstop.name)
							{
								auto len = WideStringToString(busstop.name, -1);
								char* name = new char[len];
								WideStringToString(busstop.name, len, name, len);
								ImGui::Text("name: %s", name);
								delete[] name;
							}
							else
							{
								ImGui::Text("name: (null)");
							}

							if (busstop.name_zusatz)
							{
								auto len = WideStringToString(busstop.name_zusatz, -1);
								char* name_zusatz = new char[len];
								WideStringToString(busstop.name_zusatz, len, name_zusatz, len);
								ImGui::Text("name_zusatz: %s", name_zusatz);
								delete[] name_zusatz;
							}
							else
							{
								ImGui::Text("name_zusats: (null)");
							}

							ImGui::Text("kachel: %d", busstop.kachel);
							ImGui::Text("IDCode_formal: %u", busstop.IDCode_formal);
							ImGui::Text("IDCode_real: %u", busstop.IDCode_real);
							ImGui::Text("index: %d", busstop.index);
							ImGui::Text("index_ownlist: %d", busstop.index_ownlist);
							ImGui::BulbToys_AddyLabel(reinterpret_cast<uintptr_t>(busstop.index_laternatives), "index_laternatives");
							ImGui::Text("Preset_Aussteiger: %.02f", busstop.Preset_Aussteiger);

							ImGui::Text("pathindex:");
							for (int i = 0; i < 8; i++)
							{
								ImGui::SameLine();
								ImGui::Text("%02X", busstop.pathindex[i]);
							}

							ImGui::Text("invalide:");
							ImGui::SameLine();
							ImGui::TextColored((busstop.invalide ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1)), (busstop.invalide ? "True" : "False"));

							ImGui::Text("x_path: %.02f", busstop.x_path);
							ImGui::Text("dist_relpath: %.02f", busstop.dist_relpath);
							ImGui::Text("dist: %.02f", busstop.dist);

							ImGui::Unindent(indent_w);
						}

						ImGui::Text("Trackname: %s", trip.Trackname?trip.Trackname:"(null)");
						ImGui::Text("Trackindex: %d", trip.Trackindex);
						ImGui::BulbToys_AddyLabel(reinterpret_cast<uintptr_t>(trip.StnLinkList), "StnLinkList");

						ImGui::Unindent(indent_w);
					}

					ImGui::BulbToys_AddyLabel(reinterpret_cast<uintptr_t>(ttman->StnLinks), "StnLinks");

					auto lines = ttman->Lines;
					if (lines)
					{
						auto lines_len = OMSI->BulbToys_ListLength(reinterpret_cast<uintptr_t>(lines));
						ImGui::Text("Lines: (%d)", lines_len);
						auto lines_names = new const char* [lines_len + 1];
						lines_names[0] = "(select)";
						for (int i = 0; i < lines_len; i++)
						{
							if (lines[i].filename)
							{
								lines_names[i + 1] = lines[i].filename;
							}
							else
							{
								lines_names[i + 1] = "(null)";
							}
						}
						if (ImGui::Combo("##TTTLine", &current_line, lines_names, lines_len + 1))
						{
							current_tour = 0;
						}
						delete[] lines_names;
					}
					else
					{
						ImGui::Text("Lines: (0)");
					}

					if (current_line > 0)
					{
						ImGui::Indent(indent_w);

						auto line = lines[current_line - 1];

						ImGui::Text("userallowed:");
						ImGui::SameLine();
						ImGui::TextColored((line.userallowed ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1)), (line.userallowed ? "True" : "False"));

						ImGui::Text("priority: %u", line.priority);

						auto tours = line.tours;
						if (tours)
						{
							auto tours_len = OMSI->BulbToys_ListLength(reinterpret_cast<uintptr_t>(tours));
							ImGui::Text("Tours: (%d)", tours_len);
							auto tours_names = new const char* [tours_len + 1];
							tours_names[0] = "(select)";
							for (int i = 0; i < tours_len; i++)
							{
								if (tours[i].name)
								{
									tours_names[i + 1] = tours[i].name;
								}
								else
								{
									tours_names[i + 1] = "(null)";
								}
							}
							if (ImGui::Combo("##TTTTour", &current_tour, tours_names, tours_len + 1))
							{
								current_tour_entry = 0;
							}
							delete[] tours_names;
						}
						else
						{
							ImGui::Text("Tours: (0)");
						}
						
						if (current_tour > 0)
						{
							ImGui::Indent(indent_w);
							auto tour = tours[current_tour - 1];

							ImGui::Text("aigroup: %s", tour.aigroup ? tour.aigroup : "(null)");
							ImGui::Text("aitype: %d", tour.aitype);
							ImGui::Text("aigroupindex: %d", tour.aigroupindex);
							ImGui::Text("vehicle_nr_reservation: %s", tour.vehicle_nr_reservation ? tour.vehicle_nr_reservation : "(null)");
							ImGui::BulbToys_AddyLabel(reinterpret_cast<uintptr_t>(tour.vehicle_indizes), "vehicle_indizes");

							ImGui::Text("hasNormalVeh:");
							ImGui::SameLine();
							ImGui::TextColored((tour.hasNormalVeh ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1)), (tour.hasNormalVeh ? "True" : "False"));

							ImGui::Text("TagErledigt: %d", tour.TagErledigt);

							ImGui::Text("validOn: %s%s%s%s%s%s%s%s%s%s",
								tour.validOn.Mon ? "Mon, " : "",
								tour.validOn.Tue ? "Tue, " : "",
								tour.validOn.Wed ? "Wed, " : "",
								tour.validOn.Thu ? "Thu, " : "",
								tour.validOn.Fri ? "Fri, " : "",
								tour.validOn.Sat ? "Sat, " : "",
								tour.validOn.Sun ? "Sun, " : "",
								tour.validOn.Hol ? "Hol, " : "",
								tour.validOn.Hols ? "Hols, " : "",
								tour.validOn.NoHols ? "NoHols, " : ""
							);


							ImGui::Text("invalide:");
							ImGui::SameLine();
							ImGui::TextColored((tour.invalide ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1)), (tour.invalide ? "True" : "False"));


							auto tour_entries = tour.entrys;
							if (tour_entries)
							{
								auto tour_entries_len = OMSI->BulbToys_ListLength(reinterpret_cast<uintptr_t>(tour_entries));
								ImGui::Text("Tour entries: (%d)", tour_entries_len);
								auto tour_entries_names = new const char* [tour_entries_len + 1];
								tour_entries_names[0] = "(select)";
								for (int i = 0; i < tour_entries_len; i++)
								{
									if (tour_entries[i].Trip)
									{
										tour_entries_names[i + 1] = tour_entries[i].Trip;
									}
									else
									{
										tour_entries_names[i + 1] = "(null)";
									}
								}
								ImGui::Combo("##TTTTourEntries", &current_tour_entry, tour_entries_names, tour_entries_len + 1);
								delete[] tour_entries_names;
							}
							else
							{
								ImGui::Text("Tour entries: (0)");
							}
							
							if (current_tour_entry > 0)
							{
								ImGui::Indent(indent_w);

								auto tour_entry = tour_entries[current_tour_entry - 1];

								ImGui::Text("TripIndex: %d", tour_entry.TripIndex);
								ImGui::Text("Profile: %d", tour_entry.Profile);
								ImGui::Text("StartTime: %.02f", tour_entry.StartTime);
								ImGui::Text("EndTime: %.02f", tour_entry.EndTime);

								ImGui::Text("SmoothTrans:");
								ImGui::SameLine();
								ImGui::TextColored((tour_entry.SmoothTrans ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1)), (tour_entry.SmoothTrans ? "True" : "False"));

								ImGui::Unindent(indent_w);
							}
							ImGui::Unindent(indent_w);
						}

						ImGui::Text("chrono_origin: %d", line.chrono_origin);

						ImGui::Unindent(indent_w);
					}

					ImGui::Text("AllStationIndicesResetted:");
					ImGui::SameLine();
					ImGui::TextColored((ttman->AllStationIndicesResetted ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1)), (ttman->AllStationIndicesResetted ? "True" : "False"));

					ImGui::BulbToys_AddyLabel(reinterpret_cast<uintptr_t>(ttman->RVFiles), "RVFiles");

					ImGui::BulbToys_AddyLabel(reinterpret_cast<uintptr_t>(ttman->noRVNumbers), "noRVNumbers");
				}
				else
				{
					ImGui::Text("No timetable manager.");
				}
			}

			return true;
		}
	};

	IPanel* Panel(Module::DrawType dt)
	{
		if (dt == Module::DrawType::MainWindow)
		{
			return new TimetablePanel();
		}

		return nullptr;
	}
}

MODULE_PANEL_ONLY(timetable);