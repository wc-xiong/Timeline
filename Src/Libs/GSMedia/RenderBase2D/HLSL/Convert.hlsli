#ifndef _CONVERT_
#define _CONVERT_

#include "Basic.hlsli"
#include "./convert/conv_pqtosdr.hlsl"
#include "./convert/conv_colorspace.hlsl"
#include "./convert/conv_colorrange.hlsl"
#include "./convert/conv_colorproperty.hlsl"

float3 conv_colorBit(const float3 yuv, const int colorBit)
{
    //目前仅支持8bit, 10bit, 12bit
    return lerp(lerp(yuv, yuv * 64, step(10, colorBit)), yuv * 16, step(12, colorBit));;
}

float3 conv_colorRange(const float3 yuv, const int colorBit, const int colorRange)
{
    return lerp(lerp(limit2full_8bit(yuv), limit2full_10bit(yuv), step(10, colorBit)), yuv, step(CLR_RANGE_FULL, colorRange));
}

float3 conv_colorNormalization(const float3 yuv, const bool lackYUV)
{
    return lerp(yuv - float3(0.0, 0.5, 0.5), yuv, step(1, lackYUV));
}

#define CLR_GAMMA_22 0
#define CLR_GAMMA_HDR10 1
#define CLR_GAMMA_HLG 2
#define CLR_GAMMA_HDR10PLUS 3

#define CLR_HDR_CLOSE_SoftWare 0x11
#define CLR_HDR_CLOSE_HardWare 0x21
#define CLR_HDR_OPEN_SOFTWARE 0x12
#define CLR_HDR_OPEN_HADRWARE 0x22

float4 conv_softwareHDR(const float4 rgba, const int cellGamma)
{
    switch (cellGamma)
    {
        case CLR_GAMMA_HDR10:
            return convert_PQ_to_SDR(rgba);
        case CLR_GAMMA_HLG:
            return convert_HLG_to_SDR(rgba);
        default:
            return rgba;
    }
}

float4 conv_hardwareHDR(const float4 rgba, const int cellGamma)
{
    switch (cellGamma)
    {
        case CLR_GAMMA_HDR10:
            return convert_GAMMA_PQ_to_GAMMA_1_0(rgba);
        case CLR_GAMMA_HLG:
            return convert_GAMMA_HLG_to_GAMMA_1_0(rgba);
        default:
            return convert_GAMMA_2_2_to_GAMMA_1_0(rgba);
    }
}


float4 conv_colorHDR(const float4 rgba, const int hdrPty, const int cellGamma)
{
    switch (hdrPty)
    {
        case CLR_HDR_OPEN_SOFTWARE:
            return conv_softwareHDR(rgba, cellGamma);
        case CLR_HDR_OPEN_HADRWARE:
            return conv_hardwareHDR(rgba, cellGamma);
        default:
            return rgba;
    }
}

