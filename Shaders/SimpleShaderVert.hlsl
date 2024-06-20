struct VSInput
{
    float2 Position : POSITION; // Use POSITION semantic for vertex position
    float3 Color : COLOR; // Use COLOR semantic for vertex color
};

// Define the output structure
struct VSOutput
{
    float4 Position : SV_POSITION;
    float3 Color : COLOR;
};

VSOutput main(VSInput Input) 
{
    VSOutput Output;

    Output.Position = float4(Input.Position, 0.0, 1.0);
    Output.Color = Input.Color;

    return Output;
}
