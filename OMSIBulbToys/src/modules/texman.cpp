#include "../../core/bulbtoys.h"
#include "../omsi.h"

namespace texman
{
	struct TexManPanel : IPanel
	{
		virtual bool Draw() override final
		{
			if (ImGui::BulbToys_Menu("Texture Manager"))
			{
				auto texman1 = OMSI->BulbToys_GetTextureManager1();
				auto texman2 = OMSI->BulbToys_GetTextureManager2();

				ImGui::BulbToys_AddyLabel(texman1, "Texture Manager 1");
				ImGui::BulbToys_AddyLabel(texman2, "Texture Manager 2");

				static int count = 0;
				ImGui::BulbToys_InputInt("Skip count", "##REMOVEME", &count, 0);
				int skip_count = count;

				void* texture = nullptr;
				ImVec2 size;
				for (int i = 0; i < OMSI->BulbToys_ListLength(Read<uintptr_t>(texman1 + 0x8)); i++)
				{
					auto texitem = Read<Game::TTextureItem*>(texman1 + 0x8) + i;

					if (texitem->Texture_ID3DT9)
					{
						if (skip_count)
						{
							skip_count--;
						}
						else
						{
							texture = texitem->Texture_ID3DT9;
							size = ImVec2(texitem->size_x > 512 ? 512 : texitem->size_x, texitem->size_y > 512 ? 512 : texitem->size_y);
							break;
						}
					}
				}

				ImGui::BulbToys_AddyLabel(reinterpret_cast<uintptr_t>(texture), "ID3DT9");

				ImGui::Text("meow :3");
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();

					if (texture)
					{
						ImGui::Image(texture, size);
					}
					else
					{
						ImGui::Button("No texture found");
					}

					ImGui::EndTooltip();
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