cbuffer constants : register(b0)
{
  row_major float4x4 transform;
  row_major float4x4 model;
  float3 light_position;
  float pad0;
  float3 camera_position;
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

// R = 2*(N•L)*N-L
float3 reflection(float3 N, float3 L)
{
  return 2 * dot(N, L) * N - L;
}

float specular_phong(float3 R, float3 V)
{
  return pow(dot(R, V), 2);
}

float3 half_vector(float3 L, float3 V)
{
  float3 sum = L + V;
  return sum / abs(sum);
}

float specular_blinn_phong(float3 N, float3 H)
{
  float specular = 32;
  return pow(dot(N, H), specular);  
}

Pixel vs_main(Vertex v)
{
  Pixel output;
  output.position = mul(float4(v.position, 1), transform);
  output.color    = v.color;
  output.texcoord = v.texcoord;
  output.normal   = normalize(mul(float4(v.normal, 0), model).xyz);
  output.frag_pos = mul(float4(v.position, 1), model).xyz;
  return output;
}

float4 ps_main(Pixel p) : SV_TARGET
{
  float4 tex_sample = mytexture.Sample(mysampler, p.texcoord);
  float3 lightColor = float3(1, 1, 1);
  float3 lightDirection = float3(0, 0, 0);
  
  // Directional light (mesma direção para todos os pixels, como sol):
  lightDirection = normalize(float3(0.1, 0.2, -0.4));
  // Point light (direção depende da posição do fragmento):
  // lightDirection = normalize(light_position - p.frag_pos);
  float ambientStrength = 0.1;
  float3 ambient = ambientStrength * lightColor * tex_sample.rgb;
  float dot_light = dot(p.normal, lightDirection);
  float diffuseStrength = saturate(dot_light);
  float3 diffuse = diffuseStrength * lightColor * tex_sample.rgb;
  // return float4(ambient + diffuse, 1);
  /// Specular (Phong)
  float shininess = 32;
  float3 viewDir = normalize(camera_position - p.frag_pos);
  float3 reflectionVector = normalize(reflection(p.normal, lightDirection));
  float spec = pow(saturate(dot(reflectionVector, viewDir)), shininess);
  float3 specular = spec * lightColor;
  // return float4(p.normal * 0.5 + 0.5, 1);  // visualizar normais
  // return float4(diffuse, 1);                 // só diffuse
  // return float4(specular, 1);                // só specular
  float4 result = float4(ambient + diffuse + specular, 1);
  return result;
}