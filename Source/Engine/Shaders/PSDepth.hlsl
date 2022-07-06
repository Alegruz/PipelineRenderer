struct InputVertex
{
    float4 Position : SV_Position;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

float4 main(InputVertex Input) : SV_TARGET
{
    float depth = sqrt(sqrt(Input.Position.z / Input.Position.w));
    
    return float4(float3(depth, depth, depth), 1.0f);
}