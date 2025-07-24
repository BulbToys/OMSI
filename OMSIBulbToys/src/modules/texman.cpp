#include "../../core/bulbtoys.h"
#include "../omsi.h"
#include "../../core/bulbtoys/io.h"

namespace texman
{
	struct TexManPanel : IPanel
	{
		virtual bool Draw() override final
		{
			if (ImGui::BulbToys_Menu("Texture Manager"))
			{
				auto texman1 = OMSI->BulbToys_GetTextureManager1();
				auto texman1_count = OMSI->BulbToys_ListLength(Read<uintptr_t>(texman1 + 0x8));

				auto texman2 = OMSI->BulbToys_GetTextureManager2();
				auto texman2_count = OMSI->BulbToys_ListLength(Read<uintptr_t>(texman2 + 0x8));

				int count = texman1_count + texman2_count;

				static Game::TTextureItem* textures = nullptr;
				if (textures)
				{
					delete[] textures;
					textures = nullptr;
				}

				textures = new Game::TTextureItem[count];

				memcpy(textures, Read<Game::TTextureItem*>(texman1 + 0x8), texman1_count * sizeof(Game::TTextureItem));
				memcpy(textures + texman1_count, Read<Game::TTextureItem*>(texman2 + 0x8), texman2_count * sizeof(Game::TTextureItem));

				qsort(textures, count, sizeof(Game::TTextureItem), +[](const void* a, const void* b)
				{
					auto tti_a = (Game::TTextureItem*)a;
					auto tti_b = (Game::TTextureItem*)b;

					auto diff = tti_a->mem - tti_b->mem;

					if (diff < 0)
					{
						return 1;
					}
					else if (diff > 0)
					{
						return -1;
					}

					return 0;
				});

				auto d3d9 = OMSI->BulbToys_GetD3DDevice9();

				UINT (__stdcall* GetAvailableTextureMem)(LPVOID) = reinterpret_cast<decltype(GetAvailableTextureMem)>(Virtual<4>(reinterpret_cast<uintptr_t>(d3d9)));
				auto avail_mem = (float)(GetAvailableTextureMem(d3d9) >> 20);

				auto used_mem = 0.0f;
				for (int i = 0; i < count; i++)
				{
					if (textures[i].loaded)
					{
						used_mem += textures[i].mem;
					}
				}
				used_mem /= 1048576.0f;

				auto total_mem = used_mem + avail_mem;

				static char mem_info[64] { 0 };
				MYPRINTF(mem_info, 64, "%.0f / %.0f MB (%.0f MB free)", used_mem, total_mem, avail_mem);
				ImGui::ProgressBar(used_mem / total_mem, ImVec2(-FLT_MIN, 0), mem_info);

				static bool hide_unloaded = false;
				ImGui::Checkbox("Hide unloaded textures", &hide_unloaded);

				static char search[MAX_PATH] { 0 };
				ImGui::Text("Search:");
				ImGui::InputText("##TexManSearch", search, IM_ARRAYSIZE(search));

				char search_lower[MAX_PATH] { 0 };
				for (int i = 0; i < strlen(search); i++)
				{
					search_lower[i] = tolower(search[i]);
				}

				auto used_mem_table = 0.0f;

				if (ImGui::BeginTable("TextureTable", 5, ImGuiTableFlags_SizingFixedFit))
				{
					ImGui::TableNextRow();

					ImGui::TableSetColumnIndex(0);
					ImGui::Text("#");

					ImGui::TableSetColumnIndex(1);
					ImGui::Text("Path");

					ImGui::TableSetColumnIndex(2);
					ImGui::Text("Memory");

					ImGui::TableSetColumnIndex(3);
					ImGui::Text("Texture");

					ImGui::TableSetColumnIndex(4);
					ImGui::Text("Surfaces");

					int displayed_row = 1;
					for (int row = 0; row < count; row++)
					{
						if (hide_unloaded && !textures[row].loaded)
						{
							continue;
						}

						if (strlen(search) > 0)
						{
							if (!textures[row].path)
							{
								continue;
							}

							char path_lower[MAX_PATH]{ 0 };
							for (int i = 0; i < strlen(textures[row].path); i++)
							{
								path_lower[i] = tolower(textures[row].path[i]);
							}

							if (!strstr(path_lower, search_lower))
							{
								continue;
							}
						}

						if (textures[row].loaded)
						{
							used_mem_table += textures[row].mem;
						}

						ImVec4 color = textures[row].loaded ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1);

						ImGui::TableNextRow();

						for (int column = 0; column < 5; column++)
						{
							ImGui::TableSetColumnIndex(column);

							if (column == 0)
							{
								ImGui::TextColored(color, "%d", displayed_row++);
							}
							else if (column == 1)
							{
								auto path = textures[row].path;

								if (path)
								{
									ImGui::TextColored(color, "%s", path);
								}
								else
								{
									ImGui::TextColored(color, "(null)");
								}
							}
							else if (column == 2)
							{
								float mem = textures[row].mem;

								if (mem > 1048576.0)
								{
									ImGui::TextColored(color, "%.02f MB", mem / 1048576.0);
								}
								else
								{
									ImGui::TextColored(color, "%.02f KB", mem / 1024.0);
								}
							}
							else if (column == 3)
							{
								auto texture = textures[row].Texture_ID3DT9;

								char texture_label[9] = "(null)";

								if (texture)
								{
									MYPRINTF(texture_label, 9, "%p", texture);
								}

								if (ImGui::Button(texture_label))
								{
									CopyToClipboard(texture_label);
								}

								if (texture)
								{
									if (ImGui::IsItemHovered())
									{
										ImGui::BeginTooltip();
										ImGui::Image(texture, ImVec2(textures[row].size_x > 512 ? 512 : textures[row].size_x, textures[row].size_y > 512 ? 512 : textures[row].size_y));
										ImGui::EndTooltip();
									}

									char save_label[17] = "(save)##AAAAAAAA";
									MYPRINTF(save_label, 17, "(save)##%p", texture);

									ImGui::SameLine();
									if (ImGui::Button(save_label))
									{
										wchar_t filename[MAX_PATH]{ 0 };

										auto name = textures[row].justfilename;
										if (name)
										{
											StringToWideString(name, strlen(name), filename, MAX_PATH);
										}

										constexpr const wchar_t* const filter =
											L"Bitmap (*.bmp)\0*.bmp\0"
											"DirectDraw Surface (*.dds)\0*.dds\0"
											"Device Independent Bitmap (*.dib)\0*.dib\0"
											"High Dynamic Range (*.hdr)\0*.hdr\0"
											"JPG image (*.jpg)\0*.jpg\0"
											"Portable FloatMap (*.pfm)\0*.pfm\0"
											"Portable Network Graphics (*.png)\0*.png\0";

										OPENFILENAMEW ofn{};
										ofn.lStructSize = sizeof(OPENFILENAMEW);
										ofn.hwndOwner = IO::Get()->Window();
										ofn.lpstrFilter = filter;
										ofn.lpstrTitle = L"Save Texture To File";
										ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
										ofn.lpstrDefExt = L"dds";
										ofn.lpstrFile = filename;
										ofn.nMaxFile = MAX_PATH;

										if (GetSaveFileNameW(&ofn))
										{
											char extension[MAX_PATH] { 0 };
											WideStringToString(ofn.lpstrFile + ofn.nFileExtension, lstrlenW(ofn.lpstrFile) - ofn.nFileExtension, extension, MAX_PATH);

											DWORD format = -1;
											if (!_strcmpi(extension, "bmp"))
											{
												format = 0;
											}
											else if (!_strcmpi(extension, "dds"))
											{
												format = 4;
											}
											else if (!_strcmpi(extension, "dib"))
											{
												format = 6;
											}
											else if (!_strcmpi(extension, "hdr"))
											{
												format = 7;
											}
											else if (!_strcmpi(extension, "jpg"))
											{
												format = 1;
											}
											else if (!_strcmpi(extension, "pfm"))
											{
												format = 8;
											}
											else if (!_strcmpi(extension, "png"))
											{
												format = 3;
											}

											if (format != -1)
											{
												OMSI->BulbToys_D3DXSaveTextureToFile(ofn.lpstrFile, format, texture, nullptr);
											}
											else
											{
												Error("Invalid file format/extension '%s'.", extension);
											}
										}
									}
								}
							}
							else if (column == 4)
							{
								auto texture = textures[row].Texture_ID3DT9;

								if (texture)
								{
									// GetLevelCount
									auto levels = reinterpret_cast<DWORD(__stdcall*)(void*)>(Virtual<13>(reinterpret_cast<uintptr_t>(texture)))(texture);

									for (int i = 0; i < levels; i++)
									{
										void* surface = nullptr;

										// GetSurfaceLevel
										auto result = reinterpret_cast<HRESULT(__stdcall*)(void*, UINT, void**)>(Virtual<18>(reinterpret_cast<uintptr_t>(texture)))(texture, i, &surface);

										// D3D_OK
										if (result == 0)
										{
											char surface_label[9] = "(null)";

											if (surface)
											{
												MYPRINTF(surface_label, 9, "%p", surface);
											}

											if (ImGui::Button(surface_label))
											{
												CopyToClipboard(surface_label);
											}

											if (surface && ImGui::IsItemHovered())
											{
												struct D3DSURFACE_DESC
												{
													DWORD Format = 0;
													DWORD Type = 0;
													DWORD Usage = 0;
													DWORD Pool = 0;
													DWORD MultiSampleType = 0;
													DWORD MultiSampleQuality = 0;
													UINT Width = 0;
													UINT Height = 0;
												}
												desc;

												// GetDesc
												auto result = reinterpret_cast<HRESULT(__stdcall*)(void*, void*)>(Virtual<12>(reinterpret_cast<uintptr_t>(surface)))(surface, &desc);

												if (result == 0)
												{
													auto d3d_format = +[](DWORD Format)
													{
														return "todo";
													};

													auto d3d_type = +[](DWORD Type)
													{
														return "todo";
													};

													auto d3d_usage = +[](DWORD Usage)
													{
														return "todo";
													};

													auto d3d_pool = +[](DWORD Pool)
													{
														return "todo";
													};

													auto d3d_mst = +[](DWORD MultiSampleType)
													{
														return "todo";
													};

													ImGui::SetTooltip(
														"Format: %d (%s)\n"
														"Type: %d (%s)\n"
														"Usage: %d (%s)\n"
														"Pool: %d (%s)\n"
														"MultiSampleType: %d (%s)\n"
														"MultiSampleQuality: %d\n"
														"Width: %u\n"
														"Height: %u\n",
														desc.Format, d3d_format(desc.Format),
														desc.Type, d3d_type(desc.Type),
														desc.Usage, d3d_usage(desc.Usage),
														desc.Pool, d3d_pool(desc.Pool),
														desc.MultiSampleType, d3d_mst(desc.MultiSampleType),
														desc.MultiSampleQuality,
														desc.Width,
														desc.Height
													);
												}
												else
												{
													ImGui::SetTooltip("D3DERR_INVALIDCALL");
												}
											}
										}
										else
										{
											ImGui::Button("D3DERR_INVALIDCALL");
										}

										if (i < levels - 1)
										{
											ImGui::SameLine();
										}

										// Release
										reinterpret_cast<ULONG(__stdcall*)(void*)>(Virtual<2>(reinterpret_cast<uintptr_t>(surface)))(surface);
									}
								}
							}
						}
					}

					ImGui::EndTable();
				}

				float table_used_ratio = ((used_mem_table / 1048576.0) / used_mem) * 100.0f;
				float table_total_ratio = ((used_mem_table / 1048576.0) / total_mem) * 100.0f;
				if (used_mem_table > 1048576.0)
				{
					ImGui::Text("Memory Usage (from table): %.02f MB (%.02f%% of used, %.02f%% of total)", used_mem_table / 1048576.0, table_used_ratio, table_total_ratio);
				}
				else
				{
					ImGui::Text("Memory Usage (from table): %.02f KB (%.02f%% of used, %.02f%% of total)", used_mem_table / 1024.0, table_used_ratio, table_total_ratio);
				}
			}

			return true;
		}
	};


	IPanel* Panel(Module::DrawType dt)
	{
		if (dt == Module::DrawType::MainWindow)
		{
			return new TexManPanel();
		}

		return nullptr;
	}
}

MODULE_PANEL_ONLY(texman);