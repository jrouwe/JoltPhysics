Texture2D ShaderTexture : register(t2);
SamplerState SampleType : register(s1);

struct PS_INPUT
{
    float4 Position : SV_POSITION;
	float2 Tex		: TEXCOORD0;
	float4 Color	: COLOR0;
};

struct PS_OUTPUT
{
    float4 RGBColor : SV_TARGET;
};

PS_OUTPUT main(PS_INPUT In)
{
    PS_OUTPUT Output;

	Output.RGBColor = In.Color * ShaderTexture.Sample(SampleType, In.Tex);

    return Output;
}
