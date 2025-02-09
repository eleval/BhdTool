#include "pch.h"

#include "Game/GameAddress.h"
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
	infiniteAmmoPatch_.AddNops(GameAddresses[GAID_CHEAT_INFINITE_AMMO], 6);
	infiniteHealthPatch_.AddNops(GameAddresses[GAID_CHEAT_INFINITE_HEALTH], 6);
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