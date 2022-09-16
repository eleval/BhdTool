#include "pch.h"

#include "Tools/RoomJumpTool.h"

#include "Game/TargetRoomData.h"
#include "Utils/TrampHook.h"

#include "json11/json11.hpp"
#include "imgui/imgui.h"

#include <algorithm>
#include <string.h>

namespace
{
	struct RoomJumpPoint
	{
		float x;
		float y;
		float z;
		float angle;
		uint32_t cut;
		uint32_t page;
	};

	struct Room
	{
		std::string name;
		uint32_t roomNb;
		std::vector<RoomJumpPoint> jumpPoints;
	};

	std::vector<Room> rooms_;
	int selectedRoom_ = 0;
	int selectedJumpPoint_ = 0;

	int jumpRoomNb_ = 0;
	float jumpCoords_[3] = { 0.0f, 0.0f, 0.0f };
	float jumpAngle_ = 0.0f;
	uint32_t jumpCut_ = 0;
	uint32_t jumpPage_ = 0;
	bool jumpRoom_ = false;

	using bhd_CheckForTriggers =  int(__thiscall*)(void*, int, uint32_t*, TargetRoomData*);
	bhd_CheckForTriggers checkForTriggers_ = nullptr;
	TrampHook checkForTriggersHook_;
}

namespace
{

void hk_bhd_FetchDoorRoomData(TargetRoomData* target, int roomData)
{
	float uVar1;
	float uVar2;

	target->stage = (uint32_t) * (byte*)(roomData + 0x40);
	target->room = (uint32_t) * (byte*)(roomData + 0x41);
	target->cut = (int)*(char*)(roomData + 0x42);
	uVar1 = *(float*)(roomData + 0x34);
	uVar2 = *(float*)(roomData + 0x38);
	target->x = *(float*)(roomData + 0x30);
	target->y = uVar1;
	target->z = uVar2;
	target->page = 0;
	target->angle = *(float*)(roomData + 0x48);

	//bhd_FetchDoorRoomDataFunc_(target, roomData);
}

int __fastcall hk_bhd_CheckForTriggers(void* obj, void* edx, int param_1, uint32_t* param_2, TargetRoomData* target)
{
	if (jumpRoom_)
	{
		target->stage = jumpRoomNb_ / 0x100;
		target->room = jumpRoomNb_ % 0x100;
		target->cut = jumpCut_;
		target->x = jumpCoords_[0];
		target->y = jumpCoords_[1];
		target->z = jumpCoords_[2];
		target->page = jumpPage_;
		target->angle = (jumpAngle_ / 180.0f) * PI;

		jumpRoom_ = false;
		return 1;
	}
	else
	{
		return checkForTriggers_(obj, param_1, param_2, target);
	}
}

}

