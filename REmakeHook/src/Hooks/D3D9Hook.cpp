#include "pch.h"

#include "Hooks/D3D9Hook.h"

#include "BhdTool.h"
#include "ImGui_Impl.h"

#include "Game/GameAddress.h"
#include "Game/GameData.h"

#include "imgui/imgui.h"

#include "Utils/CallHook.h"
#include "Utils/CodePatch.h"
#include "Utils/TrampHook.h"

#include <atomic>
#include <d3d9.h>

#include <string>

namespace
{
	HWND window_ = nullptr;
	uintptr_t d3d9DeviceVTableAddr_ = 0;
	uintptr_t* pD3D9DeviceVTableAddr_ = &d3d9DeviceVTableAddr_;

	uintptr_t captureJmpBackAddr_ = 0;

	void* d3d9Device_[119];
	TrampHook preRenderHook_;
	TrampHook beginSceneHook_;
	TrampHook endSceneHook_;
	TrampHook presentHook_;

	CodePatch captureDeviceHook_;

	bool updateImGui_ = false;
	std::atomic_bool shouldRenderImGui_ = false;
}

namespace
{

HRESULT APIENTRY hk_BeginScene(LPDIRECT3DDEVICE9 device)
{
	using BeginSceneFunc = HRESULT(APIENTRY*)(LPDIRECT3DDEVICE9);
	BeginSceneFunc d3dBeginScene = (BeginSceneFunc)beginSceneHook_.GetGateway();

	if (g_gpd.d3dDevice == nullptr)
	{
		g_gpd.d3dDevice = device;
	}

	if (ImGui_Impl::IsInitialized())
	{
		if (updateImGui_ && !shouldRenderImGui_)
		{
			ImGui_Impl::NewFrame();
			//ImGui::ShowDemoWindow();
			BhdTool::Update();
			shouldRenderImGui_ = true;
		}
	}
	return d3dBeginScene(device);
}

HRESULT APIENTRY hk_EndScene(LPDIRECT3DDEVICE9 device)
{
	using EndSceneFunc = HRESULT(APIENTRY*)(LPDIRECT3DDEVICE9);
	EndSceneFunc d3dEndScene = (EndSceneFunc)endSceneHook_.GetGateway();

	if (ImGui_Impl::IsInitialized())
	{
		if (updateImGui_ && shouldRenderImGui_)
		{
			ImGui_Impl::EndFrame();
			ImGui::Render();
			ImGui_Impl::Render();
			updateImGui_ = false;
			shouldRenderImGui_ = false;
		}
	}

	return d3dEndScene(device);
}


HRESULT APIENTRY hk_Present(LPDIRECT3DDEVICE9 device, const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion)
{
	using PresentFunc = HRESULT(APIENTRY*)(LPDIRECT3DDEVICE9, const RECT*, const RECT*, HWND, const RGNDATA*);
	PresentFunc d3dPresent = (PresentFunc)presentHook_.GetGateway();

	return d3dPresent(device, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

// This function is called before the game's rendering is done
// We are using this to know if we should update ImGui next Begin/EndScene or not
void __fastcall hk_bhd_0x00767260(void* obj, void* edx, int param_1)
{
	using bhd_0x00767260 = void(__fastcall*)(void*, void*, int);
	bhd_0x00767260 func = (bhd_0x00767260)preRenderHook_.GetGateway();
	updateImGui_ = true;
	func(obj, edx, param_1);
}

__declspec(naked) void hk_CaptureD3D9Device()
{
	__asm
	{
		push eax
		mov eax, pD3D9DeviceVTableAddr_
		mov dword ptr [eax], ecx
		pop eax

		push 0x0
		push 0x0
		push eax
		jmp [captureJmpBackAddr_]
	}
}

}


void D3D9Hook::InstallD3D9DeviceCaptureHook()
{
	captureJmpBackAddr_ = GameAddresses[GAID_CAPTURE_DEVICE_JMP_BACK];

	uint8_t* funcAddr = (uint8_t*)&hk_CaptureD3D9Device;
	uint8_t* src = (uint8_t*)GameAddresses[GAID_CAPTURE_DEVICE_JMP];

	uint8_t hookRelAdd[4];
	*(uintptr_t*)(hookRelAdd) = (uintptr_t)(funcAddr - src - 5);

	captureDeviceHook_.AddCode(GameAddresses[GAID_CAPTURE_DEVICE_JMP], { 0xE9, hookRelAdd[0], hookRelAdd[1], hookRelAdd[2], hookRelAdd[3] });
	captureDeviceHook_.Apply();
}

void D3D9Hook::RemoveD3D9DeviceCaptureHook()
{
	captureDeviceHook_.Remove();
}

void D3D9Hook::InstallHooks()
{
	memcpy(d3d9Device_, (void*)d3d9DeviceVTableAddr_, sizeof(d3d9Device_));

	preRenderHook_.Set((char*)GameAddresses[GAID_PRE_RENDER], (char*)&hk_bhd_0x00767260, 6);
	preRenderHook_.Apply();

	beginSceneHook_.Set((char*)d3d9Device_[41], (char*)&hk_BeginScene, 7);
	beginSceneHook_.Apply();
	endSceneHook_.Set((char*)d3d9Device_[42], (char*)&hk_EndScene, 7);
	endSceneHook_.Apply();
}

bool D3D9Hook::HasFoundD3D9Device()
{
	return d3d9DeviceVTableAddr_ != 0;
}