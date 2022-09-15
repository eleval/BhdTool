#include "pch.h"

#include "BhdTool.h"

#include "Hooks/DoorSkip.h"
#include "Hooks/RoomJump.h"

#include "ImGui_Impl.h"

#include "imgui/imgui.h"

namespace
{
	bool isOpen_ = false;
}

void BhdTool::Update()
{
	if (ImGui_Impl::IsInitialized())
	{
		if (ImGui::Begin("BHD Tool"))
		{
			DoorSkip::UpdateUI();
			RoomJump::UpdateUI();
		}
		ImGui::End();
	}
}