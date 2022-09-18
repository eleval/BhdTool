#include "pch.h"

#include "BhdTool.h"

#include "Tools/DoorSkipTool.h"
#include "Tools/InventoryTool.h"
#include "Tools/RoomJumpTool.h"
#include "Tools/SaveTool.h"

#include "ImGui_Impl.h"

#include "imgui/imgui.h"

namespace
{
	bool isOpen_ = false;
}

void BhdTool::Init()
{
	DoorSkipTool::Init();
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
				DoorSkipTool::UpdateUI();
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