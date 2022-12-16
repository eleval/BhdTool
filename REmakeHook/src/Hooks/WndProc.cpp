#include "pch.h"

#include "Hooks/WndProc.h"

#include "BhdTool.h"
#include "ImGui_Impl.h"

#include "Utils/CallHook.h"
#include "Utils/TrampHook.h"

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
	bhd_WndProc_t bhd_WndProc = (bhd_WndProc_t)0x00859b00;

	ImGui_Impl::ProcessEvent(message, wParam, lParam);

	switch (message)
	{
		case WM_ACTIVATE:
		case WM_ACTIVATEAPP:
		case WM_NCACTIVATE:
		{
			if (BhdTool::IsOpen())
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
	if (GetAsyncKeyState(VK_F5) & 0x8000)
	{
		if (!wasF5Pressed)
		{
			wasF5Pressed = true;
			BhdTool::Toggle();
			if (BhdTool::IsOpen())
			{
				LPDIRECTINPUTDEVICE8A dInputDevice = (LPDIRECTINPUTDEVICE8A)(*(size_t*)((uintptr_t)obj + 0xa08));
				dInputDevice->Unacquire();
				return;
			}
		}
	}
	else
	{
		wasF5Pressed = false;
	}

	if (BhdTool::IsOpen())
	{
		return;
	}

	func(obj, edx, param_1);
}

}

void WndProc::InstallHook()
{
	// Patch RegisterClass parameters to use our WndProc instead of the game's
	CodePatch cp;
	uintptr_t wndProcAdd = (uintptr_t)(&hk_bhd_WndProc);
	cp.AddCode(0x0085bc33, wndProcAdd);
	cp.Apply();
	
	bhd_UpdateKeyboardInputHook_.Set((char*)0x007308b0, (char*)&hk_bhd_UpdateKeyboardInput, 6);
	bhd_UpdateKeyboardInputHook_.Apply();
}