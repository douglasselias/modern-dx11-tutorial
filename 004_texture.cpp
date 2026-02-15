#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <math.h>
#include <stdint.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

typedef int32_t  s32;
typedef uint8_t  u8;
typedef uint32_t u32;
typedef float    f32;

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
  char *title = "Modern DX11";

  WNDCLASS window_class      = {};
  window_class.lpfnWndProc   = DefWindowProc;
  window_class.lpszClassName = title;
  window_class.hCursor       = LoadCursor(NULL, IDC_ARROW);

  RegisterClass(&window_class);

  HWND window = CreateWindow(title, title, WS_POPUP | WS_MAXIMIZE | WS_VISIBLE, 0, 0, 0, 0, NULL, NULL, NULL, NULL);

  IDXGISwapChain *swap_chain;
  ID3D11Device *device;
  ID3D11DeviceContext *device_context;
  D3D11_VIEWPORT viewport = {};

  DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};

  swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swap_chain_desc.SampleDesc.Count = 1;
  
  swap_chain_desc.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swap_chain_desc.BufferCount  = 2;
  swap_chain_desc.OutputWindow = window;
  swap_chain_desc.Windowed     = true;
  swap_chain_desc.SwapEffect   = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  
  D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0 };
  
  D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, feature_levels, ARRAYSIZE(feature_levels), D3D11_SDK_VERSION, &swap_chain_desc, &swap_chain, &device, NULL, &device_context);

  swap_chain->GetDesc(&swap_chain_desc);
  
  viewport.Width    = (f32)swap_chain_desc.BufferDesc.Width;
  viewport.Height   = (f32)swap_chain_desc.BufferDesc.Height;
  viewport.MaxDepth = 1;

  ID3D11RenderTargetView *rtv;
  ID3D11Texture2D *rtv_texture;
  swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&rtv_texture);
  
  D3D11_RENDER_TARGET_VIEW_DESC rtv_desc = {};
  rtv_desc.Format        = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
  rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
  
  device->CreateRenderTargetView(rtv_texture, &rtv_desc, &rtv);

  f32 background_color[4] = {0, 0, 0, 1};

  f32 my_vertices[] =
  {
     0.0,  0.75, 1, 1, 1, 0.5, 1.0,
    -0.5, -0.75, 1, 1, 1, 0.0, 0.0,
     0.5, -0.75, 1, 1, 1, 1.0, 0.0,
  };

  u32 my_vertices_size = ARRAYSIZE(my_vertices);
  u8 number_of_components = 7;
  u32 my_vertices_count = my_vertices_size / number_of_components;
  u32 my_vertices_stride = sizeof(f32) * number_of_components;
  u32 my_offset = 0;

  ID3D11Buffer *my_vertices_buffer;

  D3D11_BUFFER_DESC buffer_desc = {};
  buffer_desc.ByteWidth = my_vertices_size * sizeof(f32);
  buffer_desc.Usage     = D3D11_USAGE_IMMUTABLE;
  buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  
  D3D11_SUBRESOURCE_DATA subresource = { my_vertices };
  
  device->CreateBuffer(&buffer_desc, &subresource, &my_vertices_buffer);

  stbi_set_flip_vertically_on_load(false);
  s32 width, height, number_of_color_channels = 4;
  u8* texture_data = stbi_load("texture_01.png", &width, &height, NULL, number_of_color_channels);

  ID3D11ShaderResourceView *texture_srv;

  D3D11_TEXTURE2D_DESC texture_desc = {};
  texture_desc.Width            = width;
  texture_desc.Height           = height;
  texture_desc.MipLevels        = 1;
  texture_desc.ArraySize        = 1;
  texture_desc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
  texture_desc.Usage            = D3D11_USAGE_IMMUTABLE;
  texture_desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;
  texture_desc.SampleDesc.Count = 1;

  D3D11_SUBRESOURCE_DATA texture_subresource = {};
  texture_subresource.pSysMem     = texture_data;
  texture_subresource.SysMemPitch = width * sizeof(u32);

  ID3D11Texture2D *texture_2d;
  device->CreateTexture2D(&texture_desc, &texture_subresource, &texture_2d);

  device->CreateShaderResourceView(texture_2d, NULL, &texture_srv);

  ID3D11SamplerState *sampler_state;
  D3D11_SAMPLER_DESC sampler_desc = {};
  sampler_desc.Filter         = D3D11_FILTER_MIN_MAG_MIP_POINT;
  sampler_desc.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
  sampler_desc.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
  sampler_desc.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
  sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  
  device->CreateSamplerState(&sampler_desc, &sampler_state);

  ID3D11VertexShader *vertex_shader;
  ID3DBlob* vertex_shader_blob;

  D3DCompileFromFile(L"004_texture.hlsl", NULL, NULL, "vs_main", "vs_5_0", 0, 0, &vertex_shader_blob, NULL);
  device->CreateVertexShader(vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(), NULL, &vertex_shader);

  ID3D11PixelShader *pixel_shader;
  ID3DBlob* pixel_shader_blob;

  D3DCompileFromFile(L"004_texture.hlsl", NULL, NULL, "ps_main", "ps_5_0", 0, 0, &pixel_shader_blob, NULL);
  device->CreatePixelShader(pixel_shader_blob->GetBufferPointer(), pixel_shader_blob->GetBufferSize(), NULL, &pixel_shader);

  ID3D11InputLayout *input_layout;
  D3D11_INPUT_ELEMENT_DESC input_element_desc[] =
  {
    { "POS", 0, DXGI_FORMAT_R32G32_FLOAT,    0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEX", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };

  device->CreateInputLayout(input_element_desc, ARRAYSIZE(input_element_desc), vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(), &input_layout);
  
  ID3D11RasterizerState *rasterizer_state;
  D3D11_RASTERIZER_DESC rasterizer_desc = { D3D11_FILL_SOLID, D3D11_CULL_NONE };
  device->CreateRasterizerState(&rasterizer_desc, &rasterizer_state);

  for(bool running = true; running;)
  {
    MSG msg;
    if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
      DispatchMessage(&msg);
    }

    if(GetAsyncKeyState(VK_ESCAPE) & 0x8000) break;

    device_context->ClearRenderTargetView(rtv, background_color);

    device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    device_context->IASetInputLayout(input_layout);
    device_context->IASetVertexBuffers(0, 1, &my_vertices_buffer, &my_vertices_stride, &my_offset);
    
    device_context->VSSetShader(vertex_shader, NULL, 0);
    
    device_context->RSSetViewports(1, &viewport);
    device_context->RSSetState(rasterizer_state);

    device_context->PSSetShader(pixel_shader, NULL, 0);
    device_context->PSSetSamplers(0, 1, &sampler_state);
    device_context->PSSetShaderResources(0, 1, &texture_srv);

    device_context->OMSetRenderTargets(1, &rtv, NULL);

    device_context->Draw(my_vertices_count, 0);

    swap_chain->Present(1, 0);
  }

  return 0;
}