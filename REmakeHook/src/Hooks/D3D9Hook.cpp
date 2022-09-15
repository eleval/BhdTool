#include "pch.h"

#include "Hooks/D3D9Hook.h"

#include "ImGui_Impl.h"

#include "Game/GameData.h"
#include "Hooks/DoorSkip.h"

#include "imgui/imgui.h"

#include "Utils/CallHook.h"
#include "Utils/CodePatch.h"

#include <d3d9.h>

#include <string>

struct TargetRoomData
{
	float x;
	float y;
	float z;
	uint32_t d;
	int e;
	uint32_t room1;
	uint32_t room2;
	int room3;
};

namespace
{
	HWND window_ = nullptr;
	void* d3d9Device_[119];
	TrampHook beginSceneHook_;
	TrampHook endSceneHook_;
	TrampHook presentHook_;


	TrampHook fetchDoorRoomDataHook_;
	using bhd_FetchDoorRoomData = void(*)(TargetRoomData*, int);
	bhd_FetchDoorRoomData bhd_FetchDoorRoomDataFunc_ = nullptr;

	uint32_t(__stdcall*bhd_CheckDoor)(float*, int) = nullptr;

	TrampHook checkTriggersHook_;

	bool jumpRoom_ = false;

	typedef int(__thiscall* bhd_CheckForTriggers)(void*, int, uint32_t*, uint32_t);
	bhd_CheckForTriggers checkForTriggers_ = nullptr;
	TrampHook checkForTriggersHook_;

	char roomName[32] = { 0, };
	int roomNb = 0;

	char testStr[255];
}

namespace
{

BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam)
{
	DWORD wndProcId;
	GetWindowThreadProcessId(handle, &wndProcId);

	if (GetCurrentProcessId() != wndProcId)
		return TRUE; // skip to next window

	RECT rect;
	if (!GetWindowRect(handle, &rect))
	{
		return TRUE;
	}

	if (rect.left == 0xffffffff)
	{
		return TRUE;
	}

	window_ = handle;
	return FALSE; // window found abort search
}

HWND GetProcessWindow()
{
	window_ = NULL;
	EnumWindows(EnumWindowsCallback, NULL);
	return window_;
}

bool GetD3D9Device(void** pTable, size_t Size)
{
	if (!pTable)
		return false;

	IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);

	if (!pD3D)
		return false;

	IDirect3DDevice9* pDummyDevice = NULL;

	// options to create dummy device
	D3DPRESENT_PARAMETERS d3dpp = {};
	d3dpp.Windowed = true;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = GetProcessWindow();

	g_gpd.hwnd = d3dpp.hDeviceWindow;

	HRESULT dummyDeviceCreated = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDummyDevice);

	if (dummyDeviceCreated != S_OK)
	{
		// may fail in windowed fullscreen mode, trying again with windowed mode
		d3dpp.Windowed = !d3dpp.Windowed;

		dummyDeviceCreated = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDummyDevice);

		if (dummyDeviceCreated != S_OK)
		{
			pD3D->Release();
			return false;
		}
	}

	memcpy(pTable, *reinterpret_cast<void***>(pDummyDevice), Size);

	pDummyDevice->Release();
	pD3D->Release();
	return true;
}

using BeginSceneFunc = HRESULT(APIENTRY*)(LPDIRECT3DDEVICE9);

int test = 0;

HRESULT APIENTRY hk_BeginScene(LPDIRECT3DDEVICE9 device)
{
	BeginSceneFunc d3dBeginScene = (BeginSceneFunc)beginSceneHook_.GetGateway();

	if (ImGui_Impl::IsInitialized())
	{
		if (++test == 1)
		{
			ImGui_Impl::NewFrame();
			//ImGui::ShowDemoWindow();

			if (ImGui::Begin("DoorSkip"))
			{
				static bool v = false;
				if (ImGui::Checkbox("Enable Door Skip", &v))
				{
					if (v)
					{
						DoorSkip::Enable();
					}
					else
					{
						DoorSkip::Disable();
					}
				}
			}
			ImGui::End();

			if (ImGui::Begin("Jump"))
			{
				//ImGui::InputText("Room", roomName, 32);
				ImGui::InputInt("Room", &roomNb, 1, 100, ImGuiInputTextFlags_CharsHexadecimal);
				if (ImGui::Button("Click me"))
				{
					jumpRoom_ = true;
				}
			}
			ImGui::End();
		}
	}
	return d3dBeginScene(device);
}

using EndSceneFunc = HRESULT(APIENTRY*)(LPDIRECT3DDEVICE9);

