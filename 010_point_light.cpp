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

struct V3 { f32 x, y, z; };
struct Matrix { f32 m[4][4]; };
struct Constants
{
  Matrix transform;
  Matrix model;
  V3 light_position;
  float pad0;
  ////
  V3 viewPos;
};

Matrix operator*(const Matrix& m1, const Matrix& m2)
{
  return
  {
    m1.m[0][0] * m2.m[0][0] + m1.m[0][1] * m2.m[1][0] + m1.m[0][2] * m2.m[2][0] + m1.m[0][3] * m2.m[3][0],
    m1.m[0][0] * m2.m[0][1] + m1.m[0][1] * m2.m[1][1] + m1.m[0][2] * m2.m[2][1] + m1.m[0][3] * m2.m[3][1],
    m1.m[0][0] * m2.m[0][2] + m1.m[0][1] * m2.m[1][2] + m1.m[0][2] * m2.m[2][2] + m1.m[0][3] * m2.m[3][2],
    m1.m[0][0] * m2.m[0][3] + m1.m[0][1] * m2.m[1][3] + m1.m[0][2] * m2.m[2][3] + m1.m[0][3] * m2.m[3][3],
    m1.m[1][0] * m2.m[0][0] + m1.m[1][1] * m2.m[1][0] + m1.m[1][2] * m2.m[2][0] + m1.m[1][3] * m2.m[3][0],
    m1.m[1][0] * m2.m[0][1] + m1.m[1][1] * m2.m[1][1] + m1.m[1][2] * m2.m[2][1] + m1.m[1][3] * m2.m[3][1],
    m1.m[1][0] * m2.m[0][2] + m1.m[1][1] * m2.m[1][2] + m1.m[1][2] * m2.m[2][2] + m1.m[1][3] * m2.m[3][2],
    m1.m[1][0] * m2.m[0][3] + m1.m[1][1] * m2.m[1][3] + m1.m[1][2] * m2.m[2][3] + m1.m[1][3] * m2.m[3][3],
    m1.m[2][0] * m2.m[0][0] + m1.m[2][1] * m2.m[1][0] + m1.m[2][2] * m2.m[2][0] + m1.m[2][3] * m2.m[3][0],
    m1.m[2][0] * m2.m[0][1] + m1.m[2][1] * m2.m[1][1] + m1.m[2][2] * m2.m[2][1] + m1.m[2][3] * m2.m[3][1],
    m1.m[2][0] * m2.m[0][2] + m1.m[2][1] * m2.m[1][2] + m1.m[2][2] * m2.m[2][2] + m1.m[2][3] * m2.m[3][2],
    m1.m[2][0] * m2.m[0][3] + m1.m[2][1] * m2.m[1][3] + m1.m[2][2] * m2.m[2][3] + m1.m[2][3] * m2.m[3][3],
    m1.m[3][0] * m2.m[0][0] + m1.m[3][1] * m2.m[1][0] + m1.m[3][2] * m2.m[2][0] + m1.m[3][3] * m2.m[3][0],
    m1.m[3][0] * m2.m[0][1] + m1.m[3][1] * m2.m[1][1] + m1.m[3][2] * m2.m[2][1] + m1.m[3][3] * m2.m[3][1],
    m1.m[3][0] * m2.m[0][2] + m1.m[3][1] * m2.m[1][2] + m1.m[3][2] * m2.m[2][2] + m1.m[3][3] * m2.m[3][2],
    m1.m[3][0] * m2.m[0][3] + m1.m[3][1] * m2.m[1][3] + m1.m[3][2] * m2.m[2][3] + m1.m[3][3] * m2.m[3][3],
  };
}

#undef near
#undef far

Matrix create_x_axis_rotation_matrix(f32 angle)
{
  f32 sine   = sinf(angle);
  f32 cosine = cosf(angle);

  return
  {
    1, 0,       0,      0,
    0, cosine, -sine,   0,
    0, sine,    cosine, 0,
    0, 0,       0,      1,
  };
}

Matrix create_y_axis_rotation_matrix(f32 angle)
{
  f32 sine   = sinf(angle);
  f32 cosine = cosf(angle);

  return
  {
    cosine, 0, sine,   0,
    0,      1, 0,      0,
    -sine,  0, cosine, 0,
    0,      0, 0,      1,
  };
}

Matrix create_z_axis_rotation_matrix(f32 angle)
{
  f32 sine   = sinf(angle);
  f32 cosine = cosf(angle);

  return
  {
    cosine, -sine,   0, 0,
    sine,    cosine, 0, 0,
    0,       0,      1, 0,
    0,       0,      0, 1,
  };
}

