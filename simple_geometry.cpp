#define _USE_MATH_DEFINES
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
  char *title = "Simple Geometry";

  WNDCLASS window_class = {};
  window_class.lpfnWndProc   = DefWindowProc;
  window_class.lpszClassName = title;
  window_class.hCursor       = LoadCursor(NULL, IDC_ARROW);

  RegisterClass(&window_class);

  int window_size = 50;
  HWND window = CreateWindow(title, title, WS_POPUP | WS_VISIBLE| WS_MAXIMIZE, 0, 0, window_size, window_size, NULL, NULL, NULL, NULL);

  IDXGISwapChain *swap_chain;
  ID3D11Device *device;
  ID3D11DeviceContext *device_context;
  D3D11_VIEWPORT viewport = {};

  DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};
  swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swap_chain_desc.SampleDesc.Count  = 1;
  swap_chain_desc.BufferUsage       = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swap_chain_desc.BufferCount       = 2;
  swap_chain_desc.OutputWindow      = window;
  swap_chain_desc.Windowed          = true;
  swap_chain_desc.SwapEffect        = DXGI_SWAP_EFFECT_DISCARD;
  
  D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0 };
  
  D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, feature_levels, ARRAYSIZE(feature_levels), D3D11_SDK_VERSION, &swap_chain_desc, &swap_chain, &device, NULL, &device_context);

  swap_chain->GetDesc(&swap_chain_desc);
  
  viewport.Width    = (float)swap_chain_desc.BufferDesc.Width;
  viewport.Height   = (float)swap_chain_desc.BufferDesc.Height;
  viewport.MaxDepth = 1;

  ID3D11Texture2D *rtv_texture;
  swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&rtv_texture);
  
  D3D11_RENDER_TARGET_VIEW_DESC rtv_desc = {};
  rtv_desc.Format        = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
  rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
  
  ID3D11RenderTargetView *rtv;
  device->CreateRenderTargetView(rtv_texture, &rtv_desc, &rtv);

  float background_color[4] = {0, 0, 0, 1};

  float my_vertices[] =
  {
    0, 0, 0, 1, 1, 1,
    0, 0.5, 0, 1, 1, 1,
    0, -0.5, 0, 1, 1, 1,
    0.5, 0, 0, 1, 1, 1,
    -0.5, 0, 0, 1, 1, 1,

    0.5, 0.5, 0, 1, 1, 1,
    0.5, -0.5, 0, 1, 1, 1,
    -0.5, -0.5, 0, 1, 1, 1,
    -0.5, 0.5, 0, 1, 1, 1,

    // -0.5, -0.75, 0, 1, 1, 1,
    //  0.0,  0.75, 0, 1, 1, 1,
    //  0.5, -0.75, 0, 1, 1, 1,
  };

  unsigned int my_vertices_size = ARRAYSIZE(my_vertices);
  unsigned char number_of_components = 6;
  unsigned int my_vertices_count = my_vertices_size / number_of_components;
  unsigned int my_vertices_stride = sizeof(float) * number_of_components;
  unsigned int my_offset = 0;

  ID3D11Buffer *my_vertices_buffer;
  D3D11_BUFFER_DESC buffer_desc = {};

  buffer_desc.ByteWidth = my_vertices_size * sizeof(float);
  buffer_desc.Usage     = D3D11_USAGE_IMMUTABLE;
  buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  
  D3D11_SUBRESOURCE_DATA subresource = { my_vertices };
  
  device->CreateBuffer(&buffer_desc, &subresource, &my_vertices_buffer);
  
  u32 width, height;
  u8 *texture_data = decode_image("texture_gradient_radial.png", &width, &height);

  ID3D11ShaderResourceView *texture_srv;
  {
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
  }

  D3D11_BLEND_DESC desc = {};
  desc.RenderTarget[0].BlendEnable           = true;
  desc.RenderTarget[0].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
  desc.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
  desc.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
  desc.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
  desc.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ZERO;
  desc.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
  desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

  ID3D11BlendState *blend_state;
  device->CreateBlendState(&desc, &blend_state);

  char *shader =
  R"(
    static const float2 uv_values[4] =
    {
      float2(0.0f, 1.0f), // lower left
      float2(0.0f, 0.0f), // upper left
      float2(1.0f, 0.0f), // upper right
      float2(1.0f, 1.0f), // lower right
    };

    static const float indices[6] =
    {
      0, 1, 2,
      0, 2, 3,
    };

    struct Vertex
    {
      float4 position : POSITION;
      float2 size     : SIZE;
    };

    struct Geometry
    {
      float4 position : POSITION;
      float2 size     : SIZE;
    };

    struct Pixel
    {
      float4 position  : SV_Position;
      float2 tex_coord : TEXCOORD;
    };

    Texture2D    mytexture : register(t0);
    SamplerState mysampler : register(s0);

    Geometry vs_main(Vertex v)
    {
      Geometry g;

      g.position = v.position;
      g.size     = v.size;

      return g;
    }

    [maxvertexcount(6)]
    void gs_main(point Geometry IN[1], inout TriangleStream<Pixel> triangle_stream)
    {
      Pixel p = (Pixel)0;

      float2 halfSize = IN[0].size / 2.0f;
      float3 right = float3(1.0f, 0.0f, 0.0f);
      float3 cameraUp = float3(0.0f, 1.0f, 0.0f);

      float3 offsetX = halfSize.x * right;
      float3 offsetY = halfSize.y * cameraUp;

      float4 vertices[4];
      vertices[0] = float4(IN[0].position.xyz + offsetX - offsetY, 1.0f); // lower-left
      vertices[1] = float4(IN[0].position.xyz + offsetX + offsetY, 1.0f); // upper-left
      vertices[2] = float4(IN[0].position.xyz - offsetX + offsetY, 1.0f); // upper-right
      vertices[3] = float4(IN[0].position.xyz - offsetX - offsetY, 1.0f); // lower-right

      for(int i = 0; i < 6; i++)
      {
        p.position  = vertices[indices[i]];
        p.tex_coord = uv_values[indices[i]];
        triangle_stream.Append(p);
        if((i + 1) % 3 == 0) triangle_stream.RestartStrip();
      }
    }

    float4 ps_main(Pixel p) : SV_Target
    {
      float4 sample = mytexture.Sample(mysampler, p.tex_coord);
      float d = sample.r - 0.1;
      float aaf = fwidth(d);
      float alpha = smoothstep(0.5f - aaf, 0.5f + aaf, d);
      // float alpha = 0.1;
      return float4(sample.rgb, alpha);
      // return sample;
      // return float4(p.tex_coord, 0.0f, 1.0f);
    }
  )";

  size_t shader_length = strlen(shader);

  ID3DBlob *vertex_shader_blob;
  ID3DBlob *geometry_shader_blob;
  ID3DBlob *pixel_shader_blob;
  
  ID3D11VertexShader *vertex_shader;
  ID3D11GeometryShader *geometry_shader;
  ID3D11PixelShader *pixel_shader;

  D3DCompile(shader, shader_length, NULL, NULL, NULL, "vs_main", "vs_5_0", 0, 0, &vertex_shader_blob, NULL);
  D3DCompile(shader, shader_length, NULL, NULL, NULL, "gs_main", "gs_5_0", 0, 0, &geometry_shader_blob, NULL);
  D3DCompile(shader, shader_length, NULL, NULL, NULL, "ps_main", "ps_5_0", 0, 0, &pixel_shader_blob, NULL);

  device->CreateVertexShader(vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(), NULL, &vertex_shader);
  device->CreateGeometryShader(geometry_shader_blob->GetBufferPointer(), geometry_shader_blob->GetBufferSize(), NULL, &geometry_shader);
  device->CreatePixelShader(pixel_shader_blob->GetBufferPointer(), pixel_shader_blob->GetBufferSize(), NULL, &pixel_shader);

  ID3D11InputLayout *input_layout;
  D3D11_INPUT_ELEMENT_DESC input_element_desc[] =
  {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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

    device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    device_context->IASetInputLayout(input_layout);
    device_context->IASetVertexBuffers(0, 1, &my_vertices_buffer, &my_vertices_stride, &my_offset);
    
    device_context->VSSetShader(vertex_shader, NULL, 0);
    
    device_context->RSSetViewports(1, &viewport);
    device_context->RSSetState(rasterizer_state);

    device_context->GSSetShader(geometry_shader, NULL, 0);
    device_context->PSSetShader(pixel_shader, NULL, 0);
    device_context->PSSetShaderResources(0, 1, &texture_srv);
    
    device_context->OMSetBlendState(blend_state, null, 0xFFFFFFFF);
    device_context->OMSetRenderTargets(1, &rtv, NULL);

    device_context->Draw(my_vertices_count, 0);

    swap_chain->Present(1, 0);
  }

  return 0;
}

