#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
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

#define clamp(x, x_min, x_max) min(max(x, x_min), x_max)

s32 WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, s32)
{
  char *window_title = "Simple Tessellator";

  WNDCLASS window_class = {};
  window_class.lpfnWndProc   = DefWindowProc;
  window_class.lpszClassName = window_title;
  window_class.hCursor       = LoadCursor(null, IDC_ARROW);

  RegisterClass(&window_class);

  u32 window_size = 50;
  u32 window_flags = WS_POPUP | WS_VISIBLE | WS_MAXIMIZE;
  HWND window = CreateWindow(window_title, window_title, window_flags, 0, 0, window_size, window_size, null, null, null, null);

  RECT window_rect;
  GetClientRect(window, &window_rect);
  // u32 window_width  = window_rect.right;
  // u32 window_height = window_rect.bottom;

  // ---- DX11 init ----

  IDXGISwapChain *swap_chain;
  ID3D11Device *device;
  ID3D11DeviceContext *dc;
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

  D3D11CreateDeviceAndSwapChain(null, D3D_DRIVER_TYPE_HARDWARE, null, D3D11_CREATE_DEVICE_DEBUG, feature_levels, ARRAYSIZE(feature_levels), D3D11_SDK_VERSION, &swap_chain_desc, &swap_chain, &device, null, &dc);

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

  f32 background_color[4] = { 0.1f, 0.1f, 0.15f, 1 };

  // ---- Triangle vertices (3 control points for the patch) ----

  f32 vertices[] =
  {
  //  x      y      r     g     b
     0.0f,  0.7f,  1.0f, 0.0f, 0.0f,  // top
    -0.7f, -0.5f,  0.0f, 1.0f, 0.0f,  // bottom-left
     0.7f, -0.5f,  0.0f, 0.0f, 1.0f,  // bottom-right
  };

  u32 vertex_stride = sizeof(f32) * 5;
  u32 vertex_offset = 0;

  D3D11_BUFFER_DESC vb_desc = {};
  vb_desc.ByteWidth = sizeof(vertices);
  vb_desc.Usage     = D3D11_USAGE_IMMUTABLE;
  vb_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

  D3D11_SUBRESOURCE_DATA vb_data = { vertices };
  ID3D11Buffer *vertex_buffer;
  device->CreateBuffer(&vb_desc, &vb_data, &vertex_buffer);

  // ---- Constant buffer (tessellation factor) ----

  struct Constants
  {
    f32 tess_factor;
    f32 inside_factor;
    f32 pad0, pad1;
  };

  Constants constants = { 1.0f, 2.0f, 0, 0 };

  D3D11_BUFFER_DESC cb_desc = {};
  cb_desc.ByteWidth      = sizeof(Constants);
  cb_desc.Usage          = D3D11_USAGE_DYNAMIC;
  cb_desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
  cb_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

  D3D11_SUBRESOURCE_DATA cb_data = { &constants };
  ID3D11Buffer *constant_buffer;
  device->CreateBuffer(&cb_desc, &cb_data, &constant_buffer);

  // ---- Shaders ----

  char *shader =
  R"(
    cbuffer Constants : register(b0)
    {
      float TessFactor;
      float InsideFactor;
    };

    // ==== Vertex Shader ====
    // Just passes data through to the hull shader.

    struct VS_Input
    {
      float2 position : POS;
      float3 color    : COL;
    };

    struct VS_Output
    {
      float2 position : POS;
      float3 color    : COL;
    };

    VS_Output vs_main(VS_Input input)
    {
      VS_Output output;
      output.position = input.position;
      output.color    = input.color;
      return output;
    }

    // ==== Hull Shader ====
    // Defines HOW MUCH to tessellate.
    // Two parts: per-control-point function + patch constant function.

    struct HS_Output
    {
      float2 position : POS;
      float3 color    : COL;
    };

    struct PatchConstants
    {
      float edges[3]  : SV_TessFactor;
      float inside[1] : SV_InsideTessFactor;
    };

    // This runs once per patch. Sets tessellation levels.
    PatchConstants patch_constant_func(InputPatch<VS_Output, 3> patch)
    {
      PatchConstants pc;
      pc.edges[0] = TessFactor;   // edge opposite vertex 0
      pc.edges[1] = TessFactor;   // edge opposite vertex 1
      pc.edges[2] = TessFactor;   // edge opposite vertex 2
      pc.inside[0] = InsideFactor; // inside
      return pc;
    }

    // This runs once per control point (3 times for a triangle).
    [domain("tri")]
    [partitioning("integer")]
    [outputtopology("triangle_cw")]
    [outputcontrolpoints(3)]
    [patchconstantfunc("patch_constant_func")]
    HS_Output hs_main(InputPatch<VS_Output, 3> patch, uint id : SV_OutputControlPointID)
    {
      HS_Output output;
      output.position = patch[id].position;
      output.color    = patch[id].color;
      return output;
    }

    // ==== Domain Shader ====
    // Runs for EACH tessellated vertex.
    // Receives barycentric coordinates (uvw) and interpolates.

    struct DS_Output
    {
      float4 position : SV_POSITION;
      float3 color    : COL;
    };

    [domain("tri")]
    DS_Output ds_main(PatchConstants pc, float3 uvw : SV_DomainLocation, const OutputPatch<HS_Output, 3> patch)
    {
      DS_Output output;

      // Interpolate position using barycentric coordinates
      float2 pos = patch[0].position * uvw.x
                 + patch[1].position * uvw.y
                 + patch[2].position * uvw.z;

      // Interpolate color
      output.color = patch[0].color * uvw.x
                   + patch[1].color * uvw.y
                   + patch[2].color * uvw.z;

      output.position = float4(pos, 0, 1);
      return output;
    }

    // ==== Pixel Shader ====

    float4 ps_main(DS_Output input) : SV_TARGET
    {
      return float4(input.color, 1);
    }
  )";

  u64 shader_length = strlen(shader);

  ID3DBlob *vs_blob, *hs_blob, *ds_blob, *ps_blob;
  ID3DBlob *error_blob;

  D3DCompile(shader, shader_length, null, null, null, "vs_main", "vs_5_0", 0, 0, &vs_blob, &error_blob);
  if(error_blob) { OutputDebugStringA((char*)error_blob->GetBufferPointer()); error_blob->Release(); error_blob = null; }

  D3DCompile(shader, shader_length, null, null, null, "hs_main", "hs_5_0", 0, 0, &hs_blob, &error_blob);
  if(error_blob) { OutputDebugStringA((char*)error_blob->GetBufferPointer()); error_blob->Release(); error_blob = null; }

  D3DCompile(shader, shader_length, null, null, null, "ds_main", "ds_5_0", 0, 0, &ds_blob, &error_blob);
  if(error_blob) { OutputDebugStringA((char*)error_blob->GetBufferPointer()); error_blob->Release(); error_blob = null; }

  D3DCompile(shader, shader_length, null, null, null, "ps_main", "ps_5_0", 0, 0, &ps_blob, &error_blob);
  if(error_blob) { OutputDebugStringA((char*)error_blob->GetBufferPointer()); error_blob->Release(); error_blob = null; }

  ID3D11VertexShader  *vertex_shader;
  ID3D11HullShader    *hull_shader;
  ID3D11DomainShader  *domain_shader;
  ID3D11PixelShader   *pixel_shader;

  device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), null, &vertex_shader);
  device->CreateHullShader(hs_blob->GetBufferPointer(), hs_blob->GetBufferSize(), null, &hull_shader);
  device->CreateDomainShader(ds_blob->GetBufferPointer(), ds_blob->GetBufferSize(), null, &domain_shader);
  device->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), null, &pixel_shader);

  D3D11_INPUT_ELEMENT_DESC input_desc[] =
  {
    { "POS", 0, DXGI_FORMAT_R32G32_FLOAT,       0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COL", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };

  ID3D11InputLayout *input_layout;
  device->CreateInputLayout(input_desc, ARRAYSIZE(input_desc), vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), &input_layout);

  // Wireframe to see the tessellation
  D3D11_RASTERIZER_DESC raster_desc = {};
  raster_desc.FillMode = D3D11_FILL_WIREFRAME;
  raster_desc.CullMode = D3D11_CULL_NONE;

  ID3D11RasterizerState *rasterizer_wireframe;
  device->CreateRasterizerState(&raster_desc, &rasterizer_wireframe);

  raster_desc.FillMode = D3D11_FILL_SOLID;
  ID3D11RasterizerState *rasterizer_solid;
  device->CreateRasterizerState(&raster_desc, &rasterizer_solid);

  bool wireframe = true;

  // ---- Main loop ----

  for(bool running = true; running;)
  {
    MSG msg;
    if(PeekMessage(&msg, null, 0, 0, PM_REMOVE)) DispatchMessage(&msg);
    if(GetAsyncKeyState(VK_ESCAPE) & 0x8000) break;

    // Up/Down arrows to change tessellation factor
    if(GetAsyncKeyState(VK_UP) & 1)
    {
      constants.tess_factor += 1.0f;
    }
      
    if(GetAsyncKeyState(VK_RIGHT) & 1)
    {
      constants.inside_factor += 1.0f;
    }

    if(GetAsyncKeyState(VK_DOWN) & 1)
    {
      constants.tess_factor -= 1.0f;
    }

    if(GetAsyncKeyState(VK_LEFT) & 1)
    {
      constants.inside_factor -= 1.0f;
    }
    // W to toggle wireframe/solid
    if(GetAsyncKeyState('W') & 1) wireframe = !wireframe;

    constants.tess_factor   = clamp(constants.tess_factor, 1, 64);
    constants.inside_factor = clamp(constants.inside_factor, 1, 64);

    // Update constant buffer
    D3D11_MAPPED_SUBRESOURCE mapped;
    dc->Map(constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    *(Constants*)mapped.pData = constants;
    dc->Unmap(constant_buffer, 0);

    dc->ClearRenderTargetView(rtv, background_color);

    // The key difference: topology is a PATCH, not a triangle list!
    dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
    dc->IASetInputLayout(input_layout);
    dc->IASetVertexBuffers(0, 1, &vertex_buffer, &vertex_stride, &vertex_offset);

    dc->VSSetShader(vertex_shader, null, 0);
    dc->HSSetShader(hull_shader, null, 0);
    dc->HSSetConstantBuffers(0, 1, &constant_buffer);
    dc->DSSetShader(domain_shader, null, 0);
    dc->PSSetShader(pixel_shader, null, 0);

    dc->RSSetViewports(1, &viewport);
    dc->RSSetState(wireframe ? rasterizer_wireframe : rasterizer_solid);

    dc->OMSetRenderTargets(1, &rtv, null);

    dc->Draw(3, 0);

    swap_chain->Present(1, 0);
  }

  return 0;
}