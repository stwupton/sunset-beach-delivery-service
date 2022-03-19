struct VertexInput {
	float4 position : POSITION;
};

struct PixelInput {
	float4 position : SV_POSITION;
};

PixelInput vertex(VertexInput input) {
	PixelInput output;
	output.position = input.position;
	return output;
}

float4 pixel(PixelInput input) : SV_TARGET {
	float2 uv = (input.position.xy - 0.5f * float2(1920.0f, 1080.0f)) /  1080.0f;
	float d = 0.05f / length(uv);
	return float4(d, d, d, 1.0f);
}