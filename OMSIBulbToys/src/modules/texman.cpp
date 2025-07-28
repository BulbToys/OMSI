#include "../../core/bulbtoys.h"
#include "../omsi.h"
#include "../../core/bulbtoys/io.h"

namespace texman
{
	int force_mip_level = -1;

	int unload_frames = 600;
	constexpr int unload_frames_min = 15;
	constexpr int unload_frames_max = 30000;

	int tex_max_size = 256;
	constexpr int tex_max_size_min = 1;
	constexpr int tex_max_size_max = 16384;

	HRESULT __cdecl d3dx9_D3DXCreateTextureFromFileExA(
		void* pDevice,
		const char* pSrcFile,
		uint32_t Width,
		uint32_t Height,
		uint32_t MipLevels,
		uint32_t Usage,
		uint32_t Format,
		uint32_t Pool,
		uint32_t Filter,
		uint32_t MipFilter,
		uint32_t ColorKey,
		void* pSrcInfo,
		void* pPalette,
		void* ppTexture)
	{
		// force_mip is implied
		if (MipLevels == 0xFFFFFFFF)
		{
			MipLevels = force_mip_level;
		}

		return OMSI->D3DXCreateTextureFromFileExA(pDevice, pSrcFile, Width, Height, MipLevels, Usage, Format, Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, ppTexture);
	}