Matrix create_translation_matrix(V3 translation_vector)
{
  return
  {
    1,                    0,                    0,                    0,
    0,                    1,                    0,                    0,
    0,                    0,                    1,                    0,
    translation_vector.x, translation_vector.y, translation_vector.z, 1,
  };
}

#define V3_OP_V3(op) \
V3 operator op(V3 lhs, V3 rhs) \
{ \
  return {lhs.x op rhs.x, lhs.y op rhs.y, lhs.z op rhs.z}; \
} \

V3_OP_V3(+)
V3_OP_V3(-)
V3_OP_V3(*)
V3_OP_V3(/)

#define MUTABLE_V3_OP_V3(op) \
V3 operator op(V3 &lhs, V3 rhs) \
{ \
  lhs.x op rhs.x; \
  lhs.y op rhs.y; \
  lhs.z op rhs.z; \
  return lhs; \
} \

MUTABLE_V3_OP_V3(+=)
MUTABLE_V3_OP_V3(-=)
MUTABLE_V3_OP_V3(*=)
MUTABLE_V3_OP_V3(/=)

#define V3_OP_VALUE(type, op) \
V3 operator op(V3 lhs, type value) \
{ \
  return {lhs.x op value, lhs.y op value, lhs.z op value}; \
} \
V3 operator op(type value, V3 lhs) \
{ \
  return {value op lhs.x, value op lhs.y, value op lhs.z}; \
} \

V3_OP_VALUE(f32, +)
V3_OP_VALUE(f32, -)
V3_OP_VALUE(f32, *)
V3_OP_VALUE(f32, /)

#define MUTABLE_V3_OP_VALUE(type, op) \
V3 operator op(V3 &lhs, type value) \
{ \
  lhs.x op value; \
  lhs.y op value; \
  lhs.z op value; \
  return lhs; \
} \

MUTABLE_V3_OP_VALUE(f32, +=)
MUTABLE_V3_OP_VALUE(f32, -=)
MUTABLE_V3_OP_VALUE(f32, *=)
MUTABLE_V3_OP_VALUE(f32, /=)

f32 dot(V3 v1, V3 v2)
{
  V3 v = v1 * v2;
  f32 result = v.x + v.y + v.z;
  return result;
}

V3 normalize(V3 v)
{
  f32 length = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);

  if(length != 0.0f)
  {
    f32 inverse_length = 1.0f / length;
    v.x *= inverse_length;
    v.y *= inverse_length;
    v.z *= inverse_length;
  }

  return v;
}

V3 cross(V3 v1, V3 v2)
{
  V3 result =
  {
    v1.y * v2.z - v1.z * v2.y,
    v1.z * v2.x - v1.x * v2.z,
    v1.x * v2.y - v1.y * v2.x,
  };

  return result;
}

V3 v3_rotate_by_axis_angle(V3 x, V3 axis, f32 angle)
{
  angle /= 2.0f;

  V3 w   = normalize(axis) * sinf(angle);
  V3 wv  = cross(w, x)     * (cosf(angle) * 2);
  V3 wwv = cross(w, wv)    * 2;

  V3 result = x + wv + wwv;

  return result;
}

struct Camera { V3 position, target; };
V3 up_base = {0, 1, 0};

void camera_yaw(Camera* camera, f32 angle)
{
  V3 target_position = camera->target - camera->position;

  target_position = v3_rotate_by_axis_angle(target_position, up_base, angle);

  camera->target = camera->position + target_position;
}

void camera_pitch(Camera* camera, f32 angle)
{
  V3 target_position = camera->target - camera->position;
  V3 forward = normalize(target_position);
  V3 right = normalize(cross(up_base, forward));

  target_position = v3_rotate_by_axis_angle(target_position, right, angle);
  camera->target = camera->position + target_position;
}

