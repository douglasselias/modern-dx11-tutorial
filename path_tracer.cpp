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

union V3 { struct { f32 x, y, z; }; };

V3 operator +(V3 a, V3 b)  { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
V3 operator -(V3 a, V3 b)  { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
V3 operator *(V3 a, f32 s) { return { a.x * s, a.y * s, a.z * s }; }
V3 operator *(f32 s, V3 a) { return { a.x * s, a.y * s, a.z * s }; }
V3 operator /(V3 a, f32 s) { return { a.x / s, a.y / s, a.z / s }; }
V3 operator -(V3 a)        { return { -a.x, -a.y, -a.z }; }

f32 dot(V3 a, V3 b)   { return a.x * b.x + a.y * b.y + a.z * b.z; }
f32 length_sq(V3 a)   { return dot(a, a); }
f32 length(V3 a)      { return sqrtf(dot(a, a)); }
V3  normalize(V3 a)   { return a / length(a); }
V3  cross(V3 a, V3 b) { return { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x }; }

struct Ray    { V3 origin, direction; f32 t; };
struct Sphere { V3 center; f32 radius; };

bool hit_sphere(Ray *ray, Sphere sphere)
{
  V3  L = ray->origin - sphere.center;
  f32 a = dot(ray->direction, ray->direction);
  f32 b = 2.0f * dot(L, ray->direction);
  f32 c = dot(L, L) - sphere.radius * sphere.radius;
  f32 discriminant = b * b - 4 * a * c;

  if (discriminant < 0) return false;

  f32 t = (-b - sqrtf(discriminant)) / (2.0f * a);
  if (t < 0)
  {
    t = (-b + sqrtf(discriminant)) / (2.0f * a);
    if (t < 0) return false;
  }

  ray->t = t;
  return true;
}

s32 WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, s32)
{
  char *window_title = "Camera tracer - A.K.A. Path tracer";

  WNDCLASS window_class = {};
  window_class.lpfnWndProc   = DefWindowProc;
  window_class.lpszClassName = window_title;
  window_class.hCursor       = LoadCursor(null, IDC_ARROW);

  RegisterClass(&window_class);

  // u32 window_size = 1280;
  u32 window_flags = WS_POPUP | WS_VISIBLE;
  #if 0
  window_flags = WS_MAXIMIZE;
  #endif
  HWND window = CreateWindow(window_title, window_title, window_flags, 0, 0, 2560, 1080, null, null, null, null);

  RECT window_rect;
  GetClientRect(window, &window_rect);
  u32 window_width  = window_rect.right;
  u32 window_height = window_rect.bottom;

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

  ID3D11Texture2D *rtv_texture;
  swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&rtv_texture);
  
  D3D11_RENDER_TARGET_VIEW_DESC rtv_desc = {};
  rtv_desc.Format        = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
  rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
  
  ID3D11RenderTargetView *rtv;
  device->CreateRenderTargetView(rtv_texture, &rtv_desc, &rtv);

  f32 background_color[4] = {0, 0, 0, 1};

  f32 my_vertices[] =
  {
    -1,  1, 0, 0,
    -1, -1, 0, 1,
     1,  1, 1, 0,
     1, -1, 1, 1,
  };

  u32 my_vertices_size = ARRAYSIZE(my_vertices);
  u8 number_of_components = 4;
  u32 my_vertices_stride = sizeof(f32) * number_of_components;
  u32 my_offset = 0;

  D3D11_BUFFER_DESC vertices_buffer_desc = {};
  vertices_buffer_desc.ByteWidth = my_vertices_size * sizeof(f32);
  vertices_buffer_desc.Usage     = D3D11_USAGE_IMMUTABLE;
  vertices_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  
  D3D11_SUBRESOURCE_DATA vertices_subresource = { my_vertices }; 
  ID3D11Buffer *my_vertices_buffer;
  device->CreateBuffer(&vertices_buffer_desc, &vertices_subresource, &my_vertices_buffer);

  u32 my_indices[] =
  {
    0, 1, 2,
    1, 3, 2,
  };

  u32 my_indices_size = ARRAYSIZE(my_indices);

  D3D11_BUFFER_DESC indices_buffer_desc = {};
  indices_buffer_desc.ByteWidth = sizeof(u32) * my_indices_size;
  indices_buffer_desc.Usage     = D3D11_USAGE_IMMUTABLE;
  indices_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
  
  D3D11_SUBRESOURCE_DATA indices_subresource = { my_indices };
  ID3D11Buffer *my_indices_buffer;  
  device->CreateBuffer(&indices_buffer_desc, &indices_subresource, &my_indices_buffer);

  D3D11_TEXTURE2D_DESC compute_tex_desc = {};
  compute_tex_desc.Width            = window_width;
  compute_tex_desc.Height           = window_height;
  compute_tex_desc.MipLevels        = 1;
  compute_tex_desc.ArraySize        = 1;
  compute_tex_desc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
  compute_tex_desc.Usage            = D3D11_USAGE_DEFAULT;
  compute_tex_desc.BindFlags        = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
  compute_tex_desc.SampleDesc.Count = 1;

  ID3D11Texture2D *compute_texture;
  device->CreateTexture2D(&compute_tex_desc, null, &compute_texture);

  // UAV — for compute shader to write.
  ID3D11UnorderedAccessView *compute_uav;
  device->CreateUnorderedAccessView(compute_texture, null, &compute_uav);

  // SRV — Pixel shader read.
  ID3D11ShaderResourceView *compute_srv;
  device->CreateShaderResourceView(compute_texture, null, &compute_srv);

  Sphere sphere = {};
  sphere.radius = 1;
  sphere.center =
  {
    0,
    0,
    3,
  };

  Ray light = {};
  light.origin    = {-10, 10, -5};
  light.direction = sphere.center - light.origin;

  // Computer shader constants (TextureSize + BlueColor)
  struct ComputeConstants
  {
    f32 tex_w, tex_h, blue; f32 pad0;
    Sphere sphere; f32 pad1;
    Ray light;
  };
  
  ComputeConstants compute_constants =
  {
    (f32)window_width, (f32)window_height, 0.5f, 0,
    sphere, 0,
    light,
  };

  D3D11_BUFFER_DESC cb_desc = {};
  cb_desc.ByteWidth       = sizeof(ComputeConstants);
  cb_desc.Usage           = D3D11_USAGE_DYNAMIC;
  cb_desc.BindFlags       = D3D11_BIND_CONSTANT_BUFFER;
  cb_desc.CPUAccessFlags  = D3D11_CPU_ACCESS_WRITE;

  D3D11_SUBRESOURCE_DATA cb_data = { &compute_constants };
  ID3D11Buffer *compute_cb;
  device->CreateBuffer(&cb_desc, &cb_data, &compute_cb);
  
  char *shader =
  R"(
    struct Vertex
    {
      float2 position : POS;
      float2 texcoord : TEX;
    };

    struct Pixel
    {
      float4 position : SV_POSITION;
      float2 texcoord : TEX;
    };

    struct Ray
    {
      float3 origin;
      float3 direction;
      float t;
    };

    struct Sphere
    {
      float3 center;
      float radius;
    };

    cbuffer PerFrame
    {
      float2 TextureSize;
      float BlueColor;
      Sphere sphere;
      Ray light;
    };

    bool hit_sphere(inout Ray ray, Sphere sphere)
    {
      float3 L = ray.origin - sphere.center;
      float a = dot(ray.direction, ray.direction);
      float b = 2 * dot(L, ray.direction);
      float c = dot(L, L) - sphere.radius * sphere.radius;
      float discriminant = b * b - 4 * a * c;

      if(discriminant < 0) return false;

      float t = (-b - sqrt(discriminant)) / (2 * a);

      if(t < 0)
      {
        t = (-b + sqrt(discriminant)) / (2 * a);
        if(t < 0) return false;
      }

      ray.t = t;
      return true;
    }
      
    RWTexture2D<float4> out_texture;
    Texture2D    mytexture : register(t0);
    SamplerState mysampler : register(s0);

    Pixel vs_main(Vertex v)
    {
      Pixel output;
      output.position = float4(v.position, 0, 1);
      output.texcoord = v.texcoord;
      return output;
    }

    [numthreads(32, 32, 1)]
    void cs_main(uint3 thread_id : SV_DispatchThreadID)
    {
      float2 uv = (thread_id.xy + 0.5) / TextureSize;

      float2 ndc = uv * 2 - 1;
      ndc.y = -ndc.y;

      float aspect = TextureSize.x / TextureSize.y;
      float fov = radians(90);
      float half_fov = tan(fov * 0.5);
      ndc.x *= aspect * half_fov;
      ndc.y *= half_fov;

      float4 color = float4(uv, BlueColor, 1);

      Ray ray;
      ray.origin = float3(0, 0, 0);
      ray.direction = normalize(float3(ndc, 1));

      if(hit_sphere(ray, sphere))
      {
        float3 hit_point = ray.origin + ray.direction * ray.t;
        float3 normal = normalize(hit_point - sphere.center);
        float diffuse = saturate(dot(normal, -normalize(light.direction)));
        float ambient = 0.05;
        color = float4(1, 0, 0, 1) * (ambient + diffuse);
      }
      else
      {
        // color = float4(1, 1, 1, 1);
        color = float4(0.1, 0.1, 0.1, 1);
      }

      out_texture[thread_id.xy] = color;
    }

    float4 ps_main(Pixel p) : SV_TARGET
    {
      return mytexture.Sample(mysampler, p.texcoord) * 0.8;
    }
  )";


  u64 shader_length = strlen(shader);

  ID3DBlob *vertex_shader_blob;
  ID3DBlob *compute_shader_blob;
  ID3DBlob *pixel_shader_blob;

  D3DCompile(shader, shader_length, null, null, null, "vs_main", "vs_5_0", 0, 0, &vertex_shader_blob, null);
  D3DCompile(shader, shader_length, null, null, null, "cs_main", "cs_5_0", 0, 0, &compute_shader_blob, null);
  D3DCompile(shader, shader_length, null, null, null, "ps_main", "ps_5_0", 0, 0, &pixel_shader_blob, null);

  ID3D11VertexShader *vertex_shader;
  ID3D11ComputeShader *compute_shader;
  ID3D11PixelShader *pixel_shader;

  device->CreateVertexShader(vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(), null, &vertex_shader);
  device->CreateComputeShader(compute_shader_blob->GetBufferPointer(), compute_shader_blob->GetBufferSize(), null, &compute_shader);
  device->CreatePixelShader(pixel_shader_blob->GetBufferPointer(), pixel_shader_blob->GetBufferSize(), null, &pixel_shader);

  ID3D11InputLayout *input_layout;
  D3D11_INPUT_ELEMENT_DESC input_element_desc[] =
  {
    { "POS", 0, DXGI_FORMAT_R32G32_FLOAT, 0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };

  device->CreateInputLayout(input_element_desc, ARRAYSIZE(input_element_desc), vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(), &input_layout);
  
  D3D11_RASTERIZER_DESC rasterizer_desc = { D3D11_FILL_SOLID, D3D11_CULL_NONE };
  ID3D11RasterizerState *rasterizer_state;
  device->CreateRasterizerState(&rasterizer_desc, &rasterizer_state);

  for(bool running = true; running;)
  {
    MSG msg;
    if(PeekMessage(&msg, null, 0, 0, PM_REMOVE)) DispatchMessage(&msg);
    if(GetAsyncKeyState(VK_ESCAPE) & 0x8000) break;

    bool light_moved = false;
    const f32 light_step = 0.1f;
    if(GetAsyncKeyState(VK_LEFT)  & 0x8000) { compute_constants.light.origin.x -= light_step; light_moved = true; }
    if(GetAsyncKeyState(VK_RIGHT) & 0x8000) { compute_constants.light.origin.x += light_step; light_moved = true; }
    if(GetAsyncKeyState(VK_UP)    & 0x8000) { compute_constants.light.origin.y += light_step; light_moved = true; }
    if(GetAsyncKeyState(VK_DOWN)  & 0x8000) { compute_constants.light.origin.y -= light_step; light_moved = true; }

    if(light_moved)
    {
      compute_constants.light.direction = compute_constants.sphere.center - compute_constants.light.origin;

      D3D11_MAPPED_SUBRESOURCE mapped = {};
      device_context->Map(compute_cb, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
      *(ComputeConstants*)mapped.pData = compute_constants;
      device_context->Unmap(compute_cb, 0);
    }

    device_context->ClearRenderTargetView(rtv, background_color);

    device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    device_context->IASetInputLayout(input_layout);
    device_context->IASetVertexBuffers(0, 1, &my_vertices_buffer, &my_vertices_stride, &my_offset);
    device_context->IASetIndexBuffer(my_indices_buffer, DXGI_FORMAT_R32_UINT, 0);
    
    device_context->VSSetShader(vertex_shader, null, 0);
    
    device_context->RSSetViewports(1, &viewport);
    device_context->RSSetState(rasterizer_state);

    device_context->CSSetShader(compute_shader, null, 0);
    device_context->CSSetUnorderedAccessViews(0, 1, &compute_uav, null);
    device_context->CSSetConstantBuffers(0, 1, &compute_cb);
    device_context->Dispatch((window_width + 31) / 32, (window_height + 31) / 32, 1); // ceil(window_width/32) x ceil(window_height/32)

    // Desvincula o UAV para poder usar como SRV
    ID3D11UnorderedAccessView *null_uav = null;
    device_context->CSSetUnorderedAccessViews(0, 1, &null_uav, null);

    device_context->PSSetShader(pixel_shader, null, 0);
    device_context->PSSetShaderResources(0, 1, &compute_srv);

    device_context->OMSetRenderTargets(1, &rtv, null);

    device_context->DrawIndexed(my_indices_size, 0, 0);

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