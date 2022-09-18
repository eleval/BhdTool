#include "pch.h"

#include "Hooks/Hooks.h"

#include "ImGui_Impl.h"

#include "Game/GameData.h"

#include "Hooks/D3D9Hook.h"
#include "Hooks/WndProc.h"

#include "Utils/CallHook.h"
#include "Utils/CodePatch.h"
#include "Utils/TrampHook.h"

namespace
{

TrampHook bhd_ExecuteTrigger_hook;
TrampHook bhd_0041fd70_hook;

bool trySave_ = false;

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

using bhd_ExecuteTrigger_t = int(__fastcall*)(void*, void*, int, int, int);
int __fastcall hk_bhd_ExecuteTrigger(void* obj, void* edx, int param_1, int param_2, int param_3)
{
	bhd_ExecuteTrigger_t bhd_ExecuteTrigger = (bhd_ExecuteTrigger_t)bhd_ExecuteTrigger_hook.GetGateway();

	if (trySave_)
	{
		uint8_t* pVal = (uint8_t*)((uintptr_t)param_1 + 0x30);
		*pVal = 6;

		// Adding nops to skip a call showing the text but not skipping a push before it
		// This somehow shows the save menu without corrupting the stack?
		// Anyway, it works, so GG? xD
		CodePatch cp;
		cp.AddNops(0x00409c81, 2);
		cp.AddNops(0x00409c85, 5);
		cp.Apply();

		const int ret = bhd_ExecuteTrigger(obj, edx, param_1, param_2, param_3);;
		cp.Remove();
		return ret;
	}

	return bhd_ExecuteTrigger(obj, edx, param_1, param_2, param_3);
}

int __fastcall hk_bhd_0041fd70(void* obj, void* edx, int param_1)
{
	using bhd_0041fd70_t = int(__fastcall*)(void*, void*, int);
	bhd_0041fd70_t bhd_0041fd70 = (bhd_0041fd70_t)bhd_0041fd70_hook.GetGateway();

	if (trySave_)
	{
		int uVar6 = 0;
		int iVar3 = *(int*)(*(int*)((uintptr_t)obj + 0xe0) + uVar6 * 4);

		bhd_ExecuteTrigger_t bhd_ExecuteTrigger = (bhd_ExecuteTrigger_t)0x00409ae0;
		bhd_ExecuteTrigger(obj, edx, iVar3, uVar6, 0);

		trySave_ = false;

		return 1;
	}

	return bhd_0041fd70(obj, edx, param_1);
}

}

void Hooks::TrySave()
{
	trySave_ = true;
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

	bhd_ExecuteTrigger_hook.Set((char*)0x00409ae0, (char*)&hk_bhd_ExecuteTrigger, 6);
	bhd_ExecuteTrigger_hook.Apply();

	bhd_0041fd70_hook.Set((char*)0x0041fd70, (char*)&hk_bhd_0041fd70, 9);
	bhd_0041fd70_hook.Apply();

	CodePatch cp;
	cp.AddCode(0x004304be, { 0xB9, 0x1, 0x0, 0x0, 0x0 });
	cp.Apply();
}