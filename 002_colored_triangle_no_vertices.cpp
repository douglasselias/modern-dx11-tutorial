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
  char *title = "002 - Single color triangle";

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
  swap_chain_desc.SwapEffect        = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  
  D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0 };
  
  D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, feature_levels, ARRAYSIZE(feature_levels), D3D11_SDK_VERSION, &swap_chain_desc, &swap_chain, &device, NULL, &device_context);

  swap_chain->GetDesc(&swap_chain_desc);

  viewport.Width    = (float)swap_chain_desc.BufferDesc.Width;
  viewport.Height   = (float)swap_chain_desc.BufferDesc.Height;
  viewport.MaxDepth = 1;

  ID3D11RenderTargetView *rtv;
  ID3D11Texture2D *rtv_texture;
  swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&rtv_texture);
  
  D3D11_RENDER_TARGET_VIEW_DESC rtv_desc = {};
  rtv_desc.Format        = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
  rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
  
  device->CreateRenderTargetView(rtv_texture, &rtv_desc, &rtv);

  float background_color[4] = {0, 0, 0, 1};

  char *shader =
  R"(
    struct Pixel
    {
      float4 position : SV_POSITION;
      float4 color    : COLOR;
    };

    Pixel vs_main(uint vid : SV_VERTEXID)
    { 
      Pixel output;
      output.position.z = 0;
      output.position.w = 1;
      output.color.rgb = 0;
      output.color.a = 1;

      if(vid == 0)
      {
        output.position.xy = float2(-0.5, -0.5);
        output.color.r = 1;
      }
      else if(vid == 1)
      {
        output.position.xy = float2(0, 0.5);
        output.color.g = 1;
      }
      else if(vid == 2)
      {
        output.position.xy = float2(0.5, -0.5);
        output.color.b = 1;
      }

      return output;
    }

    float4 ps_main(Pixel input) : SV_TARGET 
    { 
      return input.color;
    }
  )";

  size_t shader_length = strlen(shader);

  ID3D11VertexShader *vertex_shader;
  ID3DBlob *vertex_shader_blob;

  ID3D11PixelShader *pixel_shader;
  ID3DBlob *pixel_shader_blob;

  D3DCompile(shader, shader_length, NULL, NULL, NULL, "vs_main", "vs_5_0", 0, 0, &vertex_shader_blob, NULL);

  D3DCompile(shader, shader_length, NULL, NULL, NULL, "ps_main", "ps_5_0", 0, 0, &pixel_shader_blob, NULL);

  device->CreateVertexShader(vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(), NULL, &vertex_shader);

  device->CreatePixelShader(pixel_shader_blob->GetBufferPointer(), pixel_shader_blob->GetBufferSize(), NULL, &pixel_shader);

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

    device_context->VSSetShader(vertex_shader, NULL, 0);
    
    device_context->RSSetViewports(1, &viewport);
    device_context->RSSetState(rasterizer_state);

    device_context->PSSetShader(pixel_shader, NULL, 0);

    device_context->OMSetRenderTargets(1, &rtv, NULL);

    device_context->Draw(3, 0);

    swap_chain->Present(1, 0);
  }

  return 0;
}