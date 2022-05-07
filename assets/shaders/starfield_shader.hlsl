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

float hash21(float2 p) {
    p = frac(p * float2(123.34f, 456.21f));
    p += dot(p, p + 45.32f);
    return frac(p.x * p.y);
}

float4 pixel(PixelInput input) : SV_TARGET {
	float2 uv = (input.position.xy - 0.5f * float2(1920.0f, 1080.0f)) / 1080.0f;
	uv *= 30.0f;

	float3 color = float3(0.0f, 0.0f, 0.0f);

	float2 gv = frac(uv) - 0.5f;
	float2 id = floor(uv);

	for (int y = -1; y <= 1; y++) {
		for (int x = -1; x <= 1; x++) {
			float2 offset = float2(x, y);
			float n = hash21(id - offset);

			float j = length(gv - offset - float2(n, frac(n * 10.0f)));
			float light = 0.05f / j;
			light *= smoothstep(0.5, 0.2f, j);
			color += float3(light, light, light);
		}
	}

	return float4(color, 1.0f);
}