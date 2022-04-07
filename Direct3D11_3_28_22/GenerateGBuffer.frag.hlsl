struct FullScreenTriangleVSOut {
	float2 outUV : TEXCOORD0;
	float4 outPOS : SV_Position;
};

float4 main(FullScreenTriangleVSOut input) : SV_Target0
{
	return float4(0.25f, 1.0f, 0.9f, 1.0f);
}