#include "pch.h"

#include "BhdTool.h"

#include "Tools/DoorSkipTool.h"
#include "Tools/RoomJumpTool.h"

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
}

void BhdTool::Update()
{
	if (ImGui_Impl::IsInitialized())
	{
		if (ImGui::Begin("BHD Tool"))
		{
			DoorSkipTool::UpdateUI();
			RoomJumpTool::UpdateUI();
		}
		ImGui::End();
	}
}