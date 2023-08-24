#include "pch.h"

#include "Tools/CheatTool.h"
#include "Utils/CodePatch.h"

#include "imgui/imgui.h"

namespace
{
	bool infiniteAmmo_ = false;
	bool infiniteHealth_ = false;

	CodePatch infiniteAmmoPatch_;
	CodePatch infiniteHealthPatch_;
}

void CheatTool::Init()
{
	infiniteAmmoPatch_.AddNops(0x0065f804, 6);
	infiniteHealthPatch_.AddNops(0x0050f4c2, 6);
}

void CheatTool::UpdateUI()
{
	if (ImGui::CollapsingHeader("Cheats"))
	{
		if (ImGui::Checkbox("Infinite Ammo", &infiniteAmmo_))
		{
			if (infiniteAmmo_)
			{
				infiniteAmmoPatch_.Apply();
			}
			else
			{
				infiniteAmmoPatch_.Remove();
			}
		}
		if (ImGui::Checkbox("Infinite Health", &infiniteHealth_))
		{
			if (infiniteHealth_)
			{
				infiniteHealthPatch_.Apply();
			}
			else
			{
				infiniteHealthPatch_.Remove();
			}
		}
	}
}