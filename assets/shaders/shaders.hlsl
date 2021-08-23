Texture2D texture0 : register(t0);

struct VertexInput {
	float4 position : POSITION;
	float2 textureCoord : TEXCOORD;
};

struct PixelInput {
	float4 position : SV_POSITION;
	float2 textureCoord : TEXCOORD;
};

PixelInput vertex(VertexInput input) {
	PixelInput output;
	output.position = input.position;
	output.textureCoord = input.textureCoord;
	return output;
}

SamplerState textureSampler {
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = MIRROR;
	AddressV = MIRROR;
};

float4 pixel(PixelInput input) : SV_TARGET {
	return texture0.Sample(textureSampler, input.textureCoord);
}