#include "pch.h"

#include "ImGui_Impl.h"

#include "Game/GameData.h"

#include "imgui/imgui.h"

#include "Utils/CallHook.h"

#include <chrono>

#ifdef IMGUI_USE_BGRA_PACKED_COLOR
#define IMGUI_COL_TO_DX9_ARGB(_COL)     (_COL)
#else
#define IMGUI_COL_TO_DX9_ARGB(_COL)     (((_COL) & 0xFF00FF00) | (((_COL) & 0xFF0000) >> 16) | (((_COL) & 0xFF) << 16))
#endif

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)

namespace
{
	struct ImGuiVertex
	{
		float    pos[3];
		D3DCOLOR col;
		float    uv[2];
	};

	struct ImGui_ImplRenderData
	{
		LPDIRECT3DDEVICE9 d3dDevice = nullptr;
		LPDIRECT3DVERTEXBUFFER9 vb = nullptr;
		LPDIRECT3DINDEXBUFFER9 ib = nullptr;
		LPDIRECT3DTEXTURE9 fontTexture = nullptr;
		int vertexBufferSize = 5000;
		int indexBufferSize = 10000;
	};
	ImGui_ImplRenderData rd_;

	struct ImGui_ImplPlatformData
	{
		HWND hwnd = nullptr;
		HWND mouseHwnd = nullptr;
		bool mouseTracked = false;;
		int mouseButtonsDown = 0;
		int64_t time = 0;
		int64_t ticksPerSecond = 0;
		ImGuiMouseCursor lastMouseCursor = 0;
		bool hasGamepad = false;
		bool wantUpdateHasGamepad = false;

	};
	ImGui_ImplPlatformData pd_;

	bool isInitialized_ = false;

	std::chrono::high_resolution_clock::time_point lastFrameTime;
}

namespace
{

void CreateDeviceObjects(LPDIRECT3DDEVICE9 device)
{
	ImGuiIO& io = ImGui::GetIO();
	uint8_t* pixels;
	int width;
	int height;
	int bpp;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bpp);

	device->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &rd_.fontTexture, nullptr); // TODO : Check ret
	D3DLOCKED_RECT texRect;
	rd_.fontTexture->LockRect(0, &texRect, nullptr, 0);
	for (int y = 0; y < height; ++y)
	{
		memcpy((uint8_t*)texRect.pBits + (size_t)texRect.Pitch * y, pixels + (size_t)width * bpp * y, (size_t)width * bpp);
	}
	rd_.fontTexture->UnlockRect(0);

	io.Fonts->SetTexID((ImTextureID)rd_.fontTexture);

}

void UpdateMouseCursor()
{
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
	{
		return;
	}

	ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
	if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
	{
		// Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
		::SetCursor(NULL);
	}
	else
	{
		// Show OS mouse cursor
		LPTSTR win32Cursor = IDC_ARROW;
		switch (imgui_cursor)
		{
			case ImGuiMouseCursor_Arrow:        win32Cursor = IDC_ARROW; break;
			case ImGuiMouseCursor_TextInput:    win32Cursor = IDC_IBEAM; break;
			case ImGuiMouseCursor_ResizeAll:    win32Cursor = IDC_SIZEALL; break;
			case ImGuiMouseCursor_ResizeEW:     win32Cursor = IDC_SIZEWE; break;
			case ImGuiMouseCursor_ResizeNS:     win32Cursor = IDC_SIZENS; break;
			case ImGuiMouseCursor_ResizeNESW:   win32Cursor = IDC_SIZENESW; break;
			case ImGuiMouseCursor_ResizeNWSE:   win32Cursor = IDC_SIZENWSE; break;
			case ImGuiMouseCursor_Hand:         win32Cursor = IDC_HAND; break;
			case ImGuiMouseCursor_NotAllowed:   win32Cursor = IDC_NO; break;
		}
		::SetCursor(::LoadCursor(NULL, win32Cursor));
	}
	return;
}

bool IsVkDown(int vk)
{
	return (::GetKeyState(vk) & 0x8000) != 0;
}

