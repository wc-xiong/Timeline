#include "Basic.hlsli"
#include "Tessellation_Basic.hlsli"

// 顶点着色器
VertexOut VS(VertexIn vin)
{
    VertexOut vout;

    vout.PosL = mul(float4(vin.PosL, 1.0f), g_World);
    vout.PosL = mul(vout.PosL, g_View);
    vout.PosL = mul(vout.PosL, g_Proj);

    vout.Tex = vin.Tex;
    
    return vout;
}