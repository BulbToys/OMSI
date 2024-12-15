#include "../../core/bulbtoys.h"

namespace test
{
	struct TestPanel : IPanel
	{
		char input[1024];

		virtual bool Draw() override final
		{
			if (ImGui::BulbToys_Menu("Test Input"))
			{
				ImGui::InputText("##TestInput", input, 1024);
			}

			return true;
		}
	};


	IPanel* Panel(Module::DrawType dt)
	{
		if (dt == Module::DrawType::MainWindow)
		{
			return new TestPanel();
		}

		return nullptr;
	}
}

MODULE_PANEL_ONLY(test);