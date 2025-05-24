#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <math.h>
#include <stdint.h>

typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float    f32;
typedef double   f64;
typedef wchar_t  wchar;
#define null     NULL

#define PI 3.14159265358979323846

f64 elapsed_milliseconds()
{
  return GetTickCount() / 1000.0;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
  char* title = "Modern DX11 Tutorial - Part 1 - Colored background";

  {
    WNDCLASSA window_class     = {};
    window_class.lpfnWndProc   = DefWindowProc;
    window_class.lpszClassName = title;
    window_class.hCursor       = LoadCursor(null, IDC_ARROW);
    RegisterClass(&window_class);
  }

  HWND window = CreateWindow(title, title, WS_POPUP | WS_MAXIMIZE | WS_VISIBLE, 0, 0, 0, 0, null, null, null, null);

  IDXGISwapChain* swap_chain;
  ID3D11Device* device;
  ID3D11DeviceContext* device_context;

  {
    DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};
    swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swap_chain_desc.SampleDesc.Count = 1;
    
    swap_chain_desc.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.BufferCount  = 2;
    swap_chain_desc.OutputWindow = window;
    swap_chain_desc.Windowed     = true;
    swap_chain_desc.SwapEffect   = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    
    D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0 };
    
    D3D11CreateDeviceAndSwapChain(null, D3D_DRIVER_TYPE_HARDWARE, null, D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG, feature_levels, ARRAYSIZE(feature_levels), D3D11_SDK_VERSION, &swap_chain_desc, &swap_chain, &device, null, &device_context); 
  }

  ID3D11RenderTargetView* rtv;
  {
    ID3D11Texture2D* rtv_texture;
    swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&rtv_texture);
    
    D3D11_RENDER_TARGET_VIEW_DESC rtv_desc = {};
    rtv_desc.Format        = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    
    device->CreateRenderTargetView(rtv_texture, &rtv_desc, &rtv);
  }

  f32 background_color[4] = { 0.025f, 0.025f, 0.025f, 1.0f };
  
  bool running = true;
  while(running)
  {
    MSG msg;
    while(PeekMessage(&msg, null, 0, 0, PM_REMOVE))
    {
      if(msg.message == WM_KEYDOWN)
      {
        switch (msg.wParam)
        {
          case VK_ESCAPE:
          case VK_OEM_3: running = false; break;
        }
      }

      DispatchMessage(&msg);
    }

    f64 now = elapsed_milliseconds();

    background_color[0] = (f32)(0.5 + 0.5 * sin(now));
    background_color[1] = (f32)(0.5 + 0.5 * sin(now + PI * 2 / 3));
    background_color[2] = (f32)(0.5 + 0.5 * sin(now + PI * 4 / 3));

    device_context->ClearRenderTargetView(rtv, background_color);

    swap_chain->Present(1, 0);
  }

  return 0;
}