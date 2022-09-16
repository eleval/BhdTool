#include "pch.h"

#include "Hooks/Hooks.h"

#include "ImGui_Impl.h"

#include "Game/GameData.h"

#include "Hooks/D3D9Hook.h"
#include "Hooks/WndProc.h"

#include "Utils/CallHook.h"

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

	WndProc::InstallHook();
}