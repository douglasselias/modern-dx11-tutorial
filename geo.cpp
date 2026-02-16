#define _USE_MATH_DEFINES
#include <math.h>
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
  char *title = "Modern DX11";

  WNDCLASS window_class = {};
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

  ID3D11RenderTargetView *rtv;
  ID3D11Texture2D* rtv_texture;
  swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&rtv_texture);
  
  D3D11_RENDER_TARGET_VIEW_DESC rtv_desc = {};
  rtv_desc.Format        = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
  rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
  
  device->CreateRenderTargetView(rtv_texture, &rtv_desc, &rtv);

  float background_color[4] = {0, 0, 0, 1};

  float my_vertices[] =
  {
    -0.5, -0.75, 0, 1, 1, 1,
     0.0,  0.75, 0, 1, 1, 1,
     0.5, -0.75, 0, 1, 1, 1,
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

  char *shader =
  R"(
    static const float2 uv_values[4] =
    {
      float2(0.0f, 1.0f), // lower left
      float2(0.0f, 0.0f), // upper left
      float2(1.0f, 0.0f), // upper right
      float2(1.0f, 1.0f), // lower right
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

    Geometry vs_main(Vertex v)
    {
      Geometry g = (Geometry)0;

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

      // tri: 0, 1, 2
      p.position = vertices[0];
      p.tex_coord = uv_values[0];
      triangle_stream.Append(p);

      p.position = vertices[1];
      p.tex_coord = uv_values[1];
      triangle_stream.Append(p);

      p.position = vertices[2];
      p.tex_coord = uv_values[2];
      triangle_stream.Append(p);
      triangle_stream.RestartStrip();

      // tri: 0, 2, 3
      p.position = vertices[0];
      p.tex_coord = uv_values[0];
      triangle_stream.Append(p);

      p.position = vertices[2];
      p.tex_coord = uv_values[2];
      triangle_stream.Append(p);

      p.position = vertices[3];
      p.tex_coord = uv_values[3];
      triangle_stream.Append(p);
    }

    float4 ps_main(Pixel p) : SV_Target
    {
      return float4(p.tex_coord, 0.0f, 1.0f);
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

    device_context->OMSetRenderTargets(1, &rtv, NULL);

    device_context->Draw(my_vertices_count, 0);

    swap_chain->Present(1, 0);
  }

  return 0;
}
