#include "pch.h"

#include "Hooks/DoorSkip.h"

#include "Utils/CodePatch.h"

namespace
{
	CodePatch doorSkipPatch_;
}

void DoorSkip::InitHook()
{
	doorSkipPatch_.AddCode(0x0041CDD6, { 0xEB, 0x0C, 0x90 }); // Removes the entire door animation
	doorSkipPatch_.AddNops(0x0042B0BF, 6); // Remove door being present for 5 frames
	doorSkipPatch_.AddNops(0x0042AEC5, 6); // Remove beginning of animation
	doorSkipPatch_.AddNops(0x0041CDE6, 5); // Remove sounds
}

void DoorSkip::Enable()
{
	doorSkipPatch_.Apply();
}

void DoorSkip::Disable()
{
	doorSkipPatch_.Remove();
}