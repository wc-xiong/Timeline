#include "Convert.hlsli"
// 不依赖外部环境的特效函数

// 羽化
float4 xe_feather(float4 texel, float2 uv, float feather)
{

	if (feather <= 0.0f)
		return texel;
	float offset = 0.5 - feather;
	uv = uv.xy - float2(0.5, 0.5);
    
   //float2 mxy = step(offset, abs(uv));        
    
    float2 mxy = float2(0.0f, 0.0f);
    if (abs(uv.x) > offset)
        mxy.x = 1.0f;
    if (abs(uv.y) > offset)
        mxy.y = 1.0f;
    
	//移动UV坐标中心
    float2 rxy = abs(uv);
    
    if (feather < 0.25)
        rxy = fmod(rxy, offset);
    else
        rxy = rxy - float2(offset, offset);
    
	float dstSq = rxy.x * rxy.x * mxy.x + rxy.y * rxy.y * mxy.y;  
    dstSq = saturate(dstSq / (feather * feather));
    
	texel.a *= 1.0 - dstSq; // 保留原始透明度
	texel.a = pow(texel.a, 2);

	return texel;
}

// 抠像
float4 xe_keying(float4 texel, float3 rgbKey, float threshold, float weight)
{
	float3 diff = Colorspace709_Conversion_RGB2YUV(float4(texel.rgb, 1.0f)).rgb - Colorspace709_Conversion_RGB2YUV(float4(rgbKey, 1.0f)).rgb;
	float distance = sqrt(diff.g * diff.g + diff.b * diff.b) * (1.0f - weight) + diff.r * weight;
	if (distance < threshold)
	{
		texel.a = saturate(distance / threshold);
	}
	return texel;
}

float4 xe_filter(float4 texel, float3 rgb)
{
	texel.rgb = 1 - (1 - texel.rgb) * (1 - rgb);
	return texel;
}

float4 xe_complemnt(float4 texel)
{
	texel.rgb = float3(1.0f, 1.0f, 1.0f) - texel.rgb;
	return texel;
}

