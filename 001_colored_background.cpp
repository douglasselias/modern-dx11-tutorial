#define _USE_MATH_DEFINES
#include <math.h>
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

double elapsed_milliseconds()
{
  return GetTickCount() / 1000.0;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
  char *title = "001 - Colored background";

  WNDCLASS window_class      = {};
  window_class.lpfnWndProc   = DefWindowProc;
  window_class.lpszClassName = title;
  window_class.hCursor       = LoadCursor(NULL, IDC_ARROW);

  RegisterClass(&window_class);

  HWND window = CreateWindow(title, title, WS_POPUP | WS_MAXIMIZE | WS_VISIBLE, 0, 0, 0, 0, NULL, NULL, NULL, NULL);

  DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};

  swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swap_chain_desc.SampleDesc.Count = 1;
  
  swap_chain_desc.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swap_chain_desc.BufferCount  = 2;
  swap_chain_desc.OutputWindow = window;
  swap_chain_desc.Windowed     = true;
  swap_chain_desc.SwapEffect   = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  
  D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0 };
  
  IDXGISwapChain *swap_chain;
  ID3D11Device *device;
  ID3D11DeviceContext *device_context;

  D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, feature_levels, ARRAYSIZE(feature_levels), D3D11_SDK_VERSION, &swap_chain_desc, &swap_chain, &device, NULL, &device_context);

  ID3D11Texture2D *rtv_texture;
  swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&rtv_texture);
  
  D3D11_RENDER_TARGET_VIEW_DESC rtv_desc = {};
  rtv_desc.Format        = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
  rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
  
  ID3D11RenderTargetView *rtv;
  device->CreateRenderTargetView(rtv_texture, &rtv_desc, &rtv);

  float background_color[4] = { 0.025f, 0.025f, 0.025f, 1.0f };

  for(bool running = true; running;)
  {
    MSG msg;
    if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
      DispatchMessage(&msg);
    }

    if(GetAsyncKeyState(VK_ESCAPE) & 0x8000) break;

    double now = elapsed_milliseconds();

    background_color[0] = (float)(0.5 + 0.5 * sin(now));
    background_color[1] = (float)(0.5 + 0.5 * sin(now + M_PI * 2 / 3));
    background_color[2] = (float)(0.5 + 0.5 * sin(now + M_PI * 4 / 3));

    device_context->ClearRenderTargetView(rtv, background_color);

    swap_chain->Present(1, 0);
  }

  return 0;
}