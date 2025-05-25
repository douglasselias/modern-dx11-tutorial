## Multi-colored triangle

To add color is very easy. First we add the color to our vertices.

```cpp
f32 my_vertices[] =
{
   0.0,  0.75, 0, 1, 0, // Top   (Green)
  -0.5, -0.75, 1, 0, 0, // Left  (Red)
   0.5, -0.75, 0, 0, 1, // Right (Blue)
};

u8 number_of_components = 5;
```

Also, we need to update the vertex shader layout.

```cpp
D3D11_INPUT_ELEMENT_DESC input_element_desc[] =
{
  { "POS", 0, DXGI_FORMAT_R32G32_FLOAT,    0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "COL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // D3D11_APPEND_ALIGNED_ELEMENT is to automatically add the correct offset for the COL element. Otherwise we would need to pass sizeof(f32) * 2.
};
```

Finally, we just need to update our shader.

```hlsl
struct Vertex
{
  float2 position : POS;
  float3 color    : COL;
};

struct Pixel
{
  float4 position : SV_POSITION;
  float3 color    : COL;
};

Pixel vs_main(Vertex v)
{
  Pixel output;
  output.position = float4(v.position, 0, 1);
  output.color = v.color;
  return output;
}

float4 ps_main(Pixel p) : SV_TARGET
{
  // The pixel shader automatically interpolates the color between each vertex.
  return float4(p.color, 1);
}
```