/*void AddKeyEvent(ImGuiKey key, bool down, int native_keycode, int native_scancode = -1)
{
	ImGuiIO& io = ImGui::GetIO();
	io.AddKeyEvent(key, down);
	io.SetKeyEventNativeData(key, native_keycode, native_scancode); // To support legacy indexing (<1.87 user code)
	IM_UNUSED(native_scancode);
}*/

/*void ProcessKeyEventsWorkarounds()
{
	// Left & right Shift keys: when both are pressed together, Windows tend to not generate the WM_KEYUP event for the first released one.
	if (ImGui::IsKeyDown(ImGuiKey_LeftShift) && !IsVkDown(VK_LSHIFT))
		AddKeyEvent(ImGuiKey_LeftShift, false, VK_LSHIFT);
	if (ImGui::IsKeyDown(ImGuiKey_RightShift) && !IsVkDown(VK_RSHIFT))
		AddKeyEvent(ImGuiKey_RightShift, false, VK_RSHIFT);

	// Sometimes WM_KEYUP for Win key is not passed down to the app (e.g. for Win+V on some setups, according to GLFW).
	if (ImGui::IsKeyDown(ImGuiKey_LeftSuper) && !IsVkDown(VK_LWIN))
		AddKeyEvent(ImGuiKey_LeftSuper, false, VK_LWIN);
	if (ImGui::IsKeyDown(ImGuiKey_RightSuper) && !IsVkDown(VK_RWIN))
		AddKeyEvent(ImGuiKey_RightSuper, false, VK_RWIN);
}*/

/*void UpdateKeyModifiers()
{
	ImGuiIO& io = ImGui::GetIO();
	io.AddKeyEvent(ImGuiKey_ModCtrl, IsVkDown(VK_CONTROL));
	io.AddKeyEvent(ImGuiKey_ModShift, IsVkDown(VK_SHIFT));
	io.AddKeyEvent(ImGuiKey_ModAlt, IsVkDown(VK_MENU));
	io.AddKeyEvent(ImGuiKey_ModSuper, IsVkDown(VK_APPS));
}*/

void UpdateMouseData()
{
	ImGuiIO& io = ImGui::GetIO();

	const bool is_app_focused = (::GetForegroundWindow() == pd_.hwnd);
	if (is_app_focused)
	{
		// (Optional) Set OS mouse position from Dear ImGui if requested (rarely used, only when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
		if (io.WantSetMousePos)
		{
			POINT pos = { (int)io.MousePos.x, (int)io.MousePos.y };
			if (::ClientToScreen(pd_.hwnd, &pos))
				::SetCursorPos(pos.x, pos.y);
		}

		// (Optional) Fallback to provide mouse position when focused (WM_MOUSEMOVE already provides this when hovered or captured)
		if (!io.WantSetMousePos && !pd_.mouseTracked)
		{
			/*POINT pos;
			if (::GetCursorPos(&pos) && ::ScreenToClient(pd_.hwnd, &pos))
				io.AddMousePosEvent((float)pos.x, (float)pos.y);*/
			
			POINT pos;
			if (::GetCursorPos(&pos) && ::ScreenToClient(pd_.hwnd, &pos))
			{
				io.MousePos.x = static_cast<float>(pos.x);
				io.MousePos.y = static_cast<float>(pos.y);
			}
		}

		
		io.MouseDown[0] = IsVkDown(VK_LBUTTON);
		io.MouseDown[1] = IsVkDown(VK_RBUTTON);
		io.MouseDown[2] = IsVkDown(VK_MBUTTON);
	}
}

// There is no distinct VK_xxx for keypad enter, instead it is VK_RETURN + KF_EXTENDED, we assign it an arbitrary value to make code more readable (VK_ codes go up to 255)
#define IM_VK_KEYPAD_ENTER      (VK_RETURN + 256)

