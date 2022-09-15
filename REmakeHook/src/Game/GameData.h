#pragma once

struct GamePlatformData
{
	HWND hwnd = nullptr;
	LPDIRECT3DDEVICE9 d3dDevice = nullptr;
};

extern GamePlatformData g_gpd;