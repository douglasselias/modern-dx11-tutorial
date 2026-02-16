RWTexture2D<float4> out_texture;

cbuffer PerFrame
{
  float2 TextureSize;
  float BlueColor;
};

[numthreads(32, 32, 1)]
void compute_shader(uint3 threadID : SV_DispatchThreadID)
{
  float4 color = float4((threadID.xy / TextureSize), BlueColor, 1);
  out_texture[threadID.xy] = color;
}