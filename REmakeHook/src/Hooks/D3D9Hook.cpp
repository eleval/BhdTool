#include "pch.h"

#include "Hooks/D3D9Hook.h"

#include "BhdTool.h"
#include "ImGui_Impl.h"

#include "Game/GameData.h"

#include "imgui/imgui.h"

#include "Utils/CallHook.h"
#include "Utils/CodePatch.h"
#include "Utils/TrampHook.h"

#include <d3d9.h>

#include <string>

namespace
{
	HWND window_ = nullptr;
	void* d3d9Device_[119];
	TrampHook preRenderHook_;
	TrampHook beginSceneHook_;
	TrampHook endSceneHook_;
	TrampHook presentHook_;

	bool updateImGui_ = false;
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
		if (updateImGui_)
		{
			ImGui_Impl::NewFrame();
			//ImGui::ShowDemoWindow();
			BhdTool::Update();
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
		if (updateImGui_)
		{
			ImGui_Impl::EndFrame();
			ImGui::Render();
			ImGui_Impl::Render();
			updateImGui_ = false;
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

}

void D3D9Hook::FindD3D9DeviceAndInstallHooks()
{
	if (GetD3D9Device(d3d9Device_, sizeof(d3d9Device_)))
	{
		preRenderHook_.Set((char*)0x00767260, (char*)&hk_bhd_0x00767260, 6);
		preRenderHook_.Apply();

		beginSceneHook_.Set((char*)d3d9Device_[41], (char*)&hk_BeginScene, 7);
		beginSceneHook_.Apply();
		endSceneHook_.Set((char*)d3d9Device_[42], (char*)&hk_EndScene, 7);
		endSceneHook_.Apply();

		// Hard to hook on Present has the Steam overlay already does this
		// Check https://www.unknowncheats.me/forum/general-programming-and-reversing/116896-hooking-renderers-via-steam-overlay.html
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