u8* decode_image(char *file_name, u32 *width, u32 *height)
{
  u8 *data = null;
  HRESULT hr;

  hr = CoInitializeEx(null, COINIT_MULTITHREADED);
  if(hr != S_OK) goto end;

  IWICImagingFactory *wic_factory = null;
  hr = CoCreateInstance(CLSID_WICImagingFactory, null, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (void**)&wic_factory);
  if(hr != S_OK) goto end;
  
  IWICBitmapDecoder *wic_decoder;

  s32 file_name_length = (s32)strlen(file_name);
  s32 size = MultiByteToWideChar(CP_UTF8, 0, file_name, file_name_length, null, 0);

  wchar *file_name16 = (wchar*)malloc((size + 1) * sizeof(wchar));
  MultiByteToWideChar(CP_UTF8, 0, file_name, file_name_length, file_name16, size);
  file_name16[size] = L'\0';

  hr = wic_factory->CreateDecoderFromFilename(file_name16, &GUID_VendorMicrosoftBuiltIn, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &wic_decoder);
  if(hr != S_OK) goto end;

  IWICBitmapFrameDecode *wic_bitmap = null;
  hr = wic_decoder->GetFrame(0, &wic_bitmap);
  if(hr != S_OK) goto end;

  IWICFormatConverter *wic_converter = null;
  hr = wic_factory->CreateFormatConverter(&wic_converter);
  if(hr != S_OK) goto end;

  hr = wic_converter->Initialize(wic_bitmap, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, null, 0, WICBitmapPaletteTypeMedianCut);
  if(hr != S_OK) goto end;

  hr = wic_converter->GetSize(width, height);
  if(hr != S_OK) goto end;

  u32 bytes_per_pixel = 4;
  u32 stride = (*width) * bytes_per_pixel;
  u32 data_size = stride * (*height);

  data = (u8*)malloc(data_size);

  hr = wic_converter->CopyPixels(null, stride, data_size, data);
  if(hr != S_OK) goto end;

  end:
  return data;
}