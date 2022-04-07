
struct FullScreenTriangleVSOut {
	float2 outUV : TEXCOORD0;
	float4 outPOS : SV_Position;
};

FullScreenTriangleVSOut main(uint VertexID : SV_VertexID) {
	FullScreenTriangleVSOut output;

	output.outUV = float2((VertexID << 1) & 2, VertexID & 2);
	output.outPOS = float4(output.outUV * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);                                  

	return output;
}