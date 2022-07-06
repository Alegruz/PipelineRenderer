struct InputVertex
{
    float4 Position : SV_Position;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

float4 main(InputVertex Input) : SV_TARGET
{
    //return (normalize(Input.Position) + 1.0f) / 2.0f;
    return float4((normalize(Input.Normal) + 1.0f) / 2.0f, 1.0f);
    //return float4(float3(Input.TexCoord, 0.0f), 1.0f);
}