// Map VK_xxx to ImGuiKey_xxx.
/*ImGuiKey VirtualKeyToImGuiKey(WPARAM wParam)
{
	switch (wParam)
	{
		case VK_TAB: return ImGuiKey_Tab;
		case VK_LEFT: return ImGuiKey_LeftArrow;
		case VK_RIGHT: return ImGuiKey_RightArrow;
		case VK_UP: return ImGuiKey_UpArrow;
		case VK_DOWN: return ImGuiKey_DownArrow;
		case VK_PRIOR: return ImGuiKey_PageUp;
		case VK_NEXT: return ImGuiKey_PageDown;
		case VK_HOME: return ImGuiKey_Home;
		case VK_END: return ImGuiKey_End;
		case VK_INSERT: return ImGuiKey_Insert;
		case VK_DELETE: return ImGuiKey_Delete;
		case VK_BACK: return ImGuiKey_Backspace;
		case VK_SPACE: return ImGuiKey_Space;
		case VK_RETURN: return ImGuiKey_Enter;
		case VK_ESCAPE: return ImGuiKey_Escape;
		case VK_OEM_7: return ImGuiKey_Apostrophe;
		case VK_OEM_COMMA: return ImGuiKey_Comma;
		case VK_OEM_MINUS: return ImGuiKey_Minus;
		case VK_OEM_PERIOD: return ImGuiKey_Period;
		case VK_OEM_2: return ImGuiKey_Slash;
		case VK_OEM_1: return ImGuiKey_Semicolon;
		case VK_OEM_PLUS: return ImGuiKey_Equal;
		case VK_OEM_4: return ImGuiKey_LeftBracket;
		case VK_OEM_5: return ImGuiKey_Backslash;
		case VK_OEM_6: return ImGuiKey_RightBracket;
		case VK_OEM_3: return ImGuiKey_GraveAccent;
		case VK_CAPITAL: return ImGuiKey_CapsLock;
		case VK_SCROLL: return ImGuiKey_ScrollLock;
		case VK_NUMLOCK: return ImGuiKey_NumLock;
		case VK_SNAPSHOT: return ImGuiKey_PrintScreen;
		case VK_PAUSE: return ImGuiKey_Pause;
		case VK_NUMPAD0: return ImGuiKey_Keypad0;
		case VK_NUMPAD1: return ImGuiKey_Keypad1;
		case VK_NUMPAD2: return ImGuiKey_Keypad2;
		case VK_NUMPAD3: return ImGuiKey_Keypad3;
		case VK_NUMPAD4: return ImGuiKey_Keypad4;
		case VK_NUMPAD5: return ImGuiKey_Keypad5;
		case VK_NUMPAD6: return ImGuiKey_Keypad6;
		case VK_NUMPAD7: return ImGuiKey_Keypad7;
		case VK_NUMPAD8: return ImGuiKey_Keypad8;
		case VK_NUMPAD9: return ImGuiKey_Keypad9;
		case VK_DECIMAL: return ImGuiKey_KeypadDecimal;
		case VK_DIVIDE: return ImGuiKey_KeypadDivide;
		case VK_MULTIPLY: return ImGuiKey_KeypadMultiply;
		case VK_SUBTRACT: return ImGuiKey_KeypadSubtract;
		case VK_ADD: return ImGuiKey_KeypadAdd;
		case IM_VK_KEYPAD_ENTER: return ImGuiKey_KeypadEnter;
		case VK_LSHIFT: return ImGuiKey_LeftShift;
		case VK_LCONTROL: return ImGuiKey_LeftCtrl;
		case VK_LMENU: return ImGuiKey_LeftAlt;
		case VK_LWIN: return ImGuiKey_LeftSuper;
		case VK_RSHIFT: return ImGuiKey_RightShift;
		case VK_RCONTROL: return ImGuiKey_RightCtrl;
		case VK_RMENU: return ImGuiKey_RightAlt;
		case VK_RWIN: return ImGuiKey_RightSuper;
		case VK_APPS: return ImGuiKey_Menu;
		case '0': return ImGuiKey_0;
		case '1': return ImGuiKey_1;
		case '2': return ImGuiKey_2;
		case '3': return ImGuiKey_3;
		case '4': return ImGuiKey_4;
		case '5': return ImGuiKey_5;
		case '6': return ImGuiKey_6;
		case '7': return ImGuiKey_7;
		case '8': return ImGuiKey_8;
		case '9': return ImGuiKey_9;
		case 'A': return ImGuiKey_A;
		case 'B': return ImGuiKey_B;
		case 'C': return ImGuiKey_C;
		case 'D': return ImGuiKey_D;
		case 'E': return ImGuiKey_E;
		case 'F': return ImGuiKey_F;
		case 'G': return ImGuiKey_G;
		case 'H': return ImGuiKey_H;
		case 'I': return ImGuiKey_I;
		case 'J': return ImGuiKey_J;
		case 'K': return ImGuiKey_K;
		case 'L': return ImGuiKey_L;
		case 'M': return ImGuiKey_M;
		case 'N': return ImGuiKey_N;
		case 'O': return ImGuiKey_O;
		case 'P': return ImGuiKey_P;
		case 'Q': return ImGuiKey_Q;
		case 'R': return ImGuiKey_R;
		case 'S': return ImGuiKey_S;
		case 'T': return ImGuiKey_T;
		case 'U': return ImGuiKey_U;
		case 'V': return ImGuiKey_V;
		case 'W': return ImGuiKey_W;
		case 'X': return ImGuiKey_X;
		case 'Y': return ImGuiKey_Y;
		case 'Z': return ImGuiKey_Z;
		case VK_F1: return ImGuiKey_F1;
		case VK_F2: return ImGuiKey_F2;
		case VK_F3: return ImGuiKey_F3;
		case VK_F4: return ImGuiKey_F4;
		case VK_F5: return ImGuiKey_F5;
		case VK_F6: return ImGuiKey_F6;
		case VK_F7: return ImGuiKey_F7;
		case VK_F8: return ImGuiKey_F8;
		case VK_F9: return ImGuiKey_F9;
		case VK_F10: return ImGuiKey_F10;
		case VK_F11: return ImGuiKey_F11;
		case VK_F12: return ImGuiKey_F12;
		default: return ImGuiKey_None;
	}
}*/

