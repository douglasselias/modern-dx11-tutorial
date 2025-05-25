struct Vertex
{
  float2 position : POS;
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
  float3 color = float3(97, 95, 255);
  color /= 255;
  color = pow(color, 2.2f);
  return float4(color, 1);
}