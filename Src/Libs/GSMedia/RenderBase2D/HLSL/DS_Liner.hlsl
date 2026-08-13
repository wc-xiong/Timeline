#include "Tessellation_Basic.hlsli"

[domain("quad")]
// 域着色器，对细分后的顶点处理
DomainOut DS(PatchTess patchTess,
	float2 uv : SV_DomainLocation,
	const OutputPatch<HullOut, 4> quad)
{
    DomainOut dout;

	// 计算获得当前顶点实际坐标
    float3 v1 = lerp(quad[0].PosL.rgb, quad[1].PosL.rgb, uv.x);
    float3 v2 = lerp(quad[2].PosL.rgb, quad[3].PosL.rgb, uv.x);
    float3 p = lerp(v1, v2, uv.y);
    dout.PosH = float4(p, 1.0);

	// 计算当前顶点对应的纹理坐标
    float2 vTex1 = lerp(quad[0].Tex, quad[1].Tex, uv.x);
    float2 vTex2 = lerp(quad[2].Tex, quad[3].Tex, uv.x);
    float2 pTex = lerp(vTex1, vTex2, uv.y);
    dout.Tex = pTex;

    return dout;
}