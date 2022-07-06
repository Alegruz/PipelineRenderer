struct Camera
{
	matrix View;
	float4 Position;
};

struct Display
{
	matrix Projection;
};

struct Model
{
	matrix World;
};

ConstantBuffer<Camera> cbCamera : register(b0);
ConstantBuffer<Display> cbDisplay : register(b1);
ConstantBuffer<Model> cbModel : register(b2);

struct InputVertex
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

struct OutputVertex
{
    float4 Position : SV_Position;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

OutputVertex main(InputVertex Input)
{
	OutputVertex Output;

    Output.Position = mul(cbModel.World, float4(Input.Position, 1.0f));
    Output.Position = mul(cbCamera.View, Output.Position);
    Output.Position = mul(cbDisplay.Projection, Output.Position);
    
    Output.TexCoord = Input.TexCoord;
    
    Output.Normal = normalize(mul(cbModel.World, float4(Input.Normal, 0.0f)).xyz);
	
    return Output;
}