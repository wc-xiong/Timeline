#ifndef _CONV_PQTOSDR_
#define _CONV_PQTOSDR_

// Convert HDR to SDR for SMPTE ST 2084

#include "./tonemapping.hlsl"
#include "./conv_colorgamut.hlsl"

static const float ST2084_m1 =  2610.0f / (4096.0f * 4.0f);
static const float ST2084_m2 = (2523.0f / 4096.0f) * 128.0f;
static const float ST2084_c1 =  3424.0f / 4096.0f;
static const float ST2084_c2 = (2413.0f / 4096.0f) * 32.0f;
static const float ST2084_c3 = (2392.0f / 4096.0f) * 32.0f;

#define SRC_LUMINANCE_PEAK     10000.0
#define DISPLAY_HDR_LUMINANCE_PEAK 1000.0
#define DISPLAY_LUMINANCE_PEAK 135.0
#define DISPLAY_SDR_LUMINANCE_STANDER 80.0

//#pragma warning(disable: 3571) // fix warning X3571 in pow()

// Apply the ST.2084 curve to normalized linear values and outputs normalized non-linear values
float3 LinearToST2084(float3 L)
{
    float3 Lp = pow(L, ST2084_m1);
    return pow((ST2084_c1 + ST2084_c2 * Lp) / (1 + ST2084_c3 * Lp), ST2084_m2);
}

float3 ST2084ToLinear(float3 N)
{
    float3 Np = pow(N, 1 / ST2084_m2);
    return pow(max(Np - ST2084_c1, 0) / (ST2084_c2 - ST2084_c3 * Np), 1 / ST2084_m1);
}

float3 LinearToSRGB(float3 color)
{
    // Approximately pow(color, 1.0 / 2.2)
    return color < 0.0031308 ? 12.92 * color : 1.055 * pow(abs(color), 1.0 / 2.4) - 0.055;
}

float3 SRGBToLinear(float3 color)
{
    // Approximately pow(color, 2.2)
    return color < 0.04045 ? color / 12.92 : pow(abs(color + 0.055) / 1.055, 2.4);
}

inline float inverse_HLG_F1(float x)
{
    const float B67_a = 0.17883277;
    const float B67_b = 0.28466892;
    const float B67_c = 0.55991073;
    const float B67_inv_r2 = 4.0;
    x = (x <= 0.5)
        ? x * x * B67_inv_r2
        : exp((x - B67_c) / B67_a) + B67_b;
    return x;
}

inline float3 inverse_HLG_F3(float3 rgb)
{
    rgb.r = inverse_HLG_F1(rgb.r);
    rgb.g = inverse_HLG_F1(rgb.g);
    rgb.b = inverse_HLG_F1(rgb.b);
    return rgb;
}

inline float3 HLGtoLinear(float3 rgb)
{
    rgb = inverse_HLG_F3(rgb);
    float3 ootf_2020 = float3(0.2627, 0.6780, 0.0593);
    float ootf_ys = 2000.0f * dot(ootf_2020, rgb);
    rgb *= pow(ootf_ys, 0.2f);
    return rgb;
}

inline float4 convert_PQ_to_SDR(float4 pixel)
{
    pixel = saturate(pixel); // use saturate(), because pow() can not take negative values

    // ST2084 to Linear
    pixel.rgb = ST2084ToLinear(pixel.rgb);
    
    // Peak luminance
    pixel.rgb = pixel.rgb * (SRC_LUMINANCE_PEAK / DISPLAY_LUMINANCE_PEAK);

    // HDR tone mapping
    pixel.rgb = ToneMappingHable(pixel.rgb);

    // Colorspace Gamut Conversion
    pixel.rgb = Colorspace_Gamut_Conversion_2020_to_709(pixel.rgb);

    // Linear to sRGB
    pixel.rgb = saturate(pixel.rgb);
    pixel.rgb = pow(pixel.rgb, 1.0 / 2.2);

    
    return pixel;
}

inline float4 convert_HLG_to_SDR(float4 pixel)
{
    // HLG to PQ
    pixel = saturate(pixel);
    pixel.rgb = HLGtoLinear(pixel.rgb);
    pixel.rgb /= 1000.0;
    pixel.rgb = LinearToST2084(pixel.rgb);

    pixel = convert_PQ_to_SDR(pixel);
    
    return pixel;
}

inline float4 convert_GAMMA_PQ_to_GAMMA_1_0(float4 pixel)
{ 
    pixel = saturate(pixel); // use saturate(), because pow() can not take negative values
    
    
    // ST2084 to Linear
    pixel.rgb = ST2084ToLinear(pixel.rgb);
    
   
    // Peak luminance
    pixel.rgb = pixel.rgb * (SRC_LUMINANCE_PEAK / DISPLAY_SDR_LUMINANCE_STANDER);

    // Colorspace Gamut Conversion
    pixel.rgb = Colorspace_Gamut_Conversion_2020_to_709(pixel.rgb);
 
    return pixel;
}

inline float4 convert_GAMMA_HLG_to_GAMMA_1_0(float4 pixel)
{
    pixel = saturate(pixel);
    pixel.rgb = HLGtoLinear(pixel.rgb);
    pixel.rgb = Colorspace_Gamut_Conversion_2020_to_709(pixel.rgb);
    return pixel;
}

inline float4 convert_GAMMA_2_2_to_GAMMA_1_0(float4 pixel)
{
    pixel = saturate(pixel); // use saturate(), because pow() can not take negative values

    // 2.2 to Linear
    pixel.rgb = SRGBToLinear(pixel.rgb);
    
    //test
    pixel.rgb *= SRC_LUMINANCE_PEAK / DISPLAY_LUMINANCE_PEAK;
    
    // Linear to sRGB
    pixel.rgb = saturate(pixel.rgb);
    
    return pixel;
}

#endif