Texture2D texture0 : register(t0);

cbuffer VsConstantBuffer : register(b0) {
	float4x4 projection;
}

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
	output.position = mul(input.position, projection);
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