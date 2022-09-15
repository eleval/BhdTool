#pragma once

#include <cstdint>

struct TargetRoomData
{
	// XYZ coordinates where the player will be teleported
	// NOTE : BHD uses an y up coordinate system
	float x;
	float y;
	float z;

	// No clue what these do
	uint32_t d;
	int e;

	// room1 is the room "category" (1 = Mansion, 2 = Courtyard, etc...)
	// room2 is the room ID within its category
	// Check https://residentevilmodding.boards.net/thread/3072/remaster-arc-list for the room list
	// It's also available in the game's files : Resident Evil Biohazard HD REMASTER\nativePC\arc\room
	uint32_t room1;
	uint32_t room2;
	
	// This is a pointer to something but no idea what
	int f;
};