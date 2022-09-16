#include "pch.h"

#include "Tools/DoorSkipTool.h"

#include "Utils/CodePatch.h"

#include "imgui/imgui.h"

namespace
{
	CodePatch doorSkipPatch_;
}

namespace
{

void Enable()
{
	doorSkipPatch_.Apply();
}

void Disable()
{
	doorSkipPatch_.Remove();
}

}

void DoorSkipTool::Init()
{
	doorSkipPatch_.AddCode(0x0041CDD6, { 0xEB, 0x0C, 0x90 }); // Removes the entire door animation
	doorSkipPatch_.AddNops(0x0042B0BF, 6); // Remove door being present for 5 frames
	doorSkipPatch_.AddNops(0x0042AEC5, 6); // Remove beginning of animation
	doorSkipPatch_.AddNops(0x0041CDE6, 5); // Remove sounds
}

void DoorSkipTool::UpdateUI()
{
	static bool v = false;
	if (ImGui::Checkbox("Enable Door Skip", &v))
	{
		if (v)
		{
			Enable();
		}
		else
		{
			Disable();
		}
	}
}