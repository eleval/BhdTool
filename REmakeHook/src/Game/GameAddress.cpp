#include "pch.h"

#include "Game/GameAddress.h"

#include <cassert>

std::array<size_t, GAID_COUNT> GameAddresses;

void InitGameAddresses(GameVersion gameVersion)
{
	switch (gameVersion)
	{
		case GameVersion::V_3831846201811674141:
		case GameVersion::V_7029559557684186662:
		case GameVersion::V_7029559557684186662_Steamless:
		case GameVersion::V_9087090834863155228:
		case GameVersion::V_9087090834863155228_Steamless:
		{
			GameAddresses[GAID_LATE_GAME_UPDATE] = 0x0085b1e4;
			GameAddresses[GAID_LATE_GAME_UPDATE_RET] = 0x00768480;

			GameAddresses[GAID_GAME_SHUTDOWN] = 0x00859a30;

			GameAddresses[GAID_WND_PROC] = 0x00859b00;
			GameAddresses[GAID_UPDATE_KEYBOARD_INPUT] = 0x007308b0;

			GameAddresses[GAID_CAPTURE_DEVICE_JMP] = 0x0076b677;
			GameAddresses[GAID_CAPTURE_DEVICE_JMP_BACK] = 0x0076b67c;
			GameAddresses[GAID_PRE_RENDER] = 0x00767260;

			GameAddresses[GAID_CHECK_FOR_TRIGGERS] = 0x0041dc40;
			GameAddresses[GAID_FETCH_DOOR_ROOM_DATA] = 0x0041e340;
			GameAddresses[GAID_FETCH_DOOR_ROOM_DATA_OFFSET] = GameAddresses[GAID_FETCH_DOOR_ROOM_DATA] + 0xA;
			GameAddresses[GAID_EXECUTE_TRIGGER] = 0x00409ae0;
			GameAddresses[GAID_0041FD70] = 0x0041fd70;

			GameAddresses[GAID_INSTANT_TYPE_WRITER_MENU] = 0x00409c81;
			GameAddresses[GAID_INVALID_SAVE_FIX] = 0x004304be;

			GameAddresses[GAID_DOOR_SKIP_ANIM] = 0x0041CDD6;
			GameAddresses[GAID_DOOR_SKIP_RENDER] = 0x0042B0BF;
			GameAddresses[GAID_DOOR_SKIP_ANIM_START] = 0x0042AEC5;
			GameAddresses[GAID_DOOR_SKIP_SOUNDS] = 0x0041CDE6;
			GameAddresses[GAID_DOOR_SKIP_LAB_ELEVATOR_FIX] = 0x00611A1A;

			GameAddresses[GAID_CHEAT_INFINITE_AMMO] = 0x0065f804;
			GameAddresses[GAID_CHEAT_INFINITE_HEALTH] = 0x0050f4c2;

			GameAddresses[GAID_INVENTORY] = 0x00d7c9c0;
		} break;
		default:
			assert(false);
			break;
	}
}