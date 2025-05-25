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
  return float4(p.color, 1);
}