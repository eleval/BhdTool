#include "pch.h"

#include "Hooks/WndProc.h"

#include "BhdTool.h"
#include "ImGui_Impl.h"

#include "Utils/CallHook.h"

namespace
{

LRESULT CALLBACK hk_bhd_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	using bhd_WndProc_t = LRESULT(CALLBACK*)(HWND, UINT, WPARAM, LPARAM);
	bhd_WndProc_t bhd_WndProc = (bhd_WndProc_t)0x00859b00;

	ImGui_Impl::ProcessEvent(message, wParam, lParam);

	switch (message)
	{
		// bhd does some shady things with these 3, which prevents keyboard events from being sent
		// TODO : Investigate in details what is done (things like turning off screensaver and sticky keys)
		case WM_ACTIVATE:
		case WM_ACTIVATEAPP:
		case WM_NCACTIVATE:
			return DefWindowProcW(hWnd, message, wParam, lParam);
		case WM_KEYDOWN:
			if (wParam == VK_F5)
			{
				BhdTool::Toggle();
			}
			break;
	}

	return bhd_WndProc(hWnd, message, wParam, lParam);

}

}

void WndProc::InstallHook()
{
	// Patch RegisterClass parameters to use our WndProc instead of the game's
	CodePatch cp;
	uintptr_t wndProcAdd = (uintptr_t)(&hk_bhd_WndProc);
	cp.AddCode(0x0085bc33, wndProcAdd);
	cp.Apply();
}