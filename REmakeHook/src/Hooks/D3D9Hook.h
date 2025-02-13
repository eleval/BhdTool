#pragma once

namespace D3D9Hook
{
	void InstallD3D9DeviceCaptureHook();
	void RemoveD3D9DeviceCaptureHook();
	void InstallHooks();
	bool HasFoundD3D9Device();
}