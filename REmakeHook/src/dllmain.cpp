// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "Hooks/D3D9Hook.h"
#include "Hooks/DoorSkip.h"

#include "Utils/CodePatch.h"

#include <unknwn.h>

#ifdef DLL_EXPORT
#define DINPUT8_API __declspec(dllexport)
#else
#define DINPUT8_API __declspec(dllimport)
#endif

typedef HRESULT(WINAPI*DirectInput8Create_t)(
	HINSTANCE hinst,
	DWORD dwVersion,
	REFIID riidltf,
	LPVOID* ppvOut,
	LPUNKNOWN punkOuter
	);

extern DirectInput8Create_t OriginalFunction;
extern HMODULE DInput8DLL;

extern "C"
{
	DINPUT8_API HRESULT WINAPI DirectInput8Create(
		HINSTANCE hinst,
		DWORD dwVersion,
		REFIID riidltf,
		LPVOID* ppvOut,
		LPUNKNOWN punkOuter
	);
}

void Init()
{
	// Load the original dinput8.dll from the system directory
	char DInputDllName[MAX_PATH];
	GetSystemDirectoryA(DInputDllName, MAX_PATH);
	strcat_s(DInputDllName, "\\dinput8.dll");
	DInput8DLL = LoadLibraryA(DInputDllName);
	if (DInput8DLL > (HMODULE)31)
	{
		OriginalFunction = (DirectInput8Create_t)GetProcAddress(DInput8DLL, "DirectInput8Create");
	}

	D3D9Hook::Init();

	DoorSkip::Init();
	//DoorSkip::Enable();
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		Init();
		break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

DirectInput8Create_t OriginalFunction = nullptr;
HMODULE DInput8DLL = nullptr;


DINPUT8_API HRESULT WINAPI DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter)
{
	#pragma comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)
	if (OriginalFunction)
	{
		return OriginalFunction(hinst, dwVersion, riidltf, ppvOut, punkOuter);
	}
	return S_FALSE;
}