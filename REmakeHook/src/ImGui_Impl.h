#pragma once

#include <d3d9.h>

namespace ImGui_Impl
{
	void Init();
	void Shutdown();
	bool IsInitialized();

	void NewFrame();
	void EndFrame();

	void Render();
}