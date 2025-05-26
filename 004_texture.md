## Part 4 - Texture

To render a texture we need to load the contents of the image. `stb_image.h` is a single header library that is very simple to use for this purpose.

```cpp
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
```

Load the texture.

```cpp
stbi_set_flip_vertically_on_load(true); // Images have the 0 coordinate on Y axis on the top side. But the screen expects the opposite.  
s32 width, height, number_of_color_channels = 4;
u8* texture_data = stbi_load("texture_01.png", &width, &height, null, number_of_color_channels);
```

Create the shader resource view that stores the texture data and description.

```cpp
ID3D11ShaderResourceView* texture_srv;
{
  D3D11_TEXTURE2D_DESC texture_desc = {};
  texture_desc.Width            = width;
  texture_desc.Height           = height;
  texture_desc.MipLevels        = 1;
  texture_desc.ArraySize        = 1;
  texture_desc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // Needs to match the same format of render target view.
  texture_desc.Usage            = D3D11_USAGE_IMMUTABLE;
  texture_desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;
  texture_desc.SampleDesc.Count = 1;

  D3D11_SUBRESOURCE_DATA subresource = {};
  subresource.pSysMem     = texture_data;
  subresource.SysMemPitch = width * sizeof(u32);

  ID3D11Texture2D* texture_2d;
  device->CreateTexture2D(&texture_desc, &subresource, &texture_2d);

  device->CreateShaderResourceView(texture_2d, null, &texture_srv);
}
```

We also need a sampler to be able to sample the texture.

```cpp
ID3D11SamplerState* sampler_state;
{
  D3D11_SAMPLER_DESC sampler_desc = {};
  sampler_desc.Filter         = D3D11_FILTER_MIN_MAG_MIP_POINT;
  sampler_desc.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
  sampler_desc.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
  sampler_desc.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
  sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  
  device->CreateSamplerState(&sampler_desc, &sampler_state);
}
```

Now, add the x and y coordinates for the texture. Also commonly referred to u and v.

```cpp
f32 my_vertices[] =
{
   0.0,  0.75, 1, 1, 1, 0.5, 1.0,
  -0.5, -0.75, 1, 1, 1, 0.0, 0.0,
   0.5, -0.75, 1, 1, 1, 1.0, 0.0,
};

u8 number_of_components = 7;
```

And update the input layout.

```cpp
D3D11_INPUT_ELEMENT_DESC input_element_desc[] =
{
  { "POS", 0, DXGI_FORMAT_R32G32_FLOAT,    0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "COL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "TEX", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};
```

Now on the main loop, set the texture and sampler in the pipeline.

```cpp
device_context->PSSetSamplers(0, 1, &sampler_state);
device_context->PSSetShaderResources(0, 1, &texture_srv);
```

Finally update the shader to receive the texture.

```hlsl
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
  output.color = v.color;
  output.texcoord = v.texcoord;
  return output;
}

float4 ps_main(Pixel p) : SV_TARGET
{
  return mytexture.Sample(mysampler, p.texcoord) * float4(p.color, 1);
}
```