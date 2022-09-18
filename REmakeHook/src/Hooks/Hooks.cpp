#include "pch.h"

#include "Hooks/Hooks.h"

#include "ImGui_Impl.h"
#include "Settings.h"

#include "Game/GameData.h"

#include "Hooks/D3D9Hook.h"
#include "Hooks/WndProc.h"

#include "Utils/CallHook.h"
#include "Utils/CodePatch.h"
#include "Utils/TrampHook.h"

namespace
{
	TrampHook bhd_00859a30_hook;
}

namespace
{

// This function is called post engine init
void hk_bhd_0x00666ac0()
{
	using bhd_0x0666ac0 = void (*)();
	bhd_0x0666ac0 func = reinterpret_cast<bhd_0x0666ac0>(0x00666ac0);
	func();

	D3D9Hook::FindD3D9DeviceAndInstallHooks();
}

// This function is called near the end of the game's update thread
void __fastcall hk_bhd_0x00768480(int param)
{
	// This is done in in this update function because we need to wait for at least one BeginScene to get the D3D9Device
	// This is sorta ugly but it works
	if (g_gpd.d3dDevice != nullptr && !ImGui_Impl::IsInitialized())
	{
		ImGui_Impl::Init();
	}

	void* funcPtr = (void*)0x00768480;
	__asm
	{
		mov ecx, param
		call dword ptr[funcPtr]
	}
}

// Called when the game is freeing resources
void __fastcall hk_bhd_00859a30(void* param_1)
{
	using bhd_00859a30_t = void(__fastcall*)(void*);
	bhd_00859a30_t bhd_00859a30 = (bhd_00859a30_t)bhd_00859a30_hook.GetGateway();

	// Use this to shutdown our things and save settings
	Settings::SaveSettings();

	bhd_00859a30(param_1);
}

}

void Hooks::InstallHooks()
{
	CallHook hook;
	// Post-init hook
	hook.Set(0x00478DB4, &hk_bhd_0x00666ac0);
	hook.Apply();

	// Late game update loop hook
	hook.Set(0x0085b1e4, &hk_bhd_0x00768480);
	hook.Apply();

	// Game shutdown hook
	bhd_00859a30_hook.Set((char*)0x00859a30, (char*)&hk_bhd_00859a30, 9);
	bhd_00859a30_hook.Apply();

	WndProc::InstallHook();
}