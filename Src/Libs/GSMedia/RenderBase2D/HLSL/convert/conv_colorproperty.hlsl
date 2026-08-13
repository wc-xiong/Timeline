#ifndef _CONV_COLORPROPERTY_
#define _CONV_COLORPROPERTY_

// HSV
// H: 0 - 360
// S: 0 - 1.0
// L: 0 - 1.0

float3 rgb2hsl(const float3 rgb)
{
// Convert rgb floats ([0-1],[0-1],[0-1]) to hsv floats ([0-1],[0-1],[0-1]), from Foley & van Dam p592
// Optimized http://lolengine.net/blog/2013/01/13/fast-rgb-to-hsv

	float r = rgb.r, g = rgb.g, b = rgb.b;
	float K = 0.f;
	if (g < b)
	{
		float tmp = g;
		g = b;
		b = tmp;
		K = -1.f;
	}
	if (r < g)
	{
		float tmp = r;
		r = g;
		g = tmp;
		K = -2.f / 6.f - K;
	}

	const float chroma = r - (g < b ? g : b);
	return float3(
		360.0f * abs(K + (g - b) / (6.f * chroma + 1e-20f)),
		chroma / (r + 1e-20f),
		r
	);
}

float3 hsl2rgb(const float3 hsl)
{
// Convert hsv floats ([0-1],[0-1],[0-1]) to rgb floats ([0-1],[0-1],[0-1]), from Foley & van Dam p593
// also http://en.wikipedia.org/wiki/HSL_and_HSV

	float h = hsl.x / 360.0f, s = hsl.y, v = hsl.z;
	float out_r = 0.0f, out_g = 0.0f, out_b = 0.0f;
	if (s == 0.0f)
	{
		// gray
		return float3(v, v, v);
	}

	h = fmod(h, 1.0f) / (60.0f / 360.0f);
	int i = (int)h;
	float f = h - (float)i;
	float p = v * (1.0f - s);
	float q = v * (1.0f - s * f);
	float t = v * (1.0f - s * (1.0f - f));

	switch (i)
	{
	case 0: out_r = v; out_g = t; out_b = p; break;
	case 1: out_r = q; out_g = v; out_b = p; break;
	case 2: out_r = p; out_g = v; out_b = t; break;
	case 3: out_r = p; out_g = q; out_b = v; break;
	case 4: out_r = t; out_g = p; out_b = v; break;
	case 5: default: out_r = v; out_g = p; out_b = q; break;
	}
	return float3(out_r, out_g, out_b);
}

//RGB调整
float4 rgbaAdj(float4 colorRGBA,  float4 rgba)
{
    float4x4 adj = float4x4(rgba.r, 0, 0, 0,
    0, rgba.g, 0, 0,
    0, 0, rgba.b, 0,
    0, 0, 0, rgba.a);
    
    return mul(colorRGBA, adj);
}

//对比度调整
static const float3 add = float3(0.5f, 0.5f, 0.5f); //防止指数之前小于等于0， pow不支持x小于0；
float3 contrastAdj(float3 colorRGB, float contrast)
{
    return pow(abs(float3(colorRGB.r, colorRGB.g, colorRGB.b) + add), (contrast + 1)) - add;
}

// HSV 调整
float3 hsvAdj(float3 rgb, float hue, float saturability, float lumin)
{
    float maxRGB = max(max(rgb.r, max(rgb.g, rgb.b)), 1.0f);    //开启硬件HDR情况下rgb大于1，需要在hsl2rgb后重新乘以系数。

    float3 hsl = rgb2hsl(rgb);
	float H = hsl.x, S = hsl.y, L = hsl.z;

	H = fmod(H + 180.0 * hue + 360.0, 360.0);

	//if (saturability > 0.0)
	//	S += (1.0 - S) * saturability;
	//else
	//	S += S * saturability;

	//if (lumin > 0.0)
	//	L += (1.0 - L) * lumin;
	//else
	//	L += L * lumin;

	// 叠加自身系数
	S = saturate(S + S * saturability);
	L = saturate(L + L * lumin);

	return hsl2rgb(float3(H, S, L)) * maxRGB;
}

//颜色调整
float4 colorPtyAdjUnit(const float4 cellRGBA, float lumin, float hue, float saturability, float4 rgba, float contrast)
{
	float4 ret = cellRGBA;

	// RGBA 调整
	//if (rgba.r <= 0.999f || rgba.g <= 0.999f || rgba.b <= 0.999f || rgba.a <= 0.999f)
		ret = rgbaAdj(ret, rgba);

	// 对比度
	//if (abs(contrast) >= 0.001f)
		ret.rgb = contrastAdj(ret.rgb, contrast);

	// HSV 调整
	//if (abs(lumin) >= 0.001f || abs(hue) >= 0.001f || abs(saturability) >= 0.001f)
		ret.rgb = hsvAdj(ret.rgb, hue, saturability, lumin);

	return ret;
}

#endif