float4 ColorConvert(float2 texXY)
{    
    float4x4 matrixRGB2YUV, matrixYUV2RGB;
    switch (g_cellMeta.space)
    {
        case CLR_SPACE_601:
            matrixRGB2YUV = REC_601_RGB2YUV;
            matrixYUV2RGB = REC_601_YUV2RGB;
            break;
        case CLR_SPACE_2020:
            matrixRGB2YUV = REC_2020_RGB2YUV;
            matrixYUV2RGB = REC_2020_YUV2RGB;
            break;
        default:
            matrixRGB2YUV = REC_709_RGB2YUV;
            matrixYUV2RGB = REC_709_YUV2RGB;
            break;
    }
    
    //获取节目单元的颜色
    float4 cellYUVA = { 0, 0, 0, 0 };
    float4 cellRGBA = { 0, 0, 0, 0 };
    
    switch (textType)
    {
        case 0: //yuv
            {
                float uWidth = 0;
                float uHeight = 0;
                g_Tex1.GetDimensions(uWidth, uHeight);
                float lackU = step(uWidth, 1.0);

                float vWidth = 0;
                float vHeight = 0;
                g_Tex2.GetDimensions(vWidth, vHeight);
                float lackV = step(uWidth, 1.0);
            
                cellYUVA.x = g_Tex0.Sample(g_Sampler, texXY).x;
                cellYUVA.y = lerp(g_Tex1.Sample(g_Sampler, texXY).x, 0.5, lackU);
                cellYUVA.z = lerp(g_Tex2.Sample(g_Sampler, texXY).x, 0.5, lackV);
                cellYUVA.w = 1.0f;
            
                //bit
                cellYUVA.xyz = conv_colorBit(cellYUVA.xyz, g_cellMeta.bit);
            
                //limit/full
                cellYUVA.xyz = conv_colorRange(cellYUVA.xyz, g_cellMeta.bit, g_cellMeta.range);
                
                cellYUVA.xyz = cellYUVA.xyz - float3(0.0, 0.5, 0.5);
            
                cellRGBA = mul(matrixYUV2RGB, cellYUVA);
            }
            break;
        case 1: //uyvy
            {
                float4 pix = g_Tex0.Sample(g_Sampler, texXY);
    
                float width = 0.0f;
                float height = 0.0f;
                g_Tex0.GetDimensions(width, height);
    
                float2 offset = { 2.0 / width, 1.0 / width };
            
                cellYUVA = float4((fmod(texXY.x, offset.r) < offset.g) ? pix.g : pix.a, pix.r, pix.b, 1.0f);
            
                //limit/full
                cellYUVA.xyz = conv_colorRange(cellYUVA.xyz, g_cellMeta.bit, g_cellMeta.range);
                
                cellYUVA.xyz = cellYUVA.xyz - float3(0.0, 0.5, 0.5);
            
                cellRGBA = mul(matrixYUV2RGB, cellYUVA);
            }
            break;
        case 2: //nv12,p010
            {
                cellYUVA.x = g_Tex0.Sample(g_Sampler, texXY).x;
                cellYUVA.yz = g_Tex1.Sample(g_Sampler, texXY).xy;
                cellYUVA.w = 1.0f;
            
                 //limit/full
                cellYUVA.xyz = conv_colorRange(cellYUVA.xyz, g_cellMeta.bit, g_cellMeta.range);
                
                cellYUVA.xyz = cellYUVA.xyz - float3(0.0, 0.5, 0.5);
            
                cellRGBA = mul(matrixYUV2RGB, cellYUVA);
            }
            break;
        case 3: //bc3
            {
                float4 offsets = float4(-0.50196078431373, -0.50196078431373, 0.0, 0.0);

	            //获取节目单元的颜色
                float4 CoCgSY = g_Tex0.Sample(g_Sampler, texXY) + offsets;
                float scale = (CoCgSY.z * (255.0 / 8.0)) + 1.0;
                float Co = CoCgSY.x / scale;
                float Cg = CoCgSY.y / scale;
                float Y = CoCgSY.w;

                cellRGBA = float4(Y + Co - Cg, Y + Cg, Y - Co - Cg, 1.0);
            }
            break;
        case 4: //bc3bc4
            {
                float4 offsets = float4(-0.50196078431373, -0.50196078431373, 0.0, 0.0);

	            //获取节目单元的颜色
                float4 CoCgSY = g_Tex0.Sample(g_Sampler, texXY) + offsets;
                float4 alpha = g_Tex1.Sample(g_Sampler, texXY);

                float scale = (CoCgSY.z * (255.0 / 8.0)) + 1.0;
                float Co = CoCgSY.x / scale;
                float Cg = CoCgSY.y / scale;
                float Y = CoCgSY.w;

                cellRGBA = float4(Y + Co - Cg, Y + Cg, Y - Co - Cg, alpha.r);
            }
            break;
        case 5: // YUYV -> GRGB
		    {
                float4 pix = g_Tex0.Sample(g_Sampler, texXY);
                cellYUVA = float4(pix.grb, 1.0f);
            
                //limit/full
                cellYUVA.xyz = conv_colorRange(cellYUVA.xyz, g_cellMeta.bit, g_cellMeta.range);
                
                cellYUVA.xyz = cellYUVA.xyz - float3(0.0, 0.5, 0.5);
            
                cellRGBA = mul(matrixYUV2RGB, cellYUVA);
            }
            break;
        case 6: // rgba
        case 7:
            cellRGBA = g_Tex0.Sample(g_Sampler, texXY).rgba;
            break;
        case 8:
        case 9:
            cellRGBA = float4(g_Tex0.Sample(g_Sampler, texXY).rgb, 1.0);
            break;
        case 10:
            cellRGBA = g_Tex0.Sample(g_Sampler, texXY).gbar;
            break;
        case 11:
            cellRGBA = g_Tex0.Sample(g_Sampler, texXY).abgr;
            break;
        case 12:
            cellRGBA = float4(g_Tex0.Sample(g_Sampler, texXY).gba, 1.0);
            break;
        case 13:
            cellRGBA = float4(g_Tex0.Sample(g_Sampler, texXY).abg, 1.0);
            break;
        default:
            cellRGBA = g_Tex0.Sample(g_Sampler, texXY).rgba;
            break;
    }
    
    return cellRGBA;
}

#endif
