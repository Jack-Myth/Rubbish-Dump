#include "SDL2/SDL.h"
#include "SDL2/SDL_syswm.h"
#include <windows.h>
#include <stdio.h>
#include <d3d11.h>
#include <time.h>
#include <DirectXMath.h>
#include "DxInfo.h"
#include <commdlg.h>
#include "Model.h"
#include "Shader.h"
#include "Camera.h"
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"assimp-vc140-mt.lib")


LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
bool InitD3D(HINSTANCE hInstance);
void ReleaseObjects();
void LoadShader();
void BuildScene();
void UpdateScene();
void RenderScene();

FD3D11Info D3D11Info;
FMVPBuffer MVPBuffer;
SDL_Window* pMainWindow=nullptr;
HWND MainWindowHwnd=NULL;
DirectX::XMFLOAT4 ScreenColor;
RECT WindowRect = {0,0,1280,720};
FModel* targetModel;
FShader* DefVertShader, *DefPixelShader;
Camera* pMyCamera=nullptr;
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
#if _DEBUG
	{
		AllocConsole();
		FILE* tmpDebugFile;
		freopen_s(&tmpDebugFile, "CONOUT$", "w", stdout);
	}
#endif
	pMyCamera = new Camera();
	pMyCamera->SetFOV(90.f);
	pMyCamera->SetViewportSize(DirectX::XMFLOAT2(1280, 720));
	WNDCLASSEX wnd = { NULL };
	wnd.cbSize = sizeof(WNDCLASSEX);
	wnd.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wnd.hCursor = LoadCursor(0, IDC_ARROW);
	wnd.hIcon = LoadIcon(0, IDC_ICON);
	wnd.hInstance = hInstance;
	wnd.lpfnWndProc = MainWndProc;
	wnd.lpszClassName = TEXT("GalEngine Blank Win32 Window");
	wnd.style = CS_VREDRAW || CS_HREDRAW;
	RegisterClassEx(&wnd);
	AdjustWindowRect(&WindowRect, WS_OVERLAPPEDWINDOW, false);
	MainWindowHwnd = CreateWindow(wnd.lpszClassName,TEXT("Hello DirectX!"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, NULL, NULL, hInstance, NULL);
	if (MainWindowHwnd)
		ShowWindow(MainWindowHwnd,nShowCmd);
	else
	{
		MessageBox(NULL, TEXT("Unable to create Window,Exit!"),TEXT("Error"),MB_OK);
		return -1;
	}
	if (!InitD3D(hInstance))
	{
		MessageBox(MainWindowHwnd, TEXT("DirectX Init Failed,Exit!"), TEXT("Error"), MB_OK);
		return -1;
	}
	LoadShader();
	BuildScene();
	ShowCursor(FALSE);
	MSG msg = { NULL };
	while (msg.message!=WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			UpdateScene();
			RenderScene();
		}
	}
	ReleaseObjects();
	return 0;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CLOSE:
			PostQuitMessage(0);
			break;
		case WM_SIZE:
			if (wParam!= SIZE_MINIMIZED&&D3D11Info.DXGISwapChain)
			{
				D3D11Info.DXGISwapChain->ResizeBuffers(1, LOWORD(lParam), HIWORD(lParam), DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
				pMyCamera->SetViewportSize(DirectX::XMFLOAT2(LOWORD(lParam), HIWORD(lParam)));
				break;
			}
		case WM_MOUSEMOVE:
			{
				RECT Clientrc;
				GetClientRect(hwnd, &Clientrc);
				POINT ScreenPos;
				ScreenPos.y = (Clientrc.bottom - Clientrc.top) / 2 + Clientrc.top;
				ScreenPos.x = (Clientrc.right - Clientrc.left) / 2 + Clientrc.left;
				POINT CursorDetail;
				GetCursorPos(&CursorDetail);
				ClientToScreen(hwnd, &ScreenPos);
				SetCursorPos(ScreenPos.x, ScreenPos.y);
				CursorDetail.x -= ScreenPos.x;
				CursorDetail.y -= ScreenPos.y;
				pMyCamera->ProcessMouseInput(CursorDetail.x, CursorDetail.y);
				break;
			}
		case WM_KEYDOWN:
			{
				if (wParam == VK_ESCAPE)
					PostQuitMessage(0);
				break;
			}
		default:
			break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

bool InitD3D(HINSTANCE hInstance)
{
	HRESULT hr;
	DXGI_MODE_DESC backbufferDesc = { NULL };
	backbufferDesc.Height = 720;
	backbufferDesc.Width = 1280;
	backbufferDesc.RefreshRate.Numerator = 60;
	backbufferDesc.RefreshRate.Denominator = 1;
	backbufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	backbufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	backbufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_SWAP_CHAIN_DESC swapchainDesc = { NULL };
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferCount = 1;
	swapchainDesc.BufferDesc = backbufferDesc;
	swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchainDesc.OutputWindow = MainWindowHwnd;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapchainDesc.Windowed = true;
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	//swapchainDesc.SampleDesc will be NULL
	hr=D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL, D3D11_SDK_VERSION,
		&swapchainDesc, &D3D11Info.DXGISwapChain, &D3D11Info.D3D11Device, NULL, &D3D11Info.D3D11DeviceContext);
	if (FAILED(hr))
	{
		HR(hr);
		return false;
	}
	ID3D11Texture2D* BackBufferTexture;
	ID3D11Texture2D* DepthStencilTexture;
	D3D11Info.DXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBufferTexture);
	D3D11Info.D3D11Device->CreateRenderTargetView(BackBufferTexture, NULL, &D3D11Info.RenderTargetView);
	D3D11_TEXTURE2D_DESC DepthStencilDesc = { NULL };
	DepthStencilDesc.ArraySize = 1;
	DepthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	DepthStencilDesc.CPUAccessFlags = NULL;
	DepthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DepthStencilDesc.Height = 720;
	DepthStencilDesc.Width = 1280;
	DepthStencilDesc.MipLevels = 1;
	DepthStencilDesc.SampleDesc.Quality = 0;
	DepthStencilDesc.SampleDesc.Count = 1;
	DepthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	D3D11Info.D3D11Device->CreateTexture2D(&DepthStencilDesc, NULL, &DepthStencilTexture);
	D3D11Info.D3D11Device->CreateDepthStencilView(DepthStencilTexture, NULL, &D3D11Info.DepthStencilRTV);
	BackBufferTexture->Release();
	D3D11Info.D3D11DeviceContext->OMSetRenderTargets(1, &D3D11Info.RenderTargetView,D3D11Info.DepthStencilRTV);
	return true;
}

