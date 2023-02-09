﻿#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <d3d11.h>
#include <assert.h>

#define IDS_APP_TITLE	    103
#define IDC_LAB_1			109

// Globals
// For initialization and utility
ID3D11Device* g_Device;
IDXGISwapChain* g_Swapchain;
ID3D11DeviceContext* g_DeviceContext;
float g_aspectRatio;

// For drawing
ID3D11RenderTargetView* g_RenderTargetView;
D3D11_VIEWPORT g_viewport;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void Redraw() {
	// Rendering Code
	if (g_DeviceContext && g_Swapchain && g_RenderTargetView) {
		ID3D11RenderTargetView* tempRTV[] = { g_RenderTargetView };
		g_DeviceContext->OMSetRenderTargets(1, tempRTV, nullptr);

		float color[4] = { 1, 0, 0, 1 };
		g_DeviceContext->ClearRenderTargetView(g_RenderTargetView, color);

		g_DeviceContext->RSSetViewports(1, &g_viewport);

		g_Swapchain->Present(0, 0);
	}
	// Rendering Code
}



int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);


	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_LAB_1, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}
	HRESULT result;
	IDXGIFactory* pFactory = nullptr;
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory);
	// Select hardware adapter 
	IDXGIAdapter* pSelectedAdapter = NULL;
	if (!FAILED(result))
	{
		IDXGIAdapter* pAdapter = NULL;
		UINT adapterIdx = 0;

		while (SUCCEEDED(pFactory->EnumAdapters(adapterIdx, &pAdapter)))
		{
			DXGI_ADAPTER_DESC desc;
			pAdapter->GetDesc(&desc);
			if (wcscmp(desc.Description, L"Microsoft Basic Render Driver") != 0) {
				pSelectedAdapter = pAdapter;
				break;
			}
			pAdapter->Release();
			adapterIdx++;

		}
	}
	assert(pSelectedAdapter != NULL);

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LAB_1));

	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	Redraw();
	// Main message loop:
	while (true)
	{
		PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
			break;

		// Break if user presses escape key
		if (GetAsyncKeyState(VK_ESCAPE)) break;

	}
	g_Device->Release();
	g_DeviceContext->Release();
	g_RenderTargetView->Release();
	g_Swapchain->Release();

	return (int)msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, NULL);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_LAB_1);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, NULL);

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}
	{

		RECT rc;
		rc.left = 0;
		rc.right = 640;
		rc.top = 0;
		rc.bottom = 360; 
		AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, TRUE);
		MoveWindow(hWnd, 100, 100, rc.right - rc.left, rc.bottom - rc.top, TRUE);

	}
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// D3d11 code here
	RECT rect;
	GetClientRect(hWnd, &rect);

	// Attach d3d to the window
	D3D_FEATURE_LEVEL DX11 = D3D_FEATURE_LEVEL_11_0;
	DXGI_SWAP_CHAIN_DESC swap;
	ZeroMemory(&swap, sizeof(DXGI_SWAP_CHAIN_DESC));
	swap.BufferCount = 1;
	swap.OutputWindow = hWnd;
	swap.Windowed = true;
	swap.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swap.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap.BufferDesc.Width = rect.right - rect.left;
	swap.BufferDesc.Height = rect.bottom - rect.top;
	swap.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap.SampleDesc.Count = 1;

	g_aspectRatio = swap.BufferDesc.Width / (float)swap.BufferDesc.Height;

	HRESULT result;

	result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, &DX11, 1, D3D11_SDK_VERSION, &swap, &g_Swapchain, &g_Device, 0, &g_DeviceContext);
	assert(!FAILED(result));

	ID3D11Resource* backbuffer;
	result = g_Swapchain->GetBuffer(0, __uuidof(backbuffer), (void**)&backbuffer);
	assert(!FAILED(result));
	result = g_Device->CreateRenderTargetView(backbuffer, NULL, &g_RenderTargetView);
	assert(!FAILED(result));

	// Release the resource to decrement the counter by one
	// This is necessary to keep the thing from leaking memory
	backbuffer->Release();

	// Setup viewport
	g_viewport.Width = swap.BufferDesc.Width;
	g_viewport.Height = swap.BufferDesc.Height;
	g_viewport.TopLeftY = g_viewport.TopLeftX = 0;
	g_viewport.MinDepth = 0;
	g_viewport.MaxDepth = 1;
	// D3d11 code here

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{

	case WM_DESTROY:											   
		PostQuitMessage(0);
		break;
	case WM_SIZE:
	case WM_PAINT:
		Redraw();
		return DefWindowProc(hWnd, message, wParam, lParam);
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
