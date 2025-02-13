#include "pch.h"

#include "Hooks/Hooks.h"

#include "BhdTool.h"
#include "ImGui_Impl.h"
#include "Settings.h"

#include "Game/GameAddress.h"
#include "Game/GameData.h"

#include "Hooks/D3D9Hook.h"
#include "Hooks/WndProc.h"

#include "Utils/CallHook.h"
#include "Utils/CodePatch.h"
#include "Utils/Memory.h"
#include "Utils/TrampHook.h"

#include <cassert>
#include <thread>

namespace
{
	TrampHook bhd_00859a30_hook;
	bool installedHooks_ = false;

	uintptr_t devicePtr = 0;
	uintptr_t* pDevicePtr = &devicePtr;
}

namespace
{

// Called when the game is freeing resources
void __fastcall hk_bhd_00859a30(void* param_1)
{
	using bhd_00859a30_t = void(__fastcall*)(void*);
	bhd_00859a30_t bhd_00859a30 = (bhd_00859a30_t)bhd_00859a30_hook.GetGateway();

	// Use this to shutdown our things and save settings
	Settings::SaveSettings();

	bhd_00859a30(param_1);
}

// This function is called near the end of the game's update thread
void __fastcall hk_bhd_0x00768480(int param)
{
	// Initialize all our hooks here since it's the safest place to do so.
	// We can't do it when the process starts anymore since the game's executable is encrypted by Steam's DRM
	if (!installedHooks_)
	{
		if (D3D9Hook::HasFoundD3D9Device())
		{
			D3D9Hook::RemoveD3D9DeviceCaptureHook();
			D3D9Hook::InstallHooks();
			BhdTool::Init();
			WndProc::InstallHooks();

			// Game shutdown hook
			bhd_00859a30_hook.Set((char*)GameAddresses[GAID_GAME_SHUTDOWN], (char*)&hk_bhd_00859a30, 9);
			bhd_00859a30_hook.Apply();

			installedHooks_ = true;
		}
	}

	// This is done in in this update function because we need to wait for at least one BeginScene to get the D3D9Device
	// This is sorta ugly but it works
	if (g_gpd.d3dDevice != nullptr && !ImGui_Impl::IsInitialized())
	{
		ImGui_Impl::Init();
	}

	void* funcPtr = (void*)GameAddresses[GAID_LATE_GAME_UPDATE_RET];
	__asm
	{
		mov ecx, param
		call dword ptr[funcPtr]
	}
}


}

void Hooks::InstallHooks()
{
	CallHook hook;
	
	// Late game update loop hook
	hook.Set(GameAddresses[GAID_LATE_GAME_UPDATE], &hk_bhd_0x00768480);
	hook.Apply();

	D3D9Hook::InstallD3D9DeviceCaptureHook();
}