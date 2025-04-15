#define WIN32_LEAN_AND_MEAN

#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#pragma warning(push)
#pragma warning(disable: 4061)
#pragma warning(disable: 4820)
#pragma warning(disable: 4365)
#pragma warning(disable: 4668)
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#pragma warning(pop)

#include <math.h>

struct Renderer
{
  DXGI_SWAP_CHAIN_DESC swap_chain_desc;
  IDXGISwapChain* swapchain;
  ID3D11Device* device;
  ID3D11DeviceContext* device_context;
  D3D11_VIEWPORT viewport;

  ID3D11RenderTargetView* framebufferRTV;
  ID3D11DepthStencilView* depthbufferDSV;
};

Renderer create_renderer(HWND window)
{
  Renderer renderer = {};

  DXGI_MODE_DESC BufferDesc = {};
  /// @todo: Can't specify SRGB framebuffer directly when using FLIP model swap effect.
  BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
  /// @note: You can also specify the refresh rate.
  DXGI_RATIONAL refresh_rate = {1, 75};
  BufferDesc.RefreshRate = refresh_rate;

  DXGI_SAMPLE_DESC SampleDesc = {};
  SampleDesc.Count = 1;

  renderer.swap_chain_desc.BufferDesc = BufferDesc;
  renderer.swap_chain_desc.SampleDesc = SampleDesc;

  renderer.swap_chain_desc.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  renderer.swap_chain_desc.BufferCount  = 2;
  renderer.swap_chain_desc.OutputWindow = window;
  renderer.swap_chain_desc.Windowed     = true;
  renderer.swap_chain_desc.SwapEffect   = DXGI_SWAP_EFFECT_FLIP_DISCARD;

  D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0 };

  /// @todo: Look into this flag for shader debugging: D3D11_CREATE_DEVICE_DEBUGGABLE
  D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG, feature_levels, ARRAYSIZE(feature_levels), D3D11_SDK_VERSION, &renderer.swap_chain_desc, &renderer.swapchain, &renderer.device, nullptr, &renderer.device_context);

  /// @note: Update swap_chain_desc with actual window size.
  renderer.swapchain->GetDesc(&renderer.swap_chain_desc);

  D3D11_VIEWPORT viewport = {};
  viewport.Width    = (float)renderer.swap_chain_desc.BufferDesc.Width;
  viewport.Height   = (float)renderer.swap_chain_desc.BufferDesc.Height;
  viewport.MaxDepth = 1;

  renderer.viewport = viewport;

  return renderer;
}

void create_render_target_view_and_depth_stencil_view(Renderer* renderer)
{
  ID3D11Texture2D* framebuffer;
  renderer->swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&framebuffer); // get buffer from swapchain..

  D3D11_RENDER_TARGET_VIEW_DESC framebufferRTVdesc = {}; // (needed for SRGB framebuffer when using FLIP model swap effect)
  framebufferRTVdesc.Format        = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
  framebufferRTVdesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

  ID3D11RenderTargetView* framebufferRTV;
  renderer->device->CreateRenderTargetView(framebuffer, &framebufferRTVdesc, &framebufferRTV); // ..and put a render target view on it

  renderer->framebufferRTV = framebufferRTV;

  D3D11_TEXTURE2D_DESC depth_buffer_desc;
  framebuffer->GetDesc(&depth_buffer_desc); // copy framebuffer properties; they're mostly the same

  depth_buffer_desc.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depth_buffer_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

  ID3D11Texture2D* depthbuffer;
  renderer->device->CreateTexture2D(&depth_buffer_desc, nullptr, &depthbuffer);

  ID3D11DepthStencilView* depthbufferDSV;
  renderer->device->CreateDepthStencilView(depthbuffer, nullptr, &depthbufferDSV);

  renderer->depthbufferDSV = depthbufferDSV;
}

void clear_screen(Renderer renderer, float clearcolor[4])
{
  renderer.device_context->ClearRenderTargetView(renderer.framebufferRTV, clearcolor);
  renderer.device_context->ClearDepthStencilView(renderer.depthbufferDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

HWND create_window(const char* title)
{
  /// @todo: Add a proper window proc.
  WNDCLASSA window_class = {0, DefWindowProcA, 0, 0, 0, 0, 0, 0, 0, title};
  RegisterClassA(&window_class);

  HWND window = CreateWindowExA(0, title, title, WS_POPUP | WS_MAXIMIZE | WS_VISIBLE, 0, 0, 0, 0, nullptr, nullptr, nullptr, nullptr);

  return window;
}

float lerp(float start, float end, float amount)
{
  return start + (end - start) * amount;
}

double milliseconds_now()
{
  return GetTickCount64() / 1000.0f;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
  HWND window = create_window("Modern DX11 Tutorial - Part 1 - Colored background");
  Renderer renderer = create_renderer(window);
  create_render_target_view_and_depth_stencil_view(&renderer);

  FLOAT clearcolor[4] = { 0.025f, 0.025f, 0.025f, 1.0f };
  double PI = 3.1415;

  double start = milliseconds_now();

  while (true)
  {
    MSG msg;
    while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE))
    {
      if (msg.message == WM_KEYDOWN)
      {
        switch (msg.wParam)
        {
          case VK_ESCAPE:
          case VK_OEM_3: return 0;
        }
      }
      DispatchMessageA(&msg);
    }

    double elapsed = milliseconds_now() - start;
    start = milliseconds_now();
    double now = milliseconds_now();

    clearcolor[0] = 0.5f + 0.5f * sin(now);
    clearcolor[1] = 0.5f + 0.5f * sin(now + PI * 2 / 3);
    clearcolor[2] = 0.5f + 0.5f * sin(now + PI * 4 / 3);

    clear_screen(renderer, clearcolor);

    renderer.swapchain->Present(1, 0);
  }

  return 0;
}