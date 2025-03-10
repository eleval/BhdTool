#include "pch.h"

#include "Game/GameAddress.h"
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
	doorSkipPatch_.AddCode(GameAddresses[GAID_DOOR_SKIP_ANIM], { 0xEB, 0x0C, 0x90 }); // Removes the entire door animation
	doorSkipPatch_.AddNops(GameAddresses[GAID_DOOR_SKIP_RENDER], 6); // Remove door being present for 5 frames
	doorSkipPatch_.AddNops(GameAddresses[GAID_DOOR_SKIP_ANIM_START], 6); // Remove beginning of animation
	doorSkipPatch_.AddNops(GameAddresses[GAID_DOOR_SKIP_SOUNDS], 5); // Remove sounds
	doorSkipPatch_.AddCode(GameAddresses[GAID_DOOR_SKIP_LAB_ELEVATOR_FIX], { 0xFA }); // This changes the lab's elevator sound to be a non-looping one. Credits to FluffyQuack's DS mod for this change.

	if (s_enabledDoorSkip.Get())
	{
		doorSkipPatch_.Apply();
	}
}

void OptionsTool::UpdateUI()
{
	if (ImGui::CollapsingHeader("Options"))
	{
		ImGui::Indent();
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
		ImGui::Unindent();
	}
}