#include "pch.h"

#include "Hooks/D3D9Hook.h"

#include "ImGui_Impl.h"

#include "Game/GameData.h"
#include "Hooks/DoorSkip.h"

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
	TrampHook beginSceneHook_;
	TrampHook endSceneHook_;
	TrampHook presentHook_;

	bool jumpRoom_ = false;

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

}

void D3D9Hook::FindD3D9DeviceAndInstallHooks()
{
	if (GetD3D9Device(d3d9Device_, sizeof(d3d9Device_)))
	{
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