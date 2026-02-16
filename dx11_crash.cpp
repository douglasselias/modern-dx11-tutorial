#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")
#pragma comment(lib, "windowscodecs")
#pragma comment(lib, "ole32")

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <math.h>
#include <stdint.h>
#include <wincodec.h>
// #include <wincodecsdk.h>
// #include <objbase.h>

// #define STB_IMAGE_IMPLEMENTATION
// #include "stb_image.h"

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

u8* decode_image(char *file_name, u32 *width, u32 *height);

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
  char *title = "DX11 Crash";

  WNDCLASS window_class      = {};
  window_class.lpfnWndProc   = DefWindowProc;
  window_class.lpszClassName = title;
  window_class.hCursor       = LoadCursor(null, IDC_ARROW);

  RegisterClass(&window_class);

  HWND window = CreateWindow(title, title, WS_POPUP | WS_MAXIMIZE | WS_VISIBLE, 0, 0, 0, 0, null, null, null, null);

  IDXGISwapChain *swap_chain;
  ID3D11Device *device;
  ID3D11DeviceContext *device_context;
  D3D11_VIEWPORT viewport = {};

  DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};
  swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swap_chain_desc.SampleDesc.Count  = 1;
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
    -0.5,  0.5, 1, 1, 1, 0, 0,
    -0.5, -0.5, 1, 1, 1, 0, 1,
     0.5,  0.5, 1, 1, 1, 1, 0,
     0.5, -0.5, 1, 1, 1, 1, 1,
  };

  u32 my_vertices_size = ARRAYSIZE(my_vertices);
  u8 number_of_components = 7;
  u32 my_vertices_stride = sizeof(f32) * number_of_components;
  u32 my_offset = 0;

  ID3D11Buffer *my_vertices_buffer;
  D3D11_BUFFER_DESC buffer_desc = {};

  buffer_desc.ByteWidth = my_vertices_size * sizeof(f32);
  buffer_desc.Usage     = D3D11_USAGE_IMMUTABLE;
  buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  
  D3D11_SUBRESOURCE_DATA subresource = { my_vertices };
  
  device->CreateBuffer(&buffer_desc, &subresource, &my_vertices_buffer);

  u32 my_indices[] =
  {
    0, 1, 2,
    1, 3, 2,
  };

  u32 my_indices_size = ARRAYSIZE(my_indices);

  ID3D11Buffer *my_indices_buffer;
  D3D11_BUFFER_DESC indices_buffer_desc = {};
  indices_buffer_desc.ByteWidth = sizeof(u32) * my_indices_size;
  indices_buffer_desc.Usage     = D3D11_USAGE_IMMUTABLE;
  indices_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
  
  D3D11_SUBRESOURCE_DATA indices_subresource = { my_indices };
  
  device->CreateBuffer(&indices_buffer_desc, &indices_subresource, &my_indices_buffer);

  u32 width, height;
  // u8 *texture_data = stbi_load("texture_01.png", &width, &height, null, 4);
  u8 *texture_data = decode_image("texture_01.png", &width, &height);

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

  device->CreateShaderResourceView(texture_2d, null, &texture_srv);

  char *shader =
  R"(
    struct Vertex
    {
      float2 position : POS;
      float3 color    : COL;
      float2 texcoord : TEX;
    };

    struct Pixel
    {
      float4 position : SV_POSITION;
      float3 color    : COL;
      float2 texcoord : TEX;
    };

    Texture2D    mytexture : register(t0);
    SamplerState mysampler : register(s0);

    Pixel vs_main(Vertex v)
    {
      Pixel output;
      output.position = float4(v.position, 0, 1);
      output.color    = v.color;
      output.texcoord = v.texcoord;
      return output;
    }

    float4 ps_main(Pixel p) : SV_TARGET
    {
      return mytexture.Sample(mysampler, p.texcoord) * float4(p.color, 1);
    }
  )";

  size_t shader_length = strlen(shader);
  
  ID3DBlob *vertex_shader_blob;
  ID3DBlob *pixel_shader_blob;

  D3DCompile(shader, shader_length, NULL, NULL, NULL, "vs_main", "vs_5_0", 0, 0, &vertex_shader_blob, NULL);
  D3DCompile(shader, shader_length, NULL, NULL, NULL, "ps_main", "ps_5_0", 0, 0, &pixel_shader_blob, NULL);

  ID3D11VertexShader *vertex_shader;
  ID3D11PixelShader *pixel_shader;

  device->CreateVertexShader(vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(), null, &vertex_shader);
  device->CreatePixelShader(pixel_shader_blob->GetBufferPointer(), pixel_shader_blob->GetBufferSize(), null, &pixel_shader);

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
    device_context->IASetIndexBuffer(my_indices_buffer, DXGI_FORMAT_R32_UINT, 0);
    
    device_context->VSSetShader(vertex_shader, null, 0);
    
    device_context->RSSetViewports(1, &viewport);
    device_context->RSSetState(rasterizer_state);

    device_context->PSSetShader(pixel_shader, null, 0);
    device_context->PSSetShaderResources(0, 1, &texture_srv);

    device_context->OMSetRenderTargets(1, &rtv, null);

    device_context->DrawIndexed(my_indices_size, 0, 0);

    swap_chain->Present(1, 0);
  }

  return 0;
}

u8* decode_image(char *file_name, u32 *width, u32 *height)
{
  u8 *data = NULL;
  HRESULT hr;

  hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
  if(hr != S_OK) goto end;

  IWICImagingFactory *wic_factory = NULL;
  hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (void**)&wic_factory);
  if(hr != S_OK) goto end;
  
  IWICBitmapDecoder *wic_decoder;

  s32 file_name_length = (s32)strlen(file_name);
  s32 size = MultiByteToWideChar(CP_UTF8, 0, file_name, file_name_length, NULL, 0);

  wchar_t *file_name16 = (wchar_t*)malloc((size + 1) * sizeof(wchar_t));
  MultiByteToWideChar(CP_UTF8, 0, file_name, file_name_length, file_name16, size);
  file_name16[size] = L'\0';

  hr = wic_factory->CreateDecoderFromFilename(file_name16, &GUID_VendorMicrosoftBuiltIn, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &wic_decoder);
  if(hr != S_OK) goto end;

  IWICBitmapFrameDecode *wic_bitmap = NULL;
  hr = wic_decoder->GetFrame(0, &wic_bitmap);
  if(hr != S_OK) goto end;

  IWICFormatConverter *wic_converter = NULL;
  hr = wic_factory->CreateFormatConverter(&wic_converter);
  if(hr != S_OK) goto end;

  hr = wic_converter->Initialize(wic_bitmap, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeMedianCut);
  if(hr != S_OK) goto end;

  hr = wic_converter->GetSize(width, height);
  if(hr != S_OK) goto end;

  u32 bytes_per_pixel = 4;
  u32 stride = (*width) * bytes_per_pixel;
  u32 data_size = stride * (*height);

  data = (u8*)malloc(data_size);

  hr = wic_converter->CopyPixels(NULL, stride, data_size, data);
  if(hr != S_OK) goto end;

  end:
  return data;
}