void UpdateKeyBoard()
{
	ImGuiIO& io = ImGui::GetIO();
	for (WPARAM key = VK_BACK; key < VK_F12; ++key)
	{
		/*ImGuiKey imKey = VirtualKeyToImGuiKey(key);
		if (imKey != ImGuiKey_None)*/
		{
			//io.KeysDown[key] = IsVkDown(key);
		}
	}
}

void SetupRenderState(ImDrawData* draw_data)
{
	// Setup viewport
	D3DVIEWPORT9 vp;
	vp.X = vp.Y = 0;
	vp.Width = (DWORD)draw_data->DisplaySize.x;
	vp.Height = (DWORD)draw_data->DisplaySize.y;
	vp.MinZ = 0.0f;
	vp.MaxZ = 1.0f;
	rd_.d3dDevice->SetViewport(&vp);

	// Setup render state: fixed-pipeline, alpha-blending, no face culling, no depth testing, shade mode (for gradient), bilinear sampling.
	rd_.d3dDevice->SetPixelShader(NULL);
	rd_.d3dDevice->SetVertexShader(NULL);
	rd_.d3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	rd_.d3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	rd_.d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	rd_.d3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	rd_.d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	rd_.d3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	rd_.d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	rd_.d3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	rd_.d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	rd_.d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	rd_.d3dDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
	rd_.d3dDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
	rd_.d3dDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA);
	rd_.d3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
	rd_.d3dDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
	rd_.d3dDevice->SetRenderState(D3DRS_RANGEFOGENABLE, FALSE);
	rd_.d3dDevice->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
	rd_.d3dDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);
	rd_.d3dDevice->SetRenderState(D3DRS_CLIPPING, TRUE);
	rd_.d3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	rd_.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	rd_.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	rd_.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	rd_.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	rd_.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	rd_.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	rd_.d3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	rd_.d3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	rd_.d3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	rd_.d3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	// Setup orthographic projection matrix
	// Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
	// Being agnostic of whether <d3dx9.h> or <DirectXMath.h> can be used, we aren't relying on D3DXMatrixIdentity()/D3DXMatrixOrthoOffCenterLH() or DirectX::XMMatrixIdentity()/DirectX::XMMatrixOrthographicOffCenterLH()
	{
		float L = draw_data->DisplayPos.x + 0.5f;
		float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x + 0.5f;
		float T = draw_data->DisplayPos.y + 0.5f;
		float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y + 0.5f;
		D3DMATRIX mat_identity = { { { 1.0f, 0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 0.0f, 1.0f } } };
		D3DMATRIX mat_projection =
		{ { {
			2.0f / (R - L),   0.0f,         0.0f,  0.0f,
			0.0f,         2.0f / (T - B),   0.0f,  0.0f,
			0.0f,         0.0f,         0.5f,  0.0f,
			(L + R) / (L - R),  (T + B) / (B - T),  0.5f,  1.0f
		} } };
		rd_.d3dDevice->SetTransform(D3DTS_WORLD, &mat_identity);
		rd_.d3dDevice->SetTransform(D3DTS_VIEW, &mat_identity);
		rd_.d3dDevice->SetTransform(D3DTS_PROJECTION, &mat_projection);
	}
}

}

