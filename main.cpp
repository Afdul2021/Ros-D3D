/*
* Ros D3D 0.8c by n7

How to compile:
- compile with visual studio community 2017 (..\Microsoft Visual Studio\2017\Community\Common7\IDE\devenv.exe)
- select Release x86
- click: project -> properties -> configuration properties -> general -> character set -> change to "not set"
- compile with CTRL + Shift + B

Optional: remove dependecy on vs runtime:
- click: project -> properties -> configuration properties -> C/C++ -> code generation -> runtime library: Multi-threaded (/MT)
*/

#include "main.h" //less important stuff & helper funcs here

typedef HRESULT(APIENTRY *SetStreamSource_t)(IDirect3DDevice9*, UINT, IDirect3DVertexBuffer9*, UINT, UINT);
HRESULT APIENTRY SetStreamSource_hook(IDirect3DDevice9*, UINT, IDirect3DVertexBuffer9*, UINT, UINT);
SetStreamSource_t SetStreamSource_orig = 0;

typedef HRESULT(APIENTRY *SetTexture_t)(IDirect3DDevice9*, DWORD, IDirect3DBaseTexture9 *);
HRESULT APIENTRY SetTexture_hook(IDirect3DDevice9*, DWORD, IDirect3DBaseTexture9 *);
SetTexture_t SetTexture_orig = 0;

typedef HRESULT(APIENTRY* Present_t) (IDirect3DDevice9*, const RECT *, const RECT *, HWND, const RGNDATA *);
HRESULT APIENTRY Present_hook(IDirect3DDevice9*, const RECT *, const RECT *, HWND, const RGNDATA *);
Present_t Present_orig = 0;

typedef HRESULT(APIENTRY* EndScene_t) (IDirect3DDevice9*);
HRESULT APIENTRY EndScene_hook(IDirect3DDevice9*);
EndScene_t EndScene_orig = 0;

typedef HRESULT(APIENTRY *Reset_t)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
HRESULT APIENTRY Reset_hook(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
Reset_t Reset_orig = 0;

//==========================================================================================================================

HRESULT APIENTRY SetStreamSource_hook(LPDIRECT3DDEVICE9 pDevice, UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT sStride)
{
	if (StreamNumber == 0)
		Stride = sStride;

	return SetStreamSource_orig(pDevice, StreamNumber, pStreamData, OffsetInBytes, sStride);
}

//==========================================================================================================================

HRESULT APIENTRY SetTexture_hook(LPDIRECT3DDEVICE9 pDevice, DWORD Sampler, IDirect3DBaseTexture9 *pTexture)
{
	//Texture = pTexture;

	if (InitOnce)
	{
		InitOnce = false;
		GenerateTexture(pDevice, &Red, D3DCOLOR_ARGB(255, 255, 0, 0));
		GenerateTexture(pDevice, &Green, D3DCOLOR_RGBA(0, 255, 0, 255));
		GenerateTexture(pDevice, &Blue, D3DCOLOR_ARGB(255, 0, 0, 255));
		GenerateTexture(pDevice, &Yellow, D3DCOLOR_ARGB(255, 255, 255, 0));

		LoadCfg();
	}

	//get pSize
	//if (SUCCEEDED(pDevice->GetPixelShader(&pShader)))
		//if (pShader != NULL)
			//if (SUCCEEDED(pShader->GetFunction(NULL, &pSize)))
				//if (pShader != NULL) { pShader->Release(); pShader = NULL; }

	//get vSize
	if (SUCCEEDED(pDevice->GetVertexShader(&vShader)))
		if (vShader != NULL)
			if (SUCCEEDED(vShader->GetFunction(NULL, &vSize)))
				if (vShader != NULL) { vShader->Release(); vShader = NULL; }

	//model rec sample
	//Stride == 36 && pSize == 1724 && vSize == 1952 //legs
	//Stride == 36 && pSize == 1848 && vSize == 2300 //chest
	//Stride == 44 && pSize == 2272 && vSize == 2300 //hair

	if (wallhack>0)
	{
		pDevice->SetRenderState(D3DRS_DEPTHBIAS, 0);
		if (vSize == 2300 || vSize == 900 ||
			vSize == 1952 || vSize == 640)//vSize == 1436
		{
			if (wallhack == 2)
			{
				float sColor[4] = { 0.0f, 1.0f, 0.0f, 1.0f };//green
				pDevice->SetPixelShaderConstantF(0, sColor, 1);
				//SetTexture_orig(pDevice, 0, Red);
				//SetTexture_orig(pDevice, 1, Red);
			}

			float bias = 1000.0f;
			float bias_float = static_cast<float>(-bias);
			bias_float /= 10000.0f;
			pDevice->SetRenderState(D3DRS_DEPTHBIAS, *(DWORD*)&bias_float);
		}
	}

	//worldtoscreen weapons in hand
	if (aimbot == 1 || esp > 0)
	{
		if (Stride == 48 || vSize == 2300 || vSize == 900 ||
			vSize == 1952 || vSize == 640)
			AddWeapons(pDevice);
	}
	
	//no grass by mtsagossi
	if (nograss == 1)
	{
		pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		if (vSize == 1660 || vSize == 1704)
		{
			pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_POINT);
		}
	}

	//logger
	//if (GetAsyncKeyState('O') & 1) //-
	//countnum--;
	//if (GetAsyncKeyState('P') & 1) //+
	//countnum++;
	//if (countnum == pSize / 100|| countnum == vSize / 100)
	//if (GetAsyncKeyState('I') & 1) //log
	//Log("Stride == %d && pSize == %d && vSize == %d", Stride, pSize, vSize);
	//if (countnum == pSize / 100 || countnum == vSize / 100)
	//return D3D_OK; //delete texture
	//pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_POINT);

	return SetTexture_orig(pDevice, Sampler, pTexture);
}