void RoomJumpTool::Init()
{
	checkForTriggersHook_.Set((char*)0x0041dc40, (char*)&hk_bhd_CheckForTriggers, 9);
	checkForTriggersHook_.Apply();
	checkForTriggers_ = (bhd_CheckForTriggers)checkForTriggersHook_.GetGateway();

	uintptr_t funcAdd = (uintptr_t)(&hk_bhd_FetchDoorRoomData);
	funcAdd = funcAdd - 0x0041e34A - 5;
	uint8_t* funcAdd8 = (uint8_t*)(&funcAdd);

	CodePatch cp;
	cp.AddCode(0x0041e340, { 0x8b, 0x44, 0x24, 0x08 });
	cp.AddCode(0x0041e344, { 0x8b, 0x4c, 0x24, 0x04 });
	cp.AddCode(0x0041e348, { 0x50 });
	cp.AddCode(0x0041e349, { 0x51 });
	cp.AddCode(0x0041e34A, { 0xE8, funcAdd8[0], funcAdd8[1], funcAdd8[2], funcAdd8[3] });
	cp.AddCode(0x0041e34F, { 0x83, 0xC4, 0x08 });
	cp.AddCode(0x0041e352, { 0xC2, 0x08, 0x00 });
	cp.AddCode(0x0041e355, { 0x90 });
	cp.Apply();


	std::string jsonStr;
	FILE* jsonFile = fopen("rooms.json", "rb");
	if (jsonFile != nullptr)
	{
		fseek(jsonFile, 0, SEEK_END);
		const size_t jsonFileSize = ftell(jsonFile);
		fseek(jsonFile, 0, SEEK_SET);
		jsonStr.resize(jsonFileSize);
		fread(jsonStr.data(), 1, jsonFileSize, jsonFile);
		fclose(jsonFile);
	}

	std::string jsonErrors;
	json11::Json roomsJson = json11::Json::parse(jsonStr, jsonErrors);
	for (const auto& roomJson : roomsJson.object_items())
	{
		Room room;
		room.name = roomJson.first;
		sscanf_s(roomJson.first.c_str(), "%x", &room.roomNb);
		for (const auto& jumpPointJson : roomJson.second.array_items())
		{
			RoomJumpPoint jumpPoint;
			jumpPoint.x = static_cast<float>(jumpPointJson["x"].number_value());
			jumpPoint.y = static_cast<float>(jumpPointJson["y"].number_value());
			jumpPoint.z = static_cast<float>(jumpPointJson["z"].number_value());
			jumpPoint.angle = static_cast<float>(jumpPointJson["angle"].number_value());
			jumpPoint.cut = jumpPointJson["cut"].int_value();
			jumpPoint.page = jumpPointJson["page"].int_value();
			room.jumpPoints.push_back(jumpPoint);
		}
		rooms_.push_back(room);
	}

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

void RoomJumpTool::UpdateUI()
{
	if (ImGui::CollapsingHeader("Room Jump"))
	{
		if (!rooms_.empty())
		{
			if (ImGui::BeginCombo("Room", rooms_[selectedRoom_].name.c_str()))
			{
				for (int i = 0; i < static_cast<int>(rooms_.size()); ++i)
				{
					bool selected = i == selectedRoom_;
					if (ImGui::Selectable(rooms_[i].name.c_str(), &selected))
					{
						selectedRoom_ = i;
						selectedJumpPoint_ = 0;
					}
				}
				ImGui::EndCombo();
			}
			ImGui::Text("Doors");
			const Room& room = rooms_[selectedRoom_];
			for (int i = 0; i < static_cast<int>(room.jumpPoints.size()); ++i)
			{
				if (i % 4 != 0)
				{
					ImGui::SameLine();
				}
				if (ImGui::RadioButton(std::to_string(i).c_str(), i == selectedJumpPoint_))
				{
					selectedJumpPoint_ = i;
				}
			}

			static bool manualEnter = false;
			ImGui::Checkbox("Set coords manually", &manualEnter);
			if (manualEnter)
			{
				ImGui::InputFloat3("Coords", jumpCoords_);
				ImGui::InputFloat("Angle", &jumpAngle_);
				jumpAngle_ = std::clamp(jumpAngle_, 0.0f, 360.0f);
			}

			if (ImGui::Button("Jump"))
			{
				const Room& room = rooms_[selectedRoom_];
				const RoomJumpPoint& jumpPoint = room.jumpPoints[selectedJumpPoint_];
				jumpRoomNb_ = room.roomNb;

				if (!manualEnter)
				{
					jumpCoords_[0] = jumpPoint.x;
					jumpCoords_[1] = jumpPoint.y;
					jumpCoords_[2] = jumpPoint.z;
					jumpAngle_ = jumpPoint.angle;
					jumpCut_ = jumpPoint.cut;
					jumpPage_ = jumpPoint.page;
				}
				else
				{
					jumpCut_ = 0;
					jumpPage_ = 0;
				}
				jumpRoom_ = true;
			}
		}
	}
}