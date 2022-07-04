struct InputVertex
{
    float4 Color : COLOR;
    float4 Position : SV_Position;
};

float4 main(InputVertex Input) : SV_TARGET
{
    return Input.Color;
}