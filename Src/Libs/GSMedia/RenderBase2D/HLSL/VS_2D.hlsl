#include "Basic.hlsli"

// ¶¥µã×ÅÉ«Æ÷
VertexPosHClr VS(VertexPosClr vIn)
{
    VertexPosHClr vOut;
    vOut.PosH = mul(float4(vIn.PosL, 1.0f), g_World);
    vOut.PosH = mul(vOut.PosH, g_View);
    vOut.PosH = mul(vOut.PosH, g_Proj);

    vOut.Clr = vIn.Clr;
    return vOut;
}