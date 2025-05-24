## Part 1 - Colored background

To get started we will render a colored background that changes color trough time. This is the simplest and minimal code to get something rendered on screen.

This is a way to import `.lib` files in code, without needing to specify on the compiler command.

```cpp
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")
```

Then I define some better type names for convenience.

```cpp
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
```

Before creating a window we need to register a class. Here you can define some window properties.

```cpp
char* title = "Modern DX11 Tutorial - Part 1 - Colored background";

{
  WNDCLASS window_class      = {};
  window_class.lpfnWndProc   = DefWindowProc; // Using the default WindowProc function.
  window_class.lpszClassName = title;
  window_class.hCursor       = LoadCursor(null, IDC_ARROW); // We set the cursor to be the default arrow since the window thinks the message loop is stuck, so it shows the hourglass cursor by default. More on that later.

  RegisterClass(&window_class);
}

HWND window = CreateWindow(title, title, WS_POPUP | WS_MAXIMIZE | WS_VISIBLE, 0, 0, 0, 0, null, null, null, null); // Create a fullscreen window without border.
```

Swap chain describes the properties of how to do [double buffering](https://en.wikipedia.org/wiki/Multiple_buffering#Double_buffering_in_computer_graphics). The device handles mostly the creation of resources and the device context handles the graphics pipeline.

```cpp
IDXGISwapChain* swap_chain;
ID3D11Device* device;
ID3D11DeviceContext* device_context;

{
  DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};

  swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swap_chain_desc.SampleDesc.Count = 1; // No MSAA.
  
  swap_chain_desc.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swap_chain_desc.BufferCount  = 2; // Use two buffers. (Double buffering)
  swap_chain_desc.OutputWindow = window;
  swap_chain_desc.Windowed     = true;
  swap_chain_desc.SwapEffect   = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  
  D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0 };
  
  D3D11CreateDeviceAndSwapChain(null, D3D_DRIVER_TYPE_HARDWARE, null, 0, feature_levels, ARRAYSIZE(feature_levels), D3D11_SDK_VERSION, &swap_chain_desc, &swap_chain, &device, null, &device_context);
}
```

Now we need a render target view to be able to render.

```cpp
ID3D11RenderTargetView* rtv;
{
  ID3D11Texture2D* rtv_texture;
  swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&rtv_texture); // Haven't found any explanation of this, but every code sample uses this. This just creates a texture for the render target view.
  
  D3D11_RENDER_TARGET_VIEW_DESC rtv_desc = {};
  rtv_desc.Format        = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // SRGB -> Gamma corrected
  rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
  
  device->CreateRenderTargetView(rtv_texture, &rtv_desc, &rtv);
}
```

Now we write the main loop.

```cpp
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

  // A simple and nice way to smoothly transition between colors.
  background_color[0] = (f32)(0.5 + 0.5 * sin(now));
  background_color[1] = (f32)(0.5 + 0.5 * sin(now + PI * 2 / 3));
  background_color[2] = (f32)(0.5 + 0.5 * sin(now + PI * 4 / 3));

  device_context->ClearRenderTargetView(rtv, background_color);

  swap_chain->Present(1, 0);
}
```