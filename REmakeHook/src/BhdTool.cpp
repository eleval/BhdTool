#include "pch.h"

#include "BhdTool.h"

#include "Tools/OptionsTool.h"
#include "Tools/InventoryTool.h"
#include "Tools/RoomJumpTool.h"
#include "Tools/SaveTool.h"

#include "ImGui_Impl.h"

#include "imgui/imgui.h"

#include <string>

namespace
{
	bool isOpen_ = false;
	std::string headerText_;
}

void BhdTool::Init()
{
	headerText_ = std::string("BHD Tool v") + std::string(BuildVersion) + " by Eleval (https://github.com/eleval/)";

	OptionsTool::Init();
	RoomJumpTool::Init();
	SaveTool::Init();
}

void BhdTool::Update()
{
	if (ImGui_Impl::IsInitialized())
	{
		if (isOpen_)
		{
			if (ImGui::Begin("BHD Tool", &isOpen_))
			{
				ImGui::Text(headerText_.c_str());
				OptionsTool::UpdateUI();
				RoomJumpTool::UpdateUI();
				InventoryTool::UpdateUI();
				SaveTool::UpdateUI();
			}
			ImGui::End();
		}
	}
}

void BhdTool::Toggle()
{
	isOpen_ = !isOpen_;
}