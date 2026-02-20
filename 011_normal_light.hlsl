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
Texture2D    normaltex : register(t1);
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
  return normalize(sum);
}

float specular_blinn_phong(float3 N, float3 H)
{
  float specular = 32;
  return pow(dot(N, H), specular);  
}

float3x3 cotangent_frame(float3 N, float3 p, float2 uv)
{
  float3 dp1 = ddx(p);
  float3 dp2 = ddy(p);
  float2 duv1 = ddx(uv);
  float2 duv2 = ddy(uv);

  float3 dp2perp = cross(dp2, N);
  float3 dp1perp = cross(N, dp1);
  float3 T = dp2perp * duv1.x + dp1perp * duv2.x;
  float3 B = dp2perp * duv1.y + dp1perp * duv2.y;

  float invmax = rsqrt(max(dot(T, T), dot(B, B)));
  return float3x3(T * invmax, B * invmax, N);
}

float3 perturb_normal(float3 N, float3 V, float2 texcoord)
{
  float3 tangent_normal = normaltex.Sample(mysampler, texcoord).xyz * 2.0f - 1.0f;
  float3x3 tbn = cotangent_frame(N, -V, texcoord);
  return normalize(mul(tangent_normal, tbn));
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
  float3 viewDir = normalize(camera_position - p.frag_pos);
  float3 mappedNormal = perturb_normal(normalize(p.normal), viewDir, p.texcoord);
  
  float3 lightDirection = normalize(float3(0.5, 1.0, -0.5));

  float ambientStrength = 0.1;
  float3 ambient = ambientStrength * lightColor * tex_sample.rgb;
  float dot_light = dot(mappedNormal, lightDirection);
  float diffuseStrength = saturate(dot_light);
  float3 diffuse = diffuseStrength * lightColor * tex_sample.rgb;

  float shininess = 32;
  float3 reflectionVector = reflect(-lightDirection, mappedNormal);
  float spec = pow(saturate(dot(reflectionVector, viewDir)), shininess);
  float specularStrength = 0.8;
  float3 specular = spec * lightColor * specularStrength;
  float4 result = float4(ambient + diffuse + specular, 1);
  return result;
}
