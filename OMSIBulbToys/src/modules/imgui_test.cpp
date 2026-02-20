#include "../../core/bulbtoys.h"

namespace imgui_test
{
	Stopwatch sw;

	struct ImGuiTestPanel : IPanel
	{
		float fps = 0;
		float dt = 0;
		int ms = 1000;

		virtual bool Draw() override final
		{
			if (ImGui::BulbToys_Menu("ImGui test"))
			{
				ImGui::Text("Update rate:");
				ImGui::InputInt("##ImGuiTestUpdateRate", &ms);

				if (sw.Elapsed() / 1000 > ms)
				{
					auto& imgui_io = ImGui::GetIO();
					fps = imgui_io.Framerate;
					dt = imgui_io.DeltaTime;
					sw.Reset();
					sw.Start();
				}

				ImGui::Text(" FPS: %.0f", fps);
				ImGui::Text("  dT: %.0f ms", dt * 1000);
				ImGui::Text("1/dT: %.0f Hz", 1 / dt);
			}

			return true;
		}
	};

	IPanel* Panel(Module::DrawType dt)
	{
		if (dt == Module::DrawType::MainWindow)
		{
			return new ImGuiTestPanel();
		}

		return nullptr;
	}

	void Init()
	{
		sw.Start();
	}

	void End()
	{
		sw.Stop();
	}
}

MODULE(imgui_test);