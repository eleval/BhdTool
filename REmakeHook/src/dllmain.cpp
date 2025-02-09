// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "Game/GameAddress.h"
#include "Game/GameVersion.h"
#include "Hooks/Hooks.h"

#include "BhdTool.h"
#include "Settings.h"

#include <unknwn.h>

#ifdef DLL_EXPORT
#define DINPUT8_API __declspec(dllexport)
#else
#define DINPUT8_API __declspec(dllimport)
#endif

Setting<bool> s_enableBhdTool("EnableBhdTool", true);

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
	wchar_t DInputDllName[MAX_PATH];
	GetSystemDirectoryW(DInputDllName, MAX_PATH);
	wcscat_s(DInputDllName, MAX_PATH, L"\\dinput8.dll");
	DInput8DLL = LoadLibraryW(DInputDllName);
	if (DInput8DLL > (HMODULE)31)
	{
		OriginalFunction = (DirectInput8Create_t)GetProcAddress(DInput8DLL, "DirectInput8Create");
	}

	Settings::LoadSettings();

	if (s_enableBhdTool.Get())
	{
		const GameVersion gameVersion = GetGameVersion();
		if (gameVersion == GameVersion::Unknown)
		{
			const int choice = MessageBoxW(nullptr,
				L"You're running a version of 'Resident Evil' that is not supported by BHD Tool.\n\n"
				L"What do you want to do?\n"
				L"- Abort = Close the game\n"
				L"- Retry = Start the game regardless\n"
				L"- Ignore = Disable BHD Tool and start the game\n",
				L"BHD Tool",
				MB_ABORTRETRYIGNORE | MB_ICONWARNING | MB_APPLMODAL);

			switch (choice)
			{
				case IDABORT:
					TerminateProcess(GetCurrentProcess(), 0);
					break;
				case IDRETRY:
					break;
				case IDIGNORE:
					s_enableBhdTool.Set(false);
					Settings::SaveSettings();
					break;
			}
		}

		InitGameAddresses(gameVersion);
	}
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
	
	if (s_enableBhdTool.Get())
	{
		Hooks::InstallHooks();
	}

	if (OriginalFunction)
	{
		return OriginalFunction(hinst, dwVersion, riidltf, ppvOut, punkOuter);
	}
	return S_FALSE;
}