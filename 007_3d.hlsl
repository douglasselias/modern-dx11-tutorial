cbuffer constants : register(b0)
{
  row_major float4x4 transform;
}

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
  output.position = mul(float4(v.position, -45, 1), transform);
  output.color = v.color;
  output.texcoord = v.texcoord;
  return output;
}

float4 ps_main(Pixel p) : SV_TARGET
{
  return mytexture.Sample(mysampler, p.texcoord) * float4(p.color, 1);
}