VS_OUTPUT vertex_shader(VS_INPUT IN)
{
    VS_OUTPUT OUT = (VS_OUTPUT)0;

    OUT.Position = mul(IN.ObjectPosition, WorldViewProjection);
    OUT.Normal = normalize(mul(float4(IN.Normal, 0), World).xyz);
    OUT.Tangent = normalize(mul(float4(IN.Tangent, 0), World).xyz);
    OUT.Binormal = cross(OUT.Normal, OUT.Tangent);
    OUT.TextureCoordinate = get_corrected_texture_coordinate(IN.
TextureCoordinate);
    OUT.LightDirection = normalize(-LightDirection);

    float3 worldPosition = mul(IN.ObjectPosition, World).xyz;
    float3 viewDirection = CameraPosition - worldPosition;
    OUT.ViewDirection = normalize(viewDirection);

    return OUT;
}

/************* Pixel Shader *************/

float4 pixel_shader(VS_OUTPUT IN) : SV_Target
{
    float4 OUT = (float4)0;

    float3 sampledNormal = (2 * NormalMap.Sample(TrilinearSampler,
IN.TextureCoordinate).xyz) - 1.0; // Map normal from [0..1] to [-1..1]
    float3x3 tbn = float3x3(IN.Tangent, IN.Binormal, IN.Normal);

    sampledNormal = mul(sampledNormal, tbn); // Transform normal to
world space

    float3 viewDirection = normalize(IN.ViewDirection);
    float4 color = ColorTexture.Sample(TrilinearSampler,
IN.TextureCoordinate);
    float3 ambient = get_vector_color_contribution(AmbientColor, color.
rgb);

    LIGHT_CONTRIBUTION_DATA lightContributionData;
    lightContributionData.Color = color;
    lightContributionData.Normal = sampledNormal;
    lightContributionData.ViewDirection = viewDirection;
    lightContributionData.LightDirection = float4(IN.LightDirection, 1);
    lightContributionData.SpecularColor = SpecularColor;
    lightContributionData.SpecularPower = SpecularPower;
    lightContributionData.LightColor = LightColor;
    float3 light_contribution = get_light_contribution
(lightContributionData);

    OUT.rgb = ambient + light_contribution;
    OUT.a = 1.0f;

    return OUT;
}