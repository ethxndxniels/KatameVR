#include "ImguiHandler.h"

//#include "../../vendor/imgui/imgui.h"
#include "../../vendor/imgui/imgui.h"
#include <filesystem>

namespace Katame
{
	ImguiHandler::ImguiHandler()
	{
		namespace fs = std::filesystem;
		if (!fs::exists("imgui.ini") && fs::exists("imgui_default.ini"))
		{
			fs::copy_file("imgui_default.ini", "imgui.ini");
		}

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();
	}

	ImguiHandler::~ImguiHandler()
	{
		ImGui::DestroyContext();
	}
}