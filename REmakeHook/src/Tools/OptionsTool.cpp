#include "pch.h"

#include "Tools/OptionsTool.h"

#include "Settings.h"
#include "Utils/CodePatch.h"

#include "imgui/imgui.h"

Setting<bool> s_enabledDoorSkip("EnableDoorSkip", false);

namespace
{
	CodePatch doorSkipPatch_;
}

void OptionsTool::Init()
{
	doorSkipPatch_.AddCode(0x0041CDD6, { 0xEB, 0x0C, 0x90 }); // Removes the entire door animation
	doorSkipPatch_.AddNops(0x0042B0BF, 6); // Remove door being present for 5 frames
	doorSkipPatch_.AddNops(0x0042AEC5, 6); // Remove beginning of animation
	doorSkipPatch_.AddNops(0x0041CDE6, 5); // Remove sounds
}

void OptionsTool::UpdateUI()
{
	if (ImGui::CollapsingHeader("Options"))
	{
		bool doorSkipEnabled = s_enabledDoorSkip.Get();
		if (ImGui::Checkbox("Enable Door Skip", &doorSkipEnabled))
		{
			s_enabledDoorSkip.Set(doorSkipEnabled);
			if (doorSkipEnabled)
			{
				doorSkipPatch_.Apply();
			}
			else
			{
				doorSkipPatch_.Remove();
			}
		}
	}
}