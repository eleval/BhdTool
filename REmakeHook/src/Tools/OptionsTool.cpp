#include "pch.h"

#include "Tools/OptionsTool.h"

#include "Utils/CodePatch.h"

#include "imgui/imgui.h"

namespace
{
	CodePatch doorSkipPatch_;
	bool doorSkipEnabled_ = false;
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
		if (ImGui::Checkbox("Enable Door Skip", &doorSkipEnabled_))
		{
			if (doorSkipEnabled_)
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