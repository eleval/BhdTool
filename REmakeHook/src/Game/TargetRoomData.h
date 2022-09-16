#pragma once

#include <cstdint>

struct TargetRoomData
{
	// XYZ coordinates where the player will be teleported
	// NOTE : BHD uses an y up coordinate system
	float x;
	float y;
	float z;

	// No clue what these do, one might be the direction the player is facing
	uint32_t page;
	float angle;

	// Check https://residentevilmodding.boards.net/thread/3072/remaster-arc-list for the room list
	// It's also available in the game's files : Resident Evil Biohazard HD REMASTER\nativePC\arc\room
	uint32_t stage;
	uint32_t room;
	uint32_t cut;
};