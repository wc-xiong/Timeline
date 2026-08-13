#include "Convert.hlsli"

// 像素着色器
float4 PS(VertexPosHTex pIn) : SV_Target
{     
    //获取节目单元的颜色
    float4 cellRGBA = { 0, 0, 0, 0 };
    
    cellRGBA = ColorConvert(pIn.Tex);

	if (textType == 14)
		return cellRGBA;
    
    cellRGBA.a = saturate(cellRGBA.a); // FP16格式下，alpha值有可能大于1
    
    cellRGBA = conv_colorHDR(cellRGBA, g_HDRPty, g_cellMeta.gamma);

    cellRGBA = colorPtyAdjUnit(cellRGBA, g_cellClr.lumin, g_cellClr.hue, g_cellClr.saturability, g_cellClr.rgba, g_cellClr.contrast);

	//屏幕颜色调整
    float4 screenRGBA = colorPtyAdjUnit(cellRGBA, g_screenClr.lumin, g_screenClr.hue, g_screenClr.saturability, g_screenClr.rgba, g_screenClr.contrast);

	//全局颜色调整
    float4 globalRGBA = colorPtyAdjUnit(screenRGBA, g_globalClr.lumin, g_globalClr.hue, g_globalClr.saturability, g_globalClr.rgba, g_globalClr.contrast);

    return globalRGBA;
}