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

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
  char* title = "Modern DX11 Tutorial - Part 2 - Single color triangle";

  {
    WNDCLASS window_class      = {};
    window_class.lpfnWndProc   = DefWindowProc;
    window_class.lpszClassName = title;
    window_class.hCursor       = LoadCursor(null, IDC_ARROW);

    RegisterClass(&window_class);
  }

  HWND window = CreateWindow(title, title, WS_POPUP | WS_MAXIMIZE | WS_VISIBLE, 0, 0, 0, 0, null, null, null, null);

  IDXGISwapChain* swap_chain;
  ID3D11Device* device;
  ID3D11DeviceContext* device_context;
  D3D11_VIEWPORT viewport = {};

  {
    DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};

    swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_desc.SampleDesc.Count = 1;
    
    swap_chain_desc.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.BufferCount  = 2;
    swap_chain_desc.OutputWindow = window;
    swap_chain_desc.Windowed     = true;
    swap_chain_desc.SwapEffect   = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    
    D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0 };
    
    D3D11CreateDeviceAndSwapChain(null, D3D_DRIVER_TYPE_HARDWARE, null, 0, feature_levels, ARRAYSIZE(feature_levels), D3D11_SDK_VERSION, &swap_chain_desc, &swap_chain, &device, null, &device_context);

    swap_chain->GetDesc(&swap_chain_desc);
    
    viewport.Width    = (f32)swap_chain_desc.BufferDesc.Width;
    viewport.Height   = (f32)swap_chain_desc.BufferDesc.Height;
    viewport.MaxDepth = 1;
  }

  ID3D11RenderTargetView* rtv;
  {
    ID3D11Texture2D* rtv_texture;
    swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&rtv_texture);
    
    D3D11_RENDER_TARGET_VIEW_DESC rtv_desc = {};
    rtv_desc.Format        = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    
    device->CreateRenderTargetView(rtv_texture, &rtv_desc, &rtv);
  }

  f32 background_color[4] = {0, 0, 0, 1};

  f32 my_vertices[] =
  {
     0.0,  0.75, // Top
    -0.5, -0.75, // Left
     0.5, -0.75, // Right
  };

  u32 my_vertices_size = ARRAYSIZE(my_vertices);
  u8 number_of_components = 2;
  u32 my_vertices_count = my_vertices_size / number_of_components;
  u32 my_vertices_stride = sizeof(f32) * number_of_components;
  u32 my_offset = 0;

  ID3D11Buffer* my_vertices_buffer;
  {
    D3D11_BUFFER_DESC buffer_desc = {};

    buffer_desc.ByteWidth = my_vertices_size * sizeof(f32);
    buffer_desc.Usage     = D3D11_USAGE_IMMUTABLE;
    buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    
    D3D11_SUBRESOURCE_DATA subresource = { my_vertices };
    
    device->CreateBuffer(&buffer_desc, &subresource, &my_vertices_buffer);
  }

  ID3D11VertexShader* vertex_shader;
  ID3DBlob* vertex_shader_blob;

  D3DCompileFromFile(L"002_single_color_triangle.hlsl", null, null, "vs_main", "vs_5_0", 0, 0, &vertex_shader_blob, null);
  device->CreateVertexShader(vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(), null, &vertex_shader);

  ID3D11PixelShader* pixel_shader;
  ID3DBlob* pixel_shader_blob;

  D3DCompileFromFile(L"002_single_color_triangle.hlsl", null, null, "ps_main", "ps_5_0", 0, 0, &pixel_shader_blob, null);
  device->CreatePixelShader(pixel_shader_blob->GetBufferPointer(), pixel_shader_blob->GetBufferSize(), null, &pixel_shader);

  ID3D11InputLayout* input_layout;
  {
    D3D11_INPUT_ELEMENT_DESC input_element_desc[] =
    {
      {"POS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    device->CreateInputLayout(input_element_desc, ARRAYSIZE(input_element_desc), vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(), &input_layout);
  }
  
  ID3D11RasterizerState* rasterizer_state;
  {
    D3D11_RASTERIZER_DESC rasterizer_desc = { D3D11_FILL_SOLID, D3D11_CULL_NONE };
    device->CreateRasterizerState(&rasterizer_desc, &rasterizer_state);
  }

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

    device_context->ClearRenderTargetView(rtv, background_color);

    device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    device_context->IASetInputLayout(input_layout);
    device_context->IASetVertexBuffers(0, 1, &my_vertices_buffer, &my_vertices_stride, &my_offset);
    
    device_context->VSSetShader(vertex_shader, null, 0);
    
    device_context->RSSetViewports(1, &viewport);
    device_context->RSSetState(rasterizer_state);

    device_context->PSSetShader(pixel_shader, null, 0);

    device_context->OMSetRenderTargets(1, &rtv, null);

    device_context->Draw(my_vertices_count, 0);

    swap_chain->Present(1, 0);
  }

  return 0;
}