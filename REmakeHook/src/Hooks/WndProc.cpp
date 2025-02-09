#include "pch.h"

#include "Hooks/WndProc.h"

#include "BhdTool.h"
#include "ImGui_Impl.h"

#include "Game/GameAddress.h"
#include "Game/GameData.h"
#include "Utils/CallHook.h"
#include "Utils/TrampHook.h"

#include "imgui/imgui.h"

#include <dinput.h>

namespace
{
	TrampHook bhd_UpdateKeyboardInputHook_;
}

namespace
{

LRESULT CALLBACK hk_bhd_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	using bhd_WndProc_t = LRESULT(CALLBACK*)(HWND, UINT, WPARAM, LPARAM);
	bhd_WndProc_t bhd_WndProc = (bhd_WndProc_t)GameAddresses[GAID_WND_PROC];

	ImGui_Impl::ProcessEvent(message, wParam, lParam);

	switch (message)
	{
		case WM_ACTIVATE:
		case WM_ACTIVATEAPP:
		case WM_NCACTIVATE:
		{
			if (ImGui_Impl::IsInitialized() && ImGui::GetIO().WantCaptureKeyboard)
			{
				return DefWindowProcW(hWnd, message, wParam, lParam);
			}
		} break;
	}

	return bhd_WndProc(hWnd, message, wParam, lParam);
}

void __fastcall hk_bhd_UpdateKeyboardInput(void* obj, void* edx, int param_1)
{
	using bhd_UpdateKeyboardInput = void(__fastcall*)(void*, void*, int);
	bhd_UpdateKeyboardInput func = (bhd_UpdateKeyboardInput)bhd_UpdateKeyboardInputHook_.GetGateway();

	static bool wasF5Pressed = false;
	static bool isUnacquired = false;
	if (GetAsyncKeyState(VK_F5) & 0x8000)
	{
		if (!wasF5Pressed)
		{
			wasF5Pressed = true;
			BhdTool::Toggle();
		}
	}
	else
	{
		wasF5Pressed = false;
	}

	if (ImGui_Impl::IsInitialized() && ImGui::GetIO().WantCaptureKeyboard)
	{
		if (!isUnacquired)
		{
			LPDIRECTINPUTDEVICE8A dInputDevice = (LPDIRECTINPUTDEVICE8A)(*(size_t*)((uintptr_t)obj + 0xa08));
			dInputDevice->Unacquire();
			isUnacquired = true;
		}
		return;
	}
	isUnacquired = false;

	func(obj, edx, param_1);
}

}

void WndProc::InstallHooks()
{
	SetWindowLongPtr(g_gpd.hwnd, GWL_WNDPROC, (LONG_PTR)&hk_bhd_WndProc);

	bhd_UpdateKeyboardInputHook_.Set((char*)GameAddresses[GAID_UPDATE_KEYBOARD_INPUT], (char*)&hk_bhd_UpdateKeyboardInput, 6);
	bhd_UpdateKeyboardInputHook_.Apply();
}