	struct TexManPanel : IPanel
	{
		virtual bool Draw() override final
		{
			if (ImGui::BulbToys_Menu("Texture Manager"))
			{
				ImGui::Text("Frames until unseen textures unload (default 600):");
				if (ImGui::SliderInt("##UnloadFrames", &unload_frames, unload_frames_min, unload_frames_max, "%d", ImGuiSliderFlags_AlwaysClamp))
				{
					OMSI->BulbToys_SetTextureUnloadFrames(unload_frames);
				}

				ImGui::Text("Limit all textures to X*X instead of 256*256:");
				if (ImGui::InputInt("##TexMax", &tex_max_size))
				{
					if (tex_max_size < tex_max_size_min)
					{
						tex_max_size = tex_max_size_min;
					}
					else if (tex_max_size > tex_max_size_max)
					{
						tex_max_size = tex_max_size_max;
					}

					OMSI->BulbToys_SetTextureMaxSize(tex_max_size);
				}

				/*
				static bool force_mip = false;
				if (ImGui::Checkbox("Force MipLevels", &force_mip))
				{
					OMSI->BulbToys_ForceMipLevelsPatch(!force_mip, d3dx9_D3DXCreateTextureFromFileExA);
				}

				ImGui::InputInt("##ForceMipLevels", &force_mip_level);
				*/

				ImGui::Separator();

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

				ImGui::Text("Texture memory usage:");
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

				constexpr auto column_count = 6;
				if (ImGui::BeginTable("TextureTable", column_count, ImGuiTableFlags_SizingFixedFit))
				{
					ImGui::TableNextRow();

					ImGui::TableSetColumnIndex(0);
					ImGui::Text("#");

					ImGui::TableSetColumnIndex(1);
					ImGui::Text("Path");

					ImGui::TableSetColumnIndex(2);
					ImGui::Text("Size");

					ImGui::TableSetColumnIndex(3);
					ImGui::Text("Memory");

					ImGui::TableSetColumnIndex(4);
					ImGui::Text("Texture");

					ImGui::TableSetColumnIndex(5);
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

						for (int column = 0; column < column_count; column++)
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

								if (ImGui::IsItemHovered())
								{
									ImGui::SetTooltip(
									"uint16_t size_x = %hu;\n"
									"uint16_t size_y = %hu;\n"
									"double mem = %lf;\n"
									"int datasize = %d;\n"
									"bool dataready = %s;\n"
									"void* Texture_ID3DT9 = %p;\n"
									"void* oldTexture_ID3DT9 = %p;\n"
									"char* path = %s;\n"
									"char* justfilename = %s;\n"
									"char* loadpath = %s;\n"
									"char loaded = %d;\n"
									"char load_request = %d;\n"
									"bool managed = %s;\n"
									"unsigned int failed = %u;\n"
									"uint16_t used = %hu;\n"
									"uint16_t used_highres = %hu;\n"
									"bool threadloading = %s;\n"
									"bool hasspecials = %s;\n"
									"bool no_unload = %s;\n"
									"bool onlyalpha = %s;\n"
									"int NightMap = %d;\n"
									"int WinterSnowMap = %d;\n"
									"int WinterSnowfallMap = %d;\n"
									"int FallMap = %d;\n"
									"int SpringMap = %d;\n"
									"int WinterMap = %d;\n"
									"int SummerDryMap = %d;\n"
									"int SurfMap = %d;\n"
									"bool moisture = %s;\n"
									"bool puddles = %s;\n"
									"bool moisture_ic = %s;\n"
									"bool puddles_ic = %s;\n"
									"char surface = %d;\n"
									"char surface_ic = %d;\n"
									"bool terrainmapping = %s;\n"
									"bool terrainmapping_alpha = %s;\n",
									 textures[row].size_x,
									 textures[row].size_y,
									 textures[row].mem,
									 textures[row].datasize,
									 textures[row].dataready ? "true" : "false",
									 textures[row].Texture_ID3DT9,
									 textures[row].oldTexture_ID3DT9,
									 textures[row].path ? textures[row].path : "(null)",
									 textures[row].justfilename ? textures[row].justfilename : "(null)",
									 textures[row].loadpath ? textures[row].loadpath : "(null)",
									 textures[row].loaded,
									 textures[row].load_request,
									 textures[row].managed ? "true" : "false",
									 textures[row].failed,
									 textures[row].used,
									 textures[row].used_highres,
									 textures[row].threadloading ? "true" : "false",
									 textures[row].hasspecials ? "true" : "false",
									 textures[row].no_unload ? "true" : "false",
									 textures[row].onlyalpha ? "true" : "false",
									 textures[row].NightMap,
									 textures[row].WinterSnowMap,
									 textures[row].WinterSnowfallMap,
									 textures[row].FallMap,
									 textures[row].SpringMap,
									 textures[row].WinterMap,
									 textures[row].SummerDryMap,
									 textures[row].SurfMap,
									 textures[row].moisture ? "true" : "false",
									 textures[row].puddles ? "true" : "false",
									 textures[row].moisture_ic ? "true" : "false",
									 textures[row].puddles_ic ? "true" : "false",
									 textures[row].surface,
									 textures[row].surface_ic,
									 textures[row].terrainmapping ? "true" : "false",
									 textures[row].terrainmapping_alpha ? "true" : "false"
									);
								}
							}
							else if (column == 2)
							{
								ImGui::TextColored(color, "%hux%hu", textures[row].size_x, textures[row].size_y);
							}
							else if (column == 3)
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
							else if (column == 4)
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
												OMSI->D3DXSaveTextureToFile(ofn.lpstrFile, format, texture, nullptr);
											}
											else
											{
												Error("Invalid file format/extension '%s'.", extension);
											}
										}
									}
								}
							}
							else if (column == 5)
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

	void Init()
	{
		Settings::UInt32<"TextureManager", "UnloadFrames", 600> unload_frames_setting;
		auto value = unload_frames_setting.Get();
		if (value != unload_frames && value >= 15 && value <= 30000)
		{
			if (value < unload_frames_min)
			{
				value = unload_frames_min;
			}
			else if (value > unload_frames_max)
			{
				value = unload_frames_max;
			}

			unload_frames = value;
			OMSI->BulbToys_SetTextureUnloadFrames(unload_frames);
		}

		// NOTE: won't take effect on map start because it's too late, will have to reset the device
		Settings::UInt32<"TextureManager", "TexMaxSize", 256> tex_max_size_setting;
		value = tex_max_size_setting.Get();
		if (value != tex_max_size)
		{
			if (value < tex_max_size_min)
			{
				value = tex_max_size_min;
			}
			else if (value > tex_max_size_max)
			{
				value = tex_max_size_max;
			}

			tex_max_size = value;
			OMSI->BulbToys_SetTextureMaxSize(tex_max_size);
		}
	}

	void End()
	{

	}
}

MODULE(texman);