//==========================================================================================================================

HRESULT APIENTRY Present_hook(IDirect3DDevice9* pDevice, const RECT *pSourceRect, const RECT *pDestRect, HWND hDestWindowOverride, const RGNDATA *pDirtyRegion)
{
	//get viewport
	pDevice->GetViewport(&Viewport);
	ScreenCX = (float)Viewport.Width / 2.0f;
	ScreenCY = (float)Viewport.Height / 2.0f;

	//create font
	if (Font == NULL)
		D3DXCreateFont(pDevice, 14, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Italic"), &Font);

	if (ShowMenu)
		//draw background
		DrawBox2(pDevice, 71.0f, 86.0f, 200.0f, 160.0f, D3DCOLOR_ARGB(120, 30, 200, 200));//180 = up/down, 200 = left/right

	if (Font)
		DrawMenu(pDevice);

	if (screenshot_taken)
	{
		DrawString(Font, ScreenCX-20.0f, ScreenCY-20.0f, D3DCOLOR_ARGB(255, 255, 255, 255), "Screenshot Taken (gm_complaint)");

		if (timeGetTime() - screen_pause >= 799)
		{
			screenshot_taken = false;
			screen_pause = timeGetTime();
		}
	}

	//Shift|RMouse|LMouse|Ctrl|Alt|Space|X|C
	if (aimkey == 0) Daimkey = 0;
	if (aimkey == 1) Daimkey = VK_SHIFT;
	if (aimkey == 2) Daimkey = VK_RBUTTON;
	if (aimkey == 3) Daimkey = VK_LBUTTON;
	if (aimkey == 4) Daimkey = VK_CONTROL;
	if (aimkey == 5) Daimkey = VK_MENU;
	if (aimkey == 6) Daimkey = VK_SPACE;
	if (aimkey == 7) Daimkey = 0x58; //X
	if (aimkey == 8) Daimkey = 0x43; //C

	//do esp
	if (esp > 0 && WeaponEspInfo.size() != NULL)
	{
		for (unsigned int i = 0; i < WeaponEspInfo.size(); i++)
		{
			//box esp
			if (WeaponEspInfo[i].pOutX > 1.0f && WeaponEspInfo[i].pOutY > 1.0f && (float)WeaponEspInfo[i].distance > 40.0f && (float)WeaponEspInfo[i].distance < 1000.0f)
			DrawBox2(pDevice, (int)WeaponEspInfo[i].pOutX, (int)WeaponEspInfo[i].pOutY-20.0f, 12.0f, 12.0f, D3DCOLOR_ARGB(12, 30, 200, 200));//25, 18, 12
			if (WeaponEspInfo[i].pOutX > 1.0f && WeaponEspInfo[i].pOutY > 1.0f && (float)WeaponEspInfo[i].distance >= 1000.0f && (float)WeaponEspInfo[i].distance < 10000.0f)
			DrawBox2(pDevice, (int)WeaponEspInfo[i].pOutX, (int)WeaponEspInfo[i].pOutY - 20.0f, 18.0f, 12.0f, D3DCOLOR_ARGB(12, 30, 200, 200));//25, 18, 12
			if (WeaponEspInfo[i].pOutX > 1.0f && WeaponEspInfo[i].pOutY > 1.0f && (float)WeaponEspInfo[i].distance >= 10000.0f && (float)WeaponEspInfo[i].distance < 90000.0f)
			DrawBox2(pDevice, (int)WeaponEspInfo[i].pOutX, (int)WeaponEspInfo[i].pOutY - 20.0f, 24.0f, 12.0f, D3DCOLOR_ARGB(12, 30, 200, 200));//25, 18, 12

			//line esp
			if (WeaponEspInfo[i].pOutX > 1.0f && WeaponEspInfo[i].pOutY > 1.0f && (float)WeaponEspInfo[i].distance > 40.0f)
				DrawLine(pDevice, (int)WeaponEspInfo[i].pOutX, (int)WeaponEspInfo[i].pOutY, ScreenCX, ScreenCY * ((float)esp * 0.2f), 20, D3DCOLOR_ARGB(255, 255, 255, 255));//0.1up, 1.0middle, 2.0down

			//distance esp
			if (WeaponEspInfo[i].pOutX > 1.0f && WeaponEspInfo[i].pOutY > 1.0f && (float)WeaponEspInfo[i].distance > 2000.0f)
				DrawString(Font, (int)WeaponEspInfo[i].pOutX, (int)WeaponEspInfo[i].pOutY-20.0f, D3DCOLOR_ARGB(255, 255, 255, 255), "%.f", (float)WeaponEspInfo[i].distance / 10.0f);
			if (WeaponEspInfo[i].pOutX > 1.0f && WeaponEspInfo[i].pOutY > 1.0f && (float)WeaponEspInfo[i].distance > 40.0f && (float)WeaponEspInfo[i].distance <= 2000.0f)
				DrawString(Font, (int)WeaponEspInfo[i].pOutX, (int)WeaponEspInfo[i].pOutY - 20.0f, D3DCOLOR_ARGB(255, 255, 255, 0), "%.f", (float)WeaponEspInfo[i].distance / 10.0f);

			//text esp
			//if (WeaponEspInfo[i].pOutX > 1.0f && WeaponEspInfo[i].pOutY > 1.0f && (float)WeaponEspInfo[i].distance > 1.0f)
			//DrawString(Font, (int)WeaponEspInfo[i].pOutX, (int)WeaponEspInfo[i].pOutY, D3DCOLOR_ARGB(255, 255, 255, 255), "o");
		}
	}

	//do aim
	if (aimbot == 1 && WeaponEspInfo.size() != NULL)
	{
		UINT BestTarget = -1;
		DOUBLE fClosestPos = 99999;

		for (unsigned int i = 0; i < WeaponEspInfo.size(); i++)
		{
			//aimfov
			float radiusx = (aimfov*5.0f) * (ScreenCX / 100.0f);
			float radiusy = (aimfov*5.0f) * (ScreenCY / 100.0f);

			if (aimfov == 0)
			{
				radiusx = 5.0f * (ScreenCX / 100.0f);
				radiusy = 5.0f * (ScreenCY / 100.0f);
			}

			//get crosshairdistance
			WeaponEspInfo[i].CrosshairDistance = GetDistance(WeaponEspInfo[i].pOutX, WeaponEspInfo[i].pOutY, ScreenCX, ScreenCY);

			//if in fov
			if (WeaponEspInfo[i].pOutX >= ScreenCX - radiusx && WeaponEspInfo[i].pOutX <= ScreenCX + radiusx && WeaponEspInfo[i].pOutY >= ScreenCY - radiusy && WeaponEspInfo[i].pOutY <= ScreenCY + radiusy)

				//get closest/nearest target to crosshair
				if (WeaponEspInfo[i].CrosshairDistance < fClosestPos)
				{
					fClosestPos = WeaponEspInfo[i].CrosshairDistance;
					BestTarget = i;
				}
		}


		//if nearest target to crosshair
		if (BestTarget != -1 && WeaponEspInfo[BestTarget].distance > 40.0f)
		{
			double DistX = WeaponEspInfo[BestTarget].pOutX - ScreenCX;
			double DistY = WeaponEspInfo[BestTarget].pOutY - ScreenCY;

			DistX /= (0.5f + (float)aimsens*0.5f);
			DistY /= (0.5f + (float)aimsens*0.5f);

			//aim
			if (GetAsyncKeyState(Daimkey) & 0x8000)
				mouse_event(MOUSEEVENTF_MOVE, (float)DistX, (float)DistY, 0, NULL);

			//autoshoot on
			if ((!GetAsyncKeyState(VK_LBUTTON) && (autoshoot == 1) && (GetAsyncKeyState(Daimkey) & 0x8000))) //
			{
				if (autoshoot == 1 && !IsPressed)
				{
					IsPressed = true;
					mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
				}
			}
		}
	}
	WeaponEspInfo.clear();

	//autoshoot off
	if (autoshoot == 1 && IsPressed)
	{
		if (timeGetTime() - astime >= asdelay)
		{
			IsPressed = false;
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			astime = timeGetTime();
		}
	}

	/*
	//draw logger
	if (Font)
	{
	char szString[255];
	sprintf_s(szString, "countnum = %d", countnum);
	DrawString(Font, 219, 99, D3DCOLOR_ARGB(255, 0, 0, 0), (char*)&szString[0]);
	DrawString(Font, 221, 101, D3DCOLOR_ARGB(255, 0, 0, 0), (char*)&szString[0]);
	DrawString(Font, 220, 100, D3DCOLOR_ARGB(255, 255, 255, 255), (char*)&szString[0]);
	DrawString(Font, 220, 110, D3DCOLOR_ARGB(255, 255, 255, 255), "hold P to +");
	DrawString(Font, 220, 120, D3DCOLOR_ARGB(255, 255, 255, 255), "hold O to -");
	}
	*/
	return Present_orig(pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

//==========================================================================================================================

HRESULT APIENTRY EndScene_hook(IDirect3DDevice9* pDevice)
{
	if (Font)
		DrawMenu(pDevice);

	return EndScene_orig(pDevice);
}

//==========================================================================================================================

HRESULT APIENTRY Reset_hook(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	if (Font)
		Font->OnLostDevice();

	HRESULT ResetReturn = Reset_orig(pDevice, pPresentationParameters);

	if (SUCCEEDED(ResetReturn))
	{
		if (Font)
			Font->OnResetDevice();
	}

	return ResetReturn;
}

//==========================================================================================================================

#undef UNICODE
#include <windows.h>
#include "detours.h"
#include <fstream>
#include <string>
#include <direct.h>
#include <strsafe.h>

HANDLE(WINAPI * Real_CreateFile) (
	LPCWSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile) = CreateFileW;

HANDLE WINAPI Routed_CreateFile(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	char buffer[500];
	wcstombs(buffer, lpFileName, 500);
	if(strcmp(buffer + strlen(buffer) - 4, ".jpg") == 0)
	{
		if(screenshot_taken==false)
		{
			//wallhack = 0; //too late
			Log("buffer == %s", buffer);//find gm_complaint_x.jpg
			screenshot_taken = true;
		}
	}
	return Real_CreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

//==========================================================================================================================

DWORD WINAPI RosD3D(__in LPVOID lpParameter)
{
	HMODULE dDll = NULL;
	while (!dDll)
	{
		dDll = GetModuleHandleA("d3d9.dll");
		Sleep(100);
	}
	CloseHandle(dDll);

	IDirect3D9* d3d = NULL;
	IDirect3DDevice9* d3ddev = NULL;

	HWND tmpWnd = CreateWindowA("BUTTON", "RosD3D", WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 300, 300, NULL, NULL, Hand, NULL);
	if (tmpWnd == NULL)
	{
		//Log("[DirectX] Failed to create temp window");
		return 0;
	}

	d3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (d3d == NULL)
	{
		DestroyWindow(tmpWnd);
		//Log("[DirectX] Failed to create temp Direct3D interface");
		return 0;
	}

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = tmpWnd;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

	HRESULT result = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, tmpWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &d3ddev);
	if (result != D3D_OK)
	{
		d3d->Release();
		DestroyWindow(tmpWnd);
		//Log("[DirectX] Failed to create temp Direct3D device");
		return 0;
	}

	// We have the device, so walk the vtable to get the address of all the dx functions in d3d9.dll
#if defined _M_X64
	DWORD64* dVtable = (DWORD64*)d3ddev;
	dVtable = (DWORD64*)dVtable[0];
#elif defined _M_IX86
	DWORD* dVtable = (DWORD*)d3ddev;
	dVtable = (DWORD*)dVtable[0]; // == *d3ddev
#endif
	//Log("[DirectX] dVtable: %x", dVtable);

	//for(int i = 0; i < 95; i++)
	//{
			//Log("[DirectX] vtable[%i]: %x, pointer at %x", i, dVtable[i], &dVtable[i]);
	//}

	// Detour functions x86 & x64
	if (MH_Initialize() != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable[17], &Present_hook, reinterpret_cast<void**>(&Present_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable[17]) != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable[42], &EndScene_hook, reinterpret_cast<void**>(&EndScene_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable[42]) != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable[100], &SetStreamSource_hook, reinterpret_cast<void**>(&SetStreamSource_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable[100]) != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable[65], &SetTexture_hook, reinterpret_cast<void**>(&SetTexture_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable[65]) != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable[16], &Reset_hook, reinterpret_cast<void**>(&Reset_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable[16]) != MH_OK) { return 1; }

	HMODULE mod = LoadLibrary(TEXT("Kernel32.dll"));
	void* ptr = GetProcAddress(mod, "CreateFileW");
	MH_CreateHook(ptr, Routed_CreateFile, reinterpret_cast<void**>(&Real_CreateFile));
	MH_EnableHook(ptr);

	//Log("[Detours] Detours attached\n");

	d3ddev->Release();
	d3d->Release();
	DestroyWindow(tmpWnd);

	return 1;
}

//==========================================================================================================================

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		Hand = hModule;
		DisableThreadLibraryCalls(hModule); //disable unwanted thread notifications to reduce overhead
		CreateThread(0, 0, RosD3D, 0, 0, 0); //init our hooks

		break;
	case DLL_PROCESS_DETACH:
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}
