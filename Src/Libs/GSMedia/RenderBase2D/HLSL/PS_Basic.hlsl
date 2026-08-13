#include "convert.hlsli"

// 像素着色器
float4 PS(VertexPosHTex pIn) : SV_Target
{
	float4 rgba = g_Tex0.Sample(g_Sampler, pIn.Tex);
	if (textType == 14) // HDR 测试图
	{
		if (g_HDRPty == CLR_HDR_OPEN_HADRWARE)
		{
			// 线性映射 SDR 1.0 -> HDR max
			float ratio = SRC_LUMINANCE_PEAK / DISPLAY_SDR_LUMINANCE_STANDER;
			// 非线性渐变, HDR 亮度区域分布要少很多
			rgba.rgb = pow(abs(rgba.rgb), 2.2f) * ratio;
		}
	}
	return rgba;
}

