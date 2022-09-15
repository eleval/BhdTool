#include "pch.h"

#include "Hooks/WndProc.h"

#include "ImGui_Impl.h"

#include "Utils/CallHook.h"

namespace
{

LRESULT CALLBACK hk_bhd_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	using bhd_WndProc_t = LRESULT(CALLBACK*)(HWND, UINT, WPARAM, LPARAM);
	bhd_WndProc_t bhd_WndProc = (bhd_WndProc_t)0x00859b00;

	// Do not prevent some system keys from doing anyhing
	// TODO : Should have an option to toggle it on/off
	if (message > WM_COMMAND)
	{
		return DefWindowProcW(hWnd, message, wParam, lParam);
	}

	ImGui_Impl::ProcessEvent(message, wParam, lParam);

	switch (message)
	{
		// bhd does some shady things with these 3, which prevents keyboard events from being sent
		// TODO : Investigate in details what is done
		case WM_ACTIVATE:
		case WM_ACTIVATEAPP:
		case WM_NCACTIVATE:
			return DefWindowProcW(hWnd, message, wParam, lParam);
		default:
			return bhd_WndProc(hWnd, message, wParam, lParam);
	}
}

}

void WndProc::InstallHook()
{
	CodePatch cp2;
	uintptr_t wndProcAdd = (uintptr_t)(&hk_bhd_WndProc);
	uint8_t* wndProcAdd8 = (uint8_t*)(&wndProcAdd);
	cp2.AddCode(0x0085bc33, wndProcAdd);
	cp2.Apply();
}