void update_camera(Camera* camera, f32 dt)
{
  V3 forward = normalize(camera->target - camera->position);
  V3 right   = normalize(cross(forward, up_base));

  f32 movement_speed = 4.75f * dt;

  // Move forward
  if(GetAsyncKeyState('W') & 0x8000)
  {
    V3 movement = forward * -movement_speed;
    camera->position = camera->position + movement;
    camera->target   = camera->target   + movement;
  }

  // Move backward
  if(GetAsyncKeyState('S') & 0x8000)
  {
    V3 movement = forward * movement_speed;
    camera->position = camera->position + movement;
    camera->target   = camera->target   + movement;
  }

  // Strafe left
  if(GetAsyncKeyState('A') & 0x8000)
  {
    V3 movement = right * -movement_speed;
    camera->position = camera->position + movement;
    camera->target   = camera->target   + movement;
  }
  
  // Strafe right
  if (GetAsyncKeyState('D') & 0x8000)
  {
    V3 movement = right * movement_speed;
    camera->position = camera->position + movement;
    camera->target   = camera->target   + movement;
  }

  // Move up
  if (GetAsyncKeyState('Q') & 0x8000) 
  {
    V3 movement = up_base * movement_speed;
    camera->position = camera->position + movement;
    camera->target   = camera->target   + movement;
  }
  
  // Move down
  if(GetAsyncKeyState('E') & 0x8000)
  {
    V3 movement = up_base * -movement_speed;
    camera->position = camera->position + movement;
    camera->target   = camera->target   + movement;
  }

  f32 rotation_speed = 1;
  f32 rotation_amount = rotation_speed * dt;

  // Look left
  if(GetAsyncKeyState('J') & 0x8000)
  {
    camera_yaw(camera, -rotation_amount);
  }

  // Look right
  if(GetAsyncKeyState('L') & 0x8000)
  {
    camera_yaw(camera, rotation_amount);
  }

  // Look down
  if(GetAsyncKeyState('I') & 0x8000)
  {
    camera_pitch(camera, rotation_amount);
  }

  // Look up
  if(GetAsyncKeyState('K') & 0x8000)
  {
    camera_pitch(camera, -rotation_amount);
  }
}

Matrix create_view_matrix(V3 position, V3 target, V3 up)
{
  V3 zaxis = normalize(position - target);
  V3 xaxis = normalize(cross(up, zaxis));
  V3 yaxis = cross(zaxis, xaxis);

  return
  {
    xaxis.x,                yaxis.x,               zaxis.x,              0,
    xaxis.y,                yaxis.y,               zaxis.y,              0,
    xaxis.z,                yaxis.z,               zaxis.z,              0,
    -dot(xaxis, position), -dot(yaxis, position), -dot(zaxis, position), 1,
  };
}

