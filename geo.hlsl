static const float2 uv_values[4] =
{
  float2(0.0f, 1.0f), // lower left
  float2(0.0f, 0.0f), // upper left
  float2(1.0f, 0.0f), // upper right
  float2(1.0f, 1.0f), // lower right
};

cbuffer PerFrame
{
  float3 camera_position : CAM_POS;
}

cbuffer PerObject
{
  float4x4 view_projection;
}

Texture2D mytexture;
SamplerState mysampler;

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

  g.Position = v.Position;
  g.Size     = v.Size;

  return g;
}

[maxvertexcount(6)]
void gs_main(point Geometry IN[1], inout TriangleStream<Pixel> triangle_stream)
{
  Pixel p = (Pixel)0;

  float2 halfSize = IN[0].Size / 2.0f;
  float3 direction = camera_position - IN[0].Position.xyz;

  float3 offsetX = halfSize.x * right;
  float3 offsetY = halfSize.y * CameraUp;

  float4 vertices[4];
  vertices[0] = float4(IN[0].Position.xyz + offsetX - offsetY, 1.0f); // lower-left
  vertices[1] = float4(IN[0].Position.xyz + offsetX + offsetY, 1.0f); // upper-left
  vertices[2] = float4(IN[0].Position.xyz - offsetX + offsetY, 1.0f); // upper-right
  vertices[3] = float4(IN[0].Position.xyz - offsetX - offsetY, 1.0f); // lower-right

  // tri: 0, 1, 2
  p.Position = mul(vertices[0], view_projection);
  p.TextureCoordinate = uv_values[0];
  triangle_stream.Append(p);

  p.Position = mul(vertices[1], view_projection);
  p.TextureCoordinate = uv_values[1];
  triangle_stream.Append(p);

  p.Position = mul(vertices[2], view_projection);
  p.TextureCoordinate = uv_values[2];
  triangle_stream.Append(p);
  triangle_stream.RestartStrip();

  // tri: 0, 2, 3
  p.Position = mul(vertices[0], view_projection);
  p.TextureCoordinate = uv_values[0];
  triangle_stream.Append(p);

  p.Position = mul(vertices[2], view_projection);
  p.TextureCoordinate = uv_values[2];
  triangle_stream.Append(p);

  p.Position = mul(vertices[3], view_projection);
  p.TextureCoordinate = uv_values[3];
  triangle_stream.Append(p);
}

float4 pixel_shader(Pixel p) : SV_Target
{
  return mytexture.Sample(mysampler, p.tex_coord);
}