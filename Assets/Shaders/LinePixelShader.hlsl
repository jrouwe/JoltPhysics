struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float4 Color	: COLOR0;
};

struct PS_OUTPUT
{
	float4 RGBColor : SV_TARGET;
};

PS_OUTPUT main(PS_INPUT In)
{
	PS_OUTPUT Output;

	Output.RGBColor = In.Color;

	return Output;
}
