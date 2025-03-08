#include "pch.h"

#include "Game/GameAddress.h"
#include "Tools/SaveTool.h"

#include "Utils/Memory.h"
#include "Utils/TrampHook.h"

#include "imgui/imgui.h"

namespace
{
	TrampHook bhd_ExecuteTrigger_hook;
	TrampHook bhd_0041fd70_hook;
	CodePatch instantTypeWriterMenuPatch_;
	CodePatch invalidSaveFixPatch_;;

	bool openSaveMenu_ = false;
}

namespace
{

// Having this variable in static memory somehow fixes the save bug
// I'm guessing the compiler was too agressive in how it would optimize it but that's still weird
// Anyway, it's fixed, so yay?
uint8_t* pVal = nullptr;

using bhd_ExecuteTrigger_t = int(__fastcall*)(void*, void*, int, int, int);
int __fastcall hk_bhd_ExecuteTrigger(void* obj, void* edx, int param_1, int param_2, int param_3)
{
	bhd_ExecuteTrigger_t bhd_ExecuteTrigger = (bhd_ExecuteTrigger_t)bhd_ExecuteTrigger_hook.GetGateway();

	if (openSaveMenu_)
	{
		pVal = (uint8_t*)((uintptr_t)param_1 + 0x30);
		const volatile uint8_t oldVal = *pVal;
		*pVal = 6;

		instantTypeWriterMenuPatch_.Apply();
		const int ret = bhd_ExecuteTrigger(obj, edx, param_1, param_2, param_3);;
		instantTypeWriterMenuPatch_.Remove();
		Mem::WriteMemory((size_t)pVal, (void*)&oldVal, sizeof(oldVal));
		
		return ret;
	}

	return bhd_ExecuteTrigger(obj, edx, param_1, param_2, param_3);
}

int __fastcall hk_bhd_0041fd70(void* obj, void* edx, int param_1)
{
	using bhd_0041fd70_t = int(__fastcall*)(void*, void*, int);
	bhd_0041fd70_t bhd_0041fd70 = (bhd_0041fd70_t)bhd_0041fd70_hook.GetGateway();

	if (openSaveMenu_)
	{
		volatile int uVar6 = 0;
		volatile int iVar3 = *(int*)(*(int*)((uintptr_t)obj + 0xe0) + uVar6 * 4);

		bhd_ExecuteTrigger_t bhd_ExecuteTrigger = (bhd_ExecuteTrigger_t)GameAddresses[GAID_EXECUTE_TRIGGER];
		bhd_ExecuteTrigger(obj, edx, iVar3, uVar6, 0);

		openSaveMenu_ = false;

		return 1;
	}

	return bhd_0041fd70(obj, edx, param_1);
}

}

void SaveTool::Init()
{
	// Adding nops to skip a call showing the text but not skipping a push before it
	// This somehow shows the save menu without corrupting the stack?
	// Anyway, it works, so GG? xD
	const size_t instantTypeWriterMenuPatchAddress = GameAddresses[GAID_INSTANT_TYPE_WRITER_MENU];
	instantTypeWriterMenuPatch_.AddNops(instantTypeWriterMenuPatchAddress, 2);
	instantTypeWriterMenuPatch_.AddNops(instantTypeWriterMenuPatchAddress + 0x4, 5);

	bhd_ExecuteTrigger_hook.Set((char*)GameAddresses[GAID_EXECUTE_TRIGGER], (char*)&hk_bhd_ExecuteTrigger, 6);
	bhd_ExecuteTrigger_hook.Apply();

	bhd_0041fd70_hook.Set((char*)GameAddresses[GAID_0041FD70], (char*)&hk_bhd_0041fd70, 9);
	bhd_0041fd70_hook.Apply();

	// Prevents saves from any rooms from showing as "No Data" (will show as Mansion Dining Room instead)
	invalidSaveFixPatch_.AddCode(GameAddresses[GAID_INVALID_SAVE_FIX], { 0xB9, 0x1, 0x0, 0x0, 0x0 });
	invalidSaveFixPatch_.Apply();
}

void SaveTool::UpdateUI()
{
	if (ImGui::CollapsingHeader("Save Anywhere"))
	{
		ImGui::Indent();
		ImGui::Text("NOTE : All saves using the tool done in other areas");
		ImGui::Text("than regular save rooms will appear as \"Mansion Dining Room\".");
		if (ImGui::Button("Open save menu"))
		{
			openSaveMenu_ = true;
		}
		ImGui::Unindent();
	}
}