HRESULT APIENTRY hk_EndScene(LPDIRECT3DDEVICE9 device)
{
	EndSceneFunc d3dEndScene = (EndSceneFunc)endSceneHook_.GetGateway();

	if (g_gpd.d3dDevice == nullptr)
	{
		g_gpd.d3dDevice = device;
	}

	if (ImGui_Impl::IsInitialized())
	{
		if (test == 1)
		{
			ImGui_Impl::EndFrame();
			ImGui::Render();
			ImGui_Impl::Render();
		}
		else if (test == 2)
		{
			test = 0;
		}
	}

	return d3dEndScene(device);
}

using PresentFunc = HRESULT(APIENTRY*)(LPDIRECT3DDEVICE9, const RECT*, const RECT*, HWND, const RGNDATA*);

HRESULT APIENTRY hk_Present(LPDIRECT3DDEVICE9 device, const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion)
{
	PresentFunc d3dPresent = (PresentFunc)presentHook_.GetGateway();

	return d3dPresent(device, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

void hk_bhd_0x00666ac0()
{
	using bhd_0x0666ac0 = void (*)();
	bhd_0x0666ac0 func = reinterpret_cast<bhd_0x0666ac0>(0x00666ac0);
	func();

	if (GetD3D9Device(d3d9Device_, sizeof(d3d9Device_)))
	{
		beginSceneHook_.Set((char*)d3d9Device_[41], (char*)&hk_BeginScene, 7);
		beginSceneHook_.Apply();
		endSceneHook_.Set((char*)d3d9Device_[42], (char*)&hk_EndScene, 7);
		endSceneHook_.Apply();
		/*if (*(uint8_t*)(d3d9Device_[17]) == 0xE9)
		{
			uintptr_t address = (uintptr_t)(d3d9Device_[17]);
			address += 5;
			presentHook_.Set((char*)address, (char*)&hk_Present, 6);
		}
		else
		{
			presentHook_.Set((char*)d3d9Device_[17], (char*)&hk_Present, 5);
		}
		presentHook_.Apply();*/
	}
}

void __fastcall hk_bhd_0x00768480(int param)
{
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

uint32_t __stdcall hk_bhd_CheckTriggers(float* a, int b)
{
	if (jumpRoom_)
	{
		//jumpRoom_ = false;
		return 1;
	}
	else
	{
		return bhd_CheckDoor(a, b);
	}
}

int __fastcall hk_bhd_CheckForTriggers(void* obj, void* edx, int param_1, uint32_t* param_2, int param_3)
{
	if (jumpRoom_)
	{
		//char temp[8];
		TargetRoomData* target = reinterpret_cast<TargetRoomData*>(param_3);

		//temp[0] = roomName[0];
		//temp[1] = 0;
		target->room1 = roomNb / 0x100;
		target->room2 = roomNb % 0x100;
		target->room3 = 0;
		target->x = 100.0f;
		target->y = 100.0f;
		target->z = -200.0f;
		target->d = 0;
		target->e = 0;// *(int*)(roomData + 0x48);
		jumpRoom_ = false;
		return 1;
	}
	else
	{
		return checkForTriggers_(obj, param_1, param_2, param_3);
	}

}

void hk_bhd_FetchDoorRoomData(TargetRoomData* target, int roomData)
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
}

}

LRESULT CALLBACK hk_bhd_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void D3D9Hook::Init()
{
	CallHook hook;
	hook.Set(0x00478DB4, &hk_bhd_0x00666ac0);
	hook.Apply();

	CallHook hook2;
	hook2.Set(0x0085b1e4, &hk_bhd_0x00768480);
	hook2.Apply();

	/*FunctionHook hook3;
	hook3.Set(0x0041e06a, &hk_bhd_FetchDoorRoomData);
	hook3.Apply();*/

	/*fetchDoorRoomDataHook_.Set((char*)0x0041e340, (char*)&hk_bhd_FetchDoorRoomData, 8);
	fetchDoorRoomDataHook_.Apply();/*
	bhd_FetchDoorRoomDataFunc_ = (bhd_FetchDoorRoomData)fetchDoorRoomDataHook_.GetGateway();*/

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

	/*checkTriggersHook_.Set((char*)0x0040a200, (char*)&hk_bhd_CheckTriggers, 7);
	checkTriggersHook_.Apply();

	bhd_CheckDoor = (uint32_t(__stdcall*)(float*, int))checkTriggersHook_.GetGateway();*/

	checkForTriggersHook_.Set((char*)0x0041dc40, (char*)&hk_bhd_CheckForTriggers, 9);
	checkForTriggersHook_.Apply();
	checkForTriggers_ = (bhd_CheckForTriggers)checkForTriggersHook_.GetGateway();

	CodePatch cp2;
	uintptr_t wndProcAdd = (uintptr_t)(&hk_bhd_WndProc);
	uint8_t* wndProcAdd8 = (uint8_t*)(&wndProcAdd);
	cp2.AddCode(0x0085bc33, wndProcAdd);
	cp2.Apply();
}