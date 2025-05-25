## Single color triangle

I wanted this tutorial series to be a step by step, with small increments to teach DX 11, however to actually render a triangle it requires several parts to set up.

First you need to know the graphics pipeline stages. You can see the [stages of the pipeline here](https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-graphics-pipeline).

Don't worry if it feels a lot to learn, it will be more clear as we go trough the tutorials.

First we need to have a viewport to specify the display size.

```cpp
D3D11_VIEWPORT viewport = {};

// ....

D3D11CreateDeviceAndSwapChain(null, D3D_DRIVER_TYPE_HARDWARE, null, 0, feature_levels, ARRAYSIZE(feature_levels), D3D11_SDK_VERSION, &swap_chain_desc, &swap_chain, &device, null, &device_context);

swap_chain->GetDesc(&swap_chain_desc); // Update swap_chain_desc with actual window size.

viewport.Width    = (f32)swap_chain_desc.BufferDesc.Width;
viewport.Height   = (f32)swap_chain_desc.BufferDesc.Height;
viewport.MaxDepth = 1;
```

Now we define the vertices of our triangle.

```cpp
// These points are in Normalized Device Coordinates (NDC). (0, 0) is the center. Range between -1 and 1.
f32 my_vertices[] =
{
   0.0,  0.75, // Top
  -0.5, -0.75, // Left
   0.5, -0.75, // Right
};

u32 my_vertices_size = ARRAYSIZE(my_vertices);
u8 number_of_components = 2; // X and Y values for each point.
u32 my_vertices_count = my_vertices_size / number_of_components;
u32 my_vertices_stride = sizeof(f32) * number_of_components;
u32 my_offset = 0;

// Here we need to describe the vertices array.
ID3D11Buffer* my_vertices_buffer;
{
  D3D11_BUFFER_DESC buffer_desc = {};

  buffer_desc.ByteWidth = my_vertices_size * sizeof(f32);
  buffer_desc.Usage     = D3D11_USAGE_IMMUTABLE;
  buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  
  D3D11_SUBRESOURCE_DATA subresource = { my_vertices };
  
  device->CreateBuffer(&buffer_desc, &subresource, &my_vertices_buffer);
}
```

For the next part of the pipeline, we need to create a vertex shader and pixel shader.

```cpp
ID3D11VertexShader* vertex_shader;
ID3DBlob* vertex_shader_blob;

D3DCompileFromFile(L"002_single_color_triangle.hlsl", null, null, "vs_main", "vs_5_0", 0, 0, &vertex_shader_blob, null);
device->CreateVertexShader(vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(), null, &vertex_shader);

ID3D11PixelShader* pixel_shader;
ID3DBlob* pixel_shader_blob;

D3DCompileFromFile(L"002_single_color_triangle.hlsl", null, null, "ps_main", "ps_5_0", 0, 0, &pixel_shader_blob, null);
device->CreatePixelShader(pixel_shader_blob->GetBufferPointer(), pixel_shader_blob->GetBufferSize(), null, &pixel_shader);
```

But for the shader to be able to receive the data, we need to specify the layout.

```cpp
ID3D11InputLayout* input_layout;
{
  D3D11_INPUT_ELEMENT_DESC input_element_desc[] =
  {
    // POS is a semantic name, that is used to match on the shader.
    // "R32G32" specify two 32-bit floats. X and Y in this case.
    {"POS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
  };

  device->CreateInputLayout(input_element_desc, ARRAYSIZE(input_element_desc), vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(), &input_layout);
}
```

Now, the shader.

```hlsl
struct Vertex
{
  float2 position : POS; // POS is the semantic name.
};

struct Pixel
{
  float4 position : SV_POSITION;
};

Pixel vs_main(Vertex v)
{
  Pixel output;
  output.position = float4(v.position, 0, 1);
  return output;
}

float4 ps_main(Pixel p) : SV_TARGET
{
  // Converting from SRGB color to linear before sending to the screen, since the Render Target View was configured to use SRGB, it will apply the gamma correction automatically.
  float3 color = float3(97, 95, 255); // A cool purple color :)
  color /= 255; // Divide every component by 255, to be in the range of [0..1]
  color = pow(color, 2.2f); // Remove the gamma correction.
  return float4(color, 1);
}
```

To learn more about [gamma correction](https://lettier.github.io/3d-game-shaders-for-beginners/gamma-correction.html).

The final piece of the setup is to define a rasterizer state.

```cpp
ID3D11RasterizerState* rasterizer_state;
{
  D3D11_RASTERIZER_DESC rasterizer_desc = { D3D11_FILL_SOLID, D3D11_CULL_NONE };
  device->CreateRasterizerState(&rasterizer_desc, &rasterizer_state);
}
```

Now on the main loop we set the pipeline.

```cpp
// Input Assembler stage
device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
device_context->IASetInputLayout(input_layout);
device_context->IASetVertexBuffers(0, 1, &my_vertices_buffer, &my_vertices_stride, &my_offset);

// Vertex shader stage
device_context->VSSetShader(vertex_shader, null, 0);

// Rasterizer stage
device_context->RSSetViewports(1, &viewport);
device_context->RSSetState(rasterizer_state);

// Pixel shader stage
device_context->PSSetShader(pixel_shader, null, 0);

// Output merger state
device_context->OMSetRenderTargets(1, &rtv, null);

device_context->Draw(my_vertices_count, 0);
```