void ImGui_Impl::Init()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();

	ImGui::StyleColorsDark();

	// Render stuff
	io.BackendRendererUserData = &rd_;
	io.BackendRendererName = "bhdDX9";
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
	io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

	rd_.d3dDevice = g_gpd.d3dDevice;
	rd_.d3dDevice->AddRef();

	CreateDeviceObjects(g_gpd.d3dDevice);

	// Platform stuff
	int64_t perfFreq;
	int64_t perfCounter;
	QueryPerformanceFrequency((LARGE_INTEGER*)&perfFreq);
	QueryPerformanceCounter((LARGE_INTEGER*)&perfCounter);

	io.BackendPlatformUserData = &pd_;
	io.BackendPlatformName = "bhd";
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

	pd_.hwnd = g_gpd.hwnd;
	pd_.wantUpdateHasGamepad = false;
	pd_.time = perfCounter;
	pd_.lastMouseCursor = ImGuiMouseCursor_COUNT;

	io.KeyMap[ImGuiKey_Tab] = VK_TAB;
	io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
	io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
	io.KeyMap[ImGuiKey_PageUp] = VK_NEXT;
	io.KeyMap[ImGuiKey_PageDown] = VK_PRIOR;
	io.KeyMap[ImGuiKey_Home] = VK_HOME;
	io.KeyMap[ImGuiKey_End] = VK_END;
	io.KeyMap[ImGuiKey_Insert] = VK_INSERT;
	io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
	io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
	io.KeyMap[ImGuiKey_Space] = VK_SPACE;
	io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
	io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
	io.KeyMap[ImGuiKey_A] = 'a';
	io.KeyMap[ImGuiKey_C] = 'c';
	io.KeyMap[ImGuiKey_V] = 'v';
	io.KeyMap[ImGuiKey_X] = 'x';
	io.KeyMap[ImGuiKey_Y] = 'y';
	io.KeyMap[ImGuiKey_Z] = 'z';

	//ImGui::GetMainViewport()->PlatformHandleRaw = g_gpd.hwnd;

	//wndProcHook_.Set((char*)0x00859b00, (char*)&hk_bhd_WndProc, 8);
	//wndProcHook_.Apply();

	isInitialized_ = true;
}

void ImGui_Impl::Shutdown()
{
	if (!isInitialized_)
	{
		return;
	}

	ImGui::DestroyContext();

	rd_.d3dDevice->Release();
}

bool ImGui_Impl::IsInitialized()
{
	return isInitialized_;
}

