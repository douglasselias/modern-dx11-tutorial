cbuffer constants : register(b0)
{
  row_major float4x4 transform;
  row_major float4x4 model;
  float3 light_position;
  //////
  float3 viewPos;
}

struct Vertex
{
  float3 position : POS;
  float3 color    : COL;
  float2 texcoord : TEX;
  float3 normal   : NOR;
};

struct Pixel
{
  float4 position : SV_POSITION;
  float3 color    : COL;
  float2 texcoord : TEX;
  float3 normal   : NOR;
  float3 frag_pos : FRAG;
};

Texture2D    mytexture : register(t0);
SamplerState mysampler : register(s0);

Pixel vs_main(Vertex v)
{
  Pixel output;
  output.position = mul(float4(v.position, 1), transform);
  output.color = v.color;
  output.texcoord = v.texcoord;
  // output.normal = v.normal;
  output.normal = normalize(mul(float4(v.normal, 0), model).xyz);
  output.frag_pos = mul(model, float4(v.position, 1)).xyz;
  return output;
}

float4 ps_main(Pixel p) : SV_TARGET
{
  float3 lightColor = float3(1, 1, 1);

  float ambientStrength = 0.1;
  float3 ambient = ambientStrength * lightColor;

  float3 norm = normalize(p.normal);
  float3 lightDir = normalize(light_position - p.frag_pos);

  float diff = max(dot(norm, lightDir), 0.0);
  float3 diffuse = diff * lightColor;

  /// Specular
  float specularStrength = 0.5;
  float3 viewDir = normalize(viewPos - p.frag_pos);
  float3 reflectDir = reflect(-lightDir, norm);

  float spec = pow(max(dot(viewDir, reflectDir), 0), 32);
  float3 specular = specularStrength * spec * lightColor;

// if (diff == 0.0) return float4(1, 0, 0, 1); // Red for unlit areas
  return mytexture.Sample(mysampler, p.texcoord) * float4(p.color * (ambient + diffuse + specular), 1);
}