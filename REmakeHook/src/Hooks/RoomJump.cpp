#include "pch.h"

#include "Hooks/RoomJump.h"

#include "Game/TargetRoomData.h"
#include "Utils/TrampHook.h"

#include "imgui/imgui.h"

namespace
{
	int targetRoomNb_ = 0;
	float targetRoomX_ = 100.0f;
	float targetRoomY_ = 100.0f;
	float targetRoomZ_ = 100.0f;
	bool jumpRoom_ = false;

	using bhd_CheckForTriggers =  int(__thiscall*)(void*, int, uint32_t*, TargetRoomData*);
	bhd_CheckForTriggers checkForTriggers_ = nullptr;
	TrampHook checkForTriggersHook_;
}

namespace
{

/*void hk_bhd_FetchDoorRoomData(TargetRoomData* target, int roomData)
{
	float uVar1;
	float uVar2;

	target->room1 = (uint32_t) * (byte*)(roomData + 0x40);
	target->room2 = (uint32_t) * (byte*)(roomData + 0x41);
	target->room3 = (int)*(char*)(roomData + 0x42);
	uVar1 = *(float*)(roomData + 0x34);
	uVar2 = *(float*)(roomData + 0x38);
	target->x = *(float*)(roomData + 0x30);
	target->y = uVar1;
	target->z = uVar2;
	target->d = 0;
	target->e = *(int*)(roomData + 0x48);

	return;

	//bhd_FetchDoorRoomDataFunc_(target, roomData);
}*/

int __fastcall hk_bhd_CheckForTriggers(void* obj, void* edx, int param_1, uint32_t* param_2, TargetRoomData* target)
{
	if (jumpRoom_)
	{
		target->room1 = targetRoomNb_ / 0x100;
		target->room2 = targetRoomNb_ % 0x100;
		target->x = targetRoomX_;
		target->y = targetRoomY_;
		target->z = targetRoomZ_;
		target->d = 0;
		target->e = 0;
		target->f = 0;

		jumpRoom_ = false;
		return 1;
	}
	else
	{
		return checkForTriggers_(obj, param_1, param_2, target);
	}
}

}

void RoomJump::InstallHook()
{
	checkForTriggersHook_.Set((char*)0x0041dc40, (char*)&hk_bhd_CheckForTriggers, 9);
	checkForTriggersHook_.Apply();
	checkForTriggers_ = (bhd_CheckForTriggers)checkForTriggersHook_.GetGateway();

	/*uintptr_t funcAdd = (uintptr_t)(&hk_bhd_FetchDoorRoomData);
	funcAdd = funcAdd - 0x0041e34A - 5;
	uint8_t* funcAdd8 = (uint8_t*)(&funcAdd);*/

	/*CodePatch cp;
	cp.AddCode(0x0041e340, { 0x8b, 0x44, 0x24, 0x08 });
	cp.AddCode(0x0041e344, { 0x8b, 0x4c, 0x24, 0x04 });
	cp.AddCode(0x0041e348, { 0x50 });
	cp.AddCode(0x0041e349, { 0x51 });
	cp.AddCode(0x0041e34A, { 0xE8, funcAdd8[0], funcAdd8[1], funcAdd8[2], funcAdd8[3] });
	cp.AddCode(0x0041e34F, { 0x83, 0xC4, 0x08 });
	cp.AddCode(0x0041e352, { 0xC2, 0x08, 0x00 });
	cp.AddCode(0x0041e355, { 0x90 });
	cp.Apply();*/


	/*FunctionHook hook3;
	hook3.Set(0x0041e06a, &hk_bhd_FetchDoorRoomData);
	hook3.Apply();*/

	/*fetchDoorRoomDataHook_.Set((char*)0x0041e340, (char*)&hk_bhd_FetchDoorRoomData, 8);
	fetchDoorRoomDataHook_.Apply();/*
	bhd_FetchDoorRoomDataFunc_ = (bhd_FetchDoorRoomData)fetchDoorRoomDataHook_.GetGateway();*/

	/*checkTriggersHook_.Set((char*)0x0040a200, (char*)&hk_bhd_CheckTriggers, 7);
	checkTriggersHook_.Apply();

	bhd_CheckDoor = (uint32_t(__stdcall*)(float*, int))checkTriggersHook_.GetGateway();*/
}

void RoomJump::UpdateUI()
{
	if (ImGui::CollapsingHeader("Room Jump"))
	{
		ImGui::InputInt("Room", &targetRoomNb_, 1, 100, ImGuiInputTextFlags_CharsHexadecimal);
		if (ImGui::Button("Jump"))
		{
			jumpRoom_ = true;
		}
	}
}