void LoadShader()
{
	DefVertShader = FShader::LoadVertexShader("DefShader.hlsl", "VS");
	DefPixelShader = FShader::LoadPixelShader("DefShader.hlsl", "PS");
}

void BuildScene()
{
	ScreenColor.x = 0.f;
	ScreenColor.y = 0.f;
	ScreenColor.z = 0.f;
	ScreenColor.w = 0.f;
	char FileP[1024] = { 0 };
	OPENFILENAMEA OpenFN = { NULL };
	OpenFN.lStructSize = sizeof(OPENFILENAMEA);
	OpenFN.Flags = OFN_FILEMUSTEXIST;
	OpenFN.lpstrFilter = "模型文件\0*.obj;*.fbx;*.3ds\0\0";
	OpenFN.nMaxFile = MAX_PATH;
	OpenFN.lpstrFile = FileP;
	OpenFN.hInstance = GetModuleHandle(NULL);
	OpenFN.lpstrTitle = "选择模型文件";
	if (GetOpenFileNameA(&OpenFN))
	{
		targetModel = FModel::LoadModel(FileP);
		if (targetModel)
		{
			FTransform mTransform = targetModel->GetTransform();
			mTransform.Scale = DirectX::XMFLOAT3(0.1f, 0.1f, 0.1f);
			mTransform.Location = DirectX::XMFLOAT3(0, 0, 50);
			targetModel->SetTransform(mTransform);
		}
	}
	targetModel->RenderInit(DefVertShader, DefPixelShader);
	D3D11_BUFFER_DESC MVPBufferDesc = {NULL};
	MVPBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	MVPBufferDesc.ByteWidth = sizeof(FMVPBuffer);
	MVPBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	MVPBufferDesc.StructureByteStride = NULL;
	D3D11_SUBRESOURCE_DATA MVPSubresourceD = { NULL };
	MVPSubresourceD.pSysMem = &MVPBuffer;
	D3D11Info.D3D11Device->CreateBuffer(&MVPBufferDesc, &MVPSubresourceD, &D3D11Info.D3DMVPBuffer);
	D3D11_VIEWPORT Viewport = { NULL };
	Viewport.Height = 720;
	Viewport.Width = 1280;
	Viewport.TopLeftX = 0;
	Viewport.TopLeftY = 0;
	D3D11Info.D3D11DeviceContext->RSSetViewports(1, &Viewport);
}

void UpdateScene()
{
	ScreenColor.x = (float)sin((double)clock() / CLOCKS_PER_SEC)*0.5f + 0.5f;
	ScreenColor.y = (float)sin((double)clock() / CLOCKS_PER_SEC)*0.5f + 0.5f;
	ScreenColor.z = (float)sin((double)clock() / CLOCKS_PER_SEC)*0.5f + 0.5f;
	FTransform lastTransform = targetModel->GetTransform();
	lastTransform.Location.z = -abs((float)sin((double)clock() / CLOCKS_PER_SEC)*100.f);
	targetModel->SetTransform(lastTransform);
}

void RenderScene()
{
	D3D11Info.D3D11DeviceContext->ClearRenderTargetView(D3D11Info.RenderTargetView, (float*)&ScreenColor);
	D3D11Info.D3D11DeviceContext->ClearDepthStencilView(D3D11Info.DepthStencilRTV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
	MVPBuffer.ViewMatrix = pMyCamera->GenViewMatrix();
	MVPBuffer.ProjectionMatrix = pMyCamera->GenProjectionMatrix();
	if (targetModel)
		targetModel->Draw();
	D3D11Info.DXGISwapChain->Present(0, NULL);
}

void ReleaseObjects()
{
	D3D11Info.D3D11Device->Release();
	D3D11Info.D3D11DeviceContext->Release();
	D3D11Info.DXGISwapChain->Release();
}