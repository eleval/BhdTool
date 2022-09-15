#pragma once

#include <d3d9.h>

namespace ImGui_Impl
{
	void Init();
	void Shutdown();
	bool IsInitialized();

	void ProcessEvent(UINT message, WPARAM wParam, LPARAM lParam);

	void NewFrame();
	void EndFrame();

	void Render();
}