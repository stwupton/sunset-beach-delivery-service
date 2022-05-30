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

float hash21(int2 input) {
    float2 result = frac(input * float2(123.34f, 456.21f));
    result += dot(result, result + 45.32f);
    return frac(result.x * result.y);
}

float star(float2 uv) {
	float distance = length(uv);
	float result = 0.05f / distance;
	result *= smoothstep(0.5f, 0.2f, distance);

	return result;
}

float4 pixel(PixelInput input) : SV_TARGET {
	float2 uv = (input.position.xy - float2(1920.0f, 1080.0f) * 0.5f) / 1080.0f;
	uv *= 30.0f;

	float3 color = float3(0.0f, 0.0f, 0.0f);

	float2 gv = frac(uv) - 0.5f;
	int2 id = floor(uv);

	for (int y = -1; y <= 1; y++) {
		for (int x = -1; x <= 1; x++) {
			int2 offset = float2(x, y);
			float n = hash21(id + offset);
			color += star(gv - offset - float2(n, frac(n * 10.0f)));
		}
	}

	return float4(color, 1.0f);
}