void ImGui_Impl::ProcessEvent(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_Impl::IsInitialized())
	{
		ImGuiIO& io = ImGui::GetIO();

		switch (message)
		{
			case WM_KEYDOWN:
				if (wParam < 256)
					io.KeysDown[wParam] = true;
				break;
			case WM_KEYUP:
				if (wParam < 256)
					io.KeysDown[wParam] = false;
				break;
			case WM_CHAR:
				if (wParam > 0 && wParam < 0x10000)
					io.AddInputCharacterUTF16((unsigned short)wParam);
				break;
		}
	}
}

void ImGui_Impl::NewFrame()
{
	ImGuiIO& io = ImGui::GetIO();
	const std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
	const auto deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastFrameTime);
	lastFrameTime = currentTime;
	io.DeltaTime = deltaTime.count() / 1000.0f;

	// Setup display size (every frame to accommodate for window resizing)
	RECT rect = { 0, 0, 0, 0 };
	::GetClientRect(pd_.hwnd, &rect);
	io.DisplaySize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));

	// Setup time step
	/*INT64 currentTime = 0;
	::QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
	io.DeltaTime = (float)(currentTime - pd_.time) / pd_.ticksPerSecond;
	pd_.time = currentTime;*/

	// Update OS mouse position
	UpdateMouseData();

	// Process workarounds for known Windows key handling issues
	//ProcessKeyEventsWorkarounds();

	UpdateKeyBoard();

	// Update OS mouse cursor with the cursor requested by imgui
	ImGuiMouseCursor mouse_cursor = io.MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor();
	if (pd_.lastMouseCursor != mouse_cursor)
	{
		pd_.lastMouseCursor = mouse_cursor;
		UpdateMouseCursor();
	}

	ImGui::NewFrame();
}

void ImGui_Impl::EndFrame()
{
	ImGui::EndFrame();
}

