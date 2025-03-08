#pragma once

namespace D3D9Hook
{
	void InstallD3D9DeviceCaptureHook();
	void RemoveD3D9DeviceVTableCaptureHook();
	void InstallHooks();
	bool HasFoundD3D9DeviceVTable();
	bool HasFoundD3D9Device();
}