Matrix create_projection_matrix(f32 aspect, f32 near, f32 far)
{
  // Taken from https://gist.github.com/d7samurai/261c69490cce0620d0bfc93003cd1052#file-cpu-cpp-L274
  return
  {
    2 * near / aspect, 0,        0,                         0,
    0,                 2 * near, 0,                         0,
    0,                 0,        far / (far - near),        1,
    0,                 0,        near * far / (near - far), 0,
  };
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
  char* title = "Modern DX11 Tutorial - Part 9 - Light";

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

  ID3D11DepthStencilView* dsv;
  {
    D3D11_TEXTURE2D_DESC depth_texture_desc = {};
    depth_texture_desc.Width            = (u32)viewport.Width;
    depth_texture_desc.Height           = (u32)viewport.Height;
    depth_texture_desc.MipLevels        = 1;
    depth_texture_desc.ArraySize        = 1;
    depth_texture_desc.Format           = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depth_texture_desc.SampleDesc.Count = 1;
    depth_texture_desc.Usage            = D3D11_USAGE_DEFAULT;
    depth_texture_desc.BindFlags        = D3D11_BIND_DEPTH_STENCIL;

    ID3D11Texture2D* depth_texture;
    device->CreateTexture2D(&depth_texture_desc, null, &depth_texture);
    device->CreateDepthStencilView(depth_texture, null, &dsv);
  }

  f32 background_color[4] = {0, 0, 0, 1};

  f32 my_vertices[] =
  {
    // Front face
    -3.0f, -3.0f, -3.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
     3.0f, -3.0f, -3.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,  0.0f,  0.0f, -1.0f,
     3.0f,  3.0f, -3.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
    -3.0f,  3.0f, -3.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,  0.0f,  0.0f, -1.0f,

    // Back face
     3.0f, -3.0f,  3.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,  0.0f,  0.0f,  1.0f,
    -3.0f, -3.0f,  3.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,  0.0f,  0.0f,  1.0f,
    -3.0f,  3.0f,  3.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,  0.0f,  0.0f,  1.0f,
     3.0f,  3.0f,  3.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,  0.0f,  0.0f,  1.0f,

    // Left face
    -3.0f, -3.0f,  3.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,  0.0f,  0.0f,
    -3.0f, -3.0f, -3.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f,  0.0f,  0.0f,
    -3.0f,  3.0f, -3.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
    -3.0f,  3.0f,  3.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, -1.0f,  0.0f,  0.0f,

    // Right face
     3.0f, -3.0f, -3.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,  1.0f,  0.0f,  0.0f,
     3.0f, -3.0f,  3.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,  1.0f,  0.0f,  0.0f,
     3.0f,  3.0f,  3.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,  1.0f,  0.0f,  0.0f,
     3.0f,  3.0f, -3.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,  1.0f,  0.0f,  0.0f,

    // Bottom face
    -3.0f, -3.0f,  3.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,  0.0f, -1.0f,  0.0f,
     3.0f, -3.0f,  3.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,  0.0f, -1.0f,  0.0f,
     3.0f, -3.0f, -3.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,  0.0f, -1.0f,  0.0f,
    -3.0f, -3.0f, -3.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,  0.0f, -1.0f,  0.0f,

    // Top face
    -3.0f,  3.0f, -3.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,  0.0f,  1.0f,  0.0f,
     3.0f,  3.0f, -3.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,  0.0f,  1.0f,  0.0f,
     3.0f,  3.0f,  3.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,  0.0f,  1.0f,  0.0f,
    -3.0f,  3.0f,  3.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,  0.0f,  1.0f,  0.0f,
  };

  u32 my_vertices_size = ARRAYSIZE(my_vertices);
  u8 number_of_components = 11;
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

  u32 my_indices[] =
  {
    // Front face
    0, 1, 2,    0, 2, 3,
    // Back face
    4, 5, 6,    4, 6, 7,
    // Left face
    8, 9, 10,   8, 10, 11,
    // Right face
    12, 13, 14, 12, 14, 15,
    // Bottom face
    16, 17, 18, 16, 18, 19,
    // Top face
    20, 21, 22, 20, 22, 23,
  };

  u32 my_indices_size = ARRAYSIZE(my_indices);

  ID3D11Buffer* my_indices_buffer;
  {
    D3D11_BUFFER_DESC buffer_desc = {};
    buffer_desc.ByteWidth = sizeof(u32) * my_indices_size;
    buffer_desc.Usage     = D3D11_USAGE_IMMUTABLE;
    buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    
    D3D11_SUBRESOURCE_DATA subresource = { my_indices };
    
    device->CreateBuffer(&buffer_desc, &subresource, &my_indices_buffer);
  }

  stbi_set_flip_vertically_on_load(true);
  s32 width, height, rgba = 4;
  u8* texture_data = stbi_load("texture_01.png", &width, &height, null, rgba);

  ID3D11ShaderResourceView* texture_srv;
  {
    D3D11_TEXTURE2D_DESC texture_desc = {};
    texture_desc.Width            = width;
    texture_desc.Height           = height;
    texture_desc.MipLevels        = 0;
    texture_desc.ArraySize        = 1;
    texture_desc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    texture_desc.Usage            = D3D11_USAGE_DEFAULT;
    texture_desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    texture_desc.MiscFlags        = D3D11_RESOURCE_MISC_GENERATE_MIPS;
    texture_desc.SampleDesc.Count = 1;

    ID3D11Texture2D* texture_2d;
    device->CreateTexture2D(&texture_desc, null, &texture_2d);

    device->CreateShaderResourceView(texture_2d, null, &texture_srv);

    device_context->UpdateSubresource(texture_2d, 0, null, texture_data, width * sizeof(u32), 0);
    device_context->GenerateMips(texture_srv);
  }

  stbi_image_free(texture_data);

  ID3D11SamplerState* sampler_state;
  {
    D3D11_SAMPLER_DESC sampler_desc = {};
    sampler_desc.Filter         = D3D11_FILTER_ANISOTROPIC;
    sampler_desc.MaxAnisotropy  = 16;
    sampler_desc.MinLOD         = 0;
    sampler_desc.MaxLOD         = D3D11_FLOAT32_MAX;
    sampler_desc.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    
    device->CreateSamplerState(&sampler_desc, &sampler_state);
  }

  ID3D11VertexShader* vertex_shader;
  ID3DBlob* vertex_shader_blob;

  D3DCompileFromFile(L"010_point_light.hlsl", null, null, "vs_main", "vs_5_0", 0, 0, &vertex_shader_blob, null);
  device->CreateVertexShader(vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(), null, &vertex_shader);

  ID3D11PixelShader* pixel_shader;
  ID3DBlob* pixel_shader_blob;

  D3DCompileFromFile(L"010_point_light.hlsl", null, null, "ps_main", "ps_5_0", 0, 0, &pixel_shader_blob, null);
  device->CreatePixelShader(pixel_shader_blob->GetBufferPointer(), pixel_shader_blob->GetBufferSize(), null, &pixel_shader);

  ID3D11InputLayout* input_layout;
  {
    D3D11_INPUT_ELEMENT_DESC input_element_desc[] =
    {
      { "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "COL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "TEX", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "NOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },

    };

    device->CreateInputLayout(input_element_desc, ARRAYSIZE(input_element_desc), vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(), &input_layout);
  }
  
  ID3D11RasterizerState* rasterizer_state;
  {
    D3D11_RASTERIZER_DESC rasterizer_desc = { D3D11_FILL_SOLID, D3D11_CULL_FRONT };
    device->CreateRasterizerState(&rasterizer_desc, &rasterizer_state);
  }

  ID3D11Buffer* constants_buffer;
  {
    D3D11_BUFFER_DESC buffer_desc = {};
    buffer_desc.ByteWidth      = sizeof(Constants) + 0xf & 0xfffffff0;
    buffer_desc.Usage          = D3D11_USAGE_DYNAMIC;
    buffer_desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
    buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        
    device->CreateBuffer(&buffer_desc, null, &constants_buffer);
  }

  f32 aspect = viewport.Width / viewport.Height;
  f32 near = 1.0f;
  f32 far = 9.0f;
  Matrix projection = create_projection_matrix(aspect, near, far);

  V3 my_rotation = {};
  V3 my_translation = {0, 0, 10};
  Matrix my_translation_matrix = create_translation_matrix(my_translation);

  Camera camera = {};
  camera.position = { 0.0f, 0.0f, 0.5f };
  camera.target   = { 0.0f, 0.0f, 0.0f };

  f32 delta_time = 0.016f;

  f32 light_angle = 0.0f; // Tracks rotation angle
  f32 light_radius = 15.0f; // Radius of circular path
  f32 light_speed = 1.0f; // Speed of rotation (radians per second)

  // f32 lx = 0;
  // f32 lz = 0;

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

    // lx += sinf(delta_time) + 10;
    // lz += cosf(delta_time) + 10;

    light_angle += light_speed * delta_time;

    update_camera(&camera, delta_time);

    Matrix view = create_view_matrix(camera.position, camera.target, up_base);

    // my_rotation.x += 0.005f;
    // my_rotation.y += 0.009f;
    // my_rotation.z += 0.001f;

    Matrix rotation_x = create_x_axis_rotation_matrix(my_rotation.x);
    Matrix rotation_y = create_y_axis_rotation_matrix(my_rotation.y);
    Matrix rotation_z = create_z_axis_rotation_matrix(my_rotation.z);
    Matrix model = rotation_x * rotation_y * rotation_z * my_translation_matrix;
    Matrix transform = model * view * projection;

    device_context->ClearRenderTargetView(rtv, background_color);
    device_context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

    device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    device_context->IASetInputLayout(input_layout);
    device_context->IASetVertexBuffers(0, 1, &my_vertices_buffer, &my_vertices_stride, &my_offset);
    device_context->IASetIndexBuffer(my_indices_buffer, DXGI_FORMAT_R32_UINT, 0);

    D3D11_MAPPED_SUBRESOURCE constants_subresource;
    device_context->Map(constants_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constants_subresource);
    {
      Constants* constants = (Constants*)constants_subresource.pData;
      constants->transform = transform;
      constants->model = model;
      // constants->light_position = {5, 5, 0};
      // constants->light_position = { 5.0f, 10.0f, -5.0f };
      constants->light_position = { my_translation.x + light_radius * cosf(light_angle), 10, my_translation.z + light_radius * sinf(light_angle) };
      constants->viewPos = camera.position;
    }
    device_context->Unmap(constants_buffer, 0);

    device_context->VSSetConstantBuffers(0, 1, &constants_buffer);
    
    device_context->VSSetShader(vertex_shader, null, 0);
    
    device_context->RSSetViewports(1, &viewport);
    device_context->RSSetState(rasterizer_state);

    device_context->PSSetConstantBuffers(0, 1, &constants_buffer);
    device_context->PSSetShader(pixel_shader, null, 0);
    device_context->PSSetSamplers(0, 1, &sampler_state);
    device_context->PSSetShaderResources(0, 1, &texture_srv);

    device_context->OMSetRenderTargets(1, &rtv, null);

    device_context->DrawIndexed(my_indices_size, 0, 0);

    swap_chain->Present(1, 0);
  }

  return 0;
}
