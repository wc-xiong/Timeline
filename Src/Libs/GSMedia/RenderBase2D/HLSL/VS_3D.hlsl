#include "Basic.hlsli"

// 顶点着色器
VertexPosHTex VS(VertexPosTex vIn)
{
	VertexPosHTex vOut;
	vOut.PosH = mul(float4(vIn.PosL, 1.0f), g_World);
	vOut.PosH = mul(vOut.PosH, g_View);
	vOut.PosH = mul(vOut.PosH, g_Proj);

    vOut.Tex = vIn.Tex;
    return vOut;
}