void ImGui_Impl::Render()
{
	ImDrawData* drawData = ImGui::GetDrawData();

	// Avoid rendering when minimized
	if (drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f)
		return;

	// Create and grow buffers if needed
	if (!rd_.vb || rd_.vertexBufferSize < drawData->TotalVtxCount)
	{
		if (rd_.vb) { rd_.vb->Release(); rd_.vb = NULL; }
		rd_.vertexBufferSize = drawData->TotalVtxCount + 5000;
		if (rd_.d3dDevice->CreateVertexBuffer(rd_.vertexBufferSize * sizeof(ImGuiVertex), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &rd_.vb, NULL) < 0)
			return;
	}
	if (!rd_.ib || rd_.indexBufferSize < drawData->TotalIdxCount)
	{
		if (rd_.ib) { rd_.ib->Release(); rd_.ib = NULL; }
		rd_.indexBufferSize = drawData->TotalIdxCount + 10000;
		if (rd_.d3dDevice->CreateIndexBuffer(rd_.indexBufferSize * sizeof(ImDrawIdx), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, sizeof(ImDrawIdx) == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32, D3DPOOL_DEFAULT, &rd_.ib, NULL) < 0)
			return;
	}

	// Backup the DX9 state
	IDirect3DStateBlock9* d3d9_state_block = NULL;
	if (rd_.d3dDevice->CreateStateBlock(D3DSBT_ALL, &d3d9_state_block) < 0)
		return;
	if (d3d9_state_block->Capture() < 0)
	{
		d3d9_state_block->Release();
		return;
	}

	// Backup the DX9 transform (DX9 documentation suggests that it is included in the StateBlock but it doesn't appear to)
	D3DMATRIX last_world, last_view, last_projection;
	rd_.d3dDevice->GetTransform(D3DTS_WORLD, &last_world);
	rd_.d3dDevice->GetTransform(D3DTS_VIEW, &last_view);
	rd_.d3dDevice->GetTransform(D3DTS_PROJECTION, &last_projection);

	// Allocate buffers
	ImGuiVertex* vtx_dst;
	ImDrawIdx* idx_dst;
	if (rd_.vb->Lock(0, (UINT)(drawData->TotalVtxCount * sizeof(ImGuiVertex)), (void**)&vtx_dst, D3DLOCK_DISCARD) < 0)
	{
		d3d9_state_block->Release();
		return;
	}
	if (rd_.ib->Lock(0, (UINT)(drawData->TotalIdxCount * sizeof(ImDrawIdx)), (void**)&idx_dst, D3DLOCK_DISCARD) < 0)
	{
		rd_.vb->Unlock();
		d3d9_state_block->Release();
		return;
	}

	// Copy and convert all vertices into a single contiguous buffer, convert colors to DX9 default format.
	// FIXME-OPT: This is a minor waste of resource, the ideal is to use imconfig.h and
	//  1) to avoid repacking colors:   #define IMGUI_USE_BGRA_PACKED_COLOR
	//  2) to avoid repacking vertices: #define IMGUI_OVERRIDE_DRAWVERT_STRUCT_LAYOUT struct ImDrawVert { ImVec2 pos; float z; ImU32 col; ImVec2 uv; }
	for (int n = 0; n < drawData->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = drawData->CmdLists[n];
		const ImDrawVert* vtx_src = cmd_list->VtxBuffer.Data;
		for (int i = 0; i < cmd_list->VtxBuffer.Size; i++)
		{
			vtx_dst->pos[0] = vtx_src->pos.x;
			vtx_dst->pos[1] = vtx_src->pos.y;
			vtx_dst->pos[2] = 0.0f;
			vtx_dst->col = IMGUI_COL_TO_DX9_ARGB(vtx_src->col);
			vtx_dst->uv[0] = vtx_src->uv.x;
			vtx_dst->uv[1] = vtx_src->uv.y;
			vtx_dst++;
			vtx_src++;
		}
		memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
		idx_dst += cmd_list->IdxBuffer.Size;
	}
	rd_.vb->Unlock();
	rd_.ib->Unlock();
	rd_.d3dDevice->SetStreamSource(0, rd_.vb, 0, sizeof(ImGuiVertex));
	rd_.d3dDevice->SetIndices(rd_.ib);
	rd_.d3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);

	// Setup desired DX state
	SetupRenderState(drawData);

	// Render command lists
	// (Because we merged all buffers into a single one, we maintain our own offset into them)
	int global_vtx_offset = 0;
	int global_idx_offset = 0;
	ImVec2 clip_off = drawData->DisplayPos;
	for (int n = 0; n < drawData->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = drawData->CmdLists[n];
		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback != NULL)
			{
				// User callback, registered via ImDrawList::AddCallback()
				// (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
				if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
					SetupRenderState(drawData);
				else
					pcmd->UserCallback(cmd_list, pcmd);
			}
			else
			{
				// Project scissor/clipping rectangles into framebuffer space
				ImVec2 clip_min(pcmd->ClipRect.x - clip_off.x, pcmd->ClipRect.y - clip_off.y);
				ImVec2 clip_max(pcmd->ClipRect.z - clip_off.x, pcmd->ClipRect.w - clip_off.y);
				if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
					continue;

				// Apply Scissor/clipping rectangle, Bind texture, Draw
				const RECT r = { (LONG)clip_min.x, (LONG)clip_min.y, (LONG)clip_max.x, (LONG)clip_max.y };
				const LPDIRECT3DTEXTURE9 texture = (LPDIRECT3DTEXTURE9)pcmd->GetTexID();
				rd_.d3dDevice->SetTexture(0, texture);
				rd_.d3dDevice->SetScissorRect(&r);
				rd_.d3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, pcmd->VtxOffset + global_vtx_offset, 0, (UINT)cmd_list->VtxBuffer.Size, pcmd->IdxOffset + global_idx_offset, pcmd->ElemCount / 3);
			}
		}
		global_idx_offset += cmd_list->IdxBuffer.Size;
		global_vtx_offset += cmd_list->VtxBuffer.Size;
	}

	// Restore the DX9 transform
	rd_.d3dDevice->SetTransform(D3DTS_WORLD, &last_world);
	rd_.d3dDevice->SetTransform(D3DTS_VIEW, &last_view);
	rd_.d3dDevice->SetTransform(D3DTS_PROJECTION, &last_projection);

	// Restore the DX9 state
	d3d9_state_block->Apply();
	d3d9_state_block->Release();
}