#include "pch.h"

#include "D3D9Hook.h"

#include "CodePatch.h"
#include "FunctionHook.h"
#include "GameData.h"
#include "ImGui_Impl.h"

#include "imgui/imgui.h"

#include <d3d9.h>

namespace
{
	HWND window_ = nullptr;
	void* d3d9Device_[119];
	TrampHook beginSceneHook_;
	TrampHook endSceneHook_;
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

void APIENTRY hk_BeginScene(LPDIRECT3DDEVICE9 device)
{
	BeginSceneFunc d3dBeginScene = (BeginSceneFunc)beginSceneHook_.GetGateway();

	d3dBeginScene(device);
}

using EndSceneFunc = HRESULT(APIENTRY*)(LPDIRECT3DDEVICE9);

void APIENTRY hk_EndScene(LPDIRECT3DDEVICE9 device)
{
	EndSceneFunc d3dEndScene = (EndSceneFunc)endSceneHook_.GetGateway();

	if (g_gpd.d3dDevice == nullptr)
	{
		g_gpd.d3dDevice = device;
	}

	if (ImGui_Impl::IsInitialized())
	{
		ImGui_Impl::NewFrame();
		ImGui::ShowDemoWindow();
		ImGui_Impl::EndFrame();
		ImGui::Render();
		ImGui_Impl::Render();
	}

	d3dEndScene(device);
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

}

void D3D9Hook::Init()
{
	FunctionHook hook;
	hook.Set(0x00478DB4, &hk_bhd_0x00666ac0);
	hook.Apply();

	FunctionHook hook2;
	hook2.Set(0x0085b1e4, &hk_bhd_0x00768480);
	hook2.Apply();
}