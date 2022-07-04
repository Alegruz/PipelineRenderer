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
	float3 Color : COLOR;
};

struct OutputVertex
{
	float4 Color : COLOR;
	float4 Position : SV_Position;
};

float4 main(InputVertex Input) : SV_POSITION
{
	OutputVertex Output;

    Output.Position = mul(cbModel.World, float4(Input.Position, 1.0f));
    Output.Position = mul(cbCamera.View, Output.Position);
    Output.Position = mul(cbDisplay.Projection, Output.Position);
    Output.Color = Input.Color;
	
    return Output;
}