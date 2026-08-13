#include "Convert.hlsli"
#include "XEffectCommon.hlsli"

static const float PI = 3.1415926535897932385f;
static const float _2PI = 6.2831853071795864769f; // 2 * PI
static const float _1_PI = 0.3183098861837906715f; // 1 / PI

//#define SORT_N(data, N) \
//	for (int i = 1; i < N; i++) \
//	{ \
//		float tmp = data[i]; \
//		int j; \
//		for (j = i - 1; j >= 0; j--) \
//		{ \
//			if (data[j] > tmp) \
//			{ \
//				data[j + 1] = data[j]; \
//			} \
//			else \
//			{ \
//				break; \
//			} \
//		} \
//		data[j + 1] = tmp; \
//	}

float rgba2gray(float4 c)
{
	return (c.r + c.g + c.b) / 3.0f;
}

float rgb2gray(float3 c)
{
	return (c.r + c.g + c.b) / 3.0f;
}

float4 get_texel(float2 uv)
{
    return ColorConvert(uv);
    //return g_Tex0.Sample(g_Sampler, uv);
}

float4 get_texel_mask(float2 uv)
{
    return g_TexMask.Sample(g_Sampler, uv);
}

float4 get_texel_trans(float2 uv)
{
    return g_TexTrans.Sample(g_Sampler, uv);
}

// 闪屏
float4 xe_blink(float4 texel)
{
	texel.rgb = texel.a * texel.rgb; // 先应用原始透明度, 后面需要强制置为不透明
    texel.rgb = lerp(texel.rgb, float3(g_xe.blink.r, g_xe.blink.g, g_xe.blink.b), g_xe.blink.a);
    texel.a = 1.0f;
    return texel;
}

// 分离式线性高斯模糊
float3 xe_blur_linear_1d(float2 uv, float2 texSize, int kernelSizeIndex, int dir)
{
	int gIndex[] =
	{
		0, 1, 3, 6, 10, 15, 21, 28, 36, 45,
	};

	float gWeights[] =
	{
		0.50000,

		0.44908, 0.05092,

		0.33023, 0.15701, 0.01276,

		0.24961, 0.19246, 0.05148, 0.00645,

		0.19955, 0.18945, 0.08376, 0.02321, 0.00403,

		0.16501, 0.17507, 0.10112, 0.04268, 0.01316, 0.00296,

		0.14090, 0.15927, 0.10715, 0.05747, 0.02457, 0.00837, 0.00228,

		0.12249, 0.14427, 0.10711, 0.06708, 0.03544, 0.01579, 0.00594, 0.00188,

		0.10855, 0.13135, 0.10406, 0.07216, 0.04380, 0.02328, 0.01083, 0.00441, 0.00157,

		0.09722, 0.11994, 0.09956, 0.07429, 0.04984, 0.03006, 0.01630, 0.00794, 0.00348, 0.00137,

	};

	float gOffsets[] =
	{
		0.01742,

		0.53805, 2.06278,

		0.62184, 2.27310, 4.14653,

		0.64434, 2.37885, 4.29111, 6.21661,

		0.65319, 2.42547, 4.36803, 6.31412, 8.26479,

		0.65772, 2.45017, 4.41096, 6.37285, 8.33626, 10.30153,

		0.66025, 2.46415, 4.43572, 6.40771, 8.38028, 10.35359, 12.32779,

		0.66187, 2.47315, 4.45178, 6.43058, 8.40963, 10.38901, 12.36877, 14.34899,

		0.66293, 2.47904, 4.46232, 6.44568, 8.42917, 10.41281, 12.39664, 14.38070, 16.36501,

		0.66368, 2.48326, 4.46990, 6.45657, 8.44331, 10.43013, 12.41705, 14.40408, 16.39125, 18.37856

	};

	float3 rgb = float3(0, 0, 0);
	int step = kernelSizeIndex + 1;
	int idxBase = gIndex[kernelSizeIndex];
	float2 pixelOffset;
	if (dir)
		pixelOffset = float2(1.0f, 0.0f);
	else
		pixelOffset = float2(0.0f, 1.0f);
	for (int i = 0; i < step; i++)
	{
		int index = idxBase + i;
		float2 offset = gOffsets[index] * pixelOffset / texSize;
		float3 clr = get_texel(uv + offset).rgb + get_texel(uv - offset).rgb;
		rgb += gWeights[index] * clr;
	}
	return rgb;
}

float3 xe_blur(float2 uv)
{
	float2 dim;
	g_Tex0.GetDimensions(dim.x, dim.y);
	return xe_blur_linear_1d(uv, dim, g_xe.blur.sizeIndex, g_xe.blur.x);
}

// 锐化
float4 xe_sharpen(float2 uv)
{
	float2 iResolution;
    g_Tex0.GetDimensions(iResolution.x, iResolution.y);

	const int size = 2;

	// Pixel colour
	float4 rgba = get_texel(uv);
	// 模糊除去细节
	float3 bluredRGB = (xe_blur_linear_1d(uv, iResolution, size, 0) + xe_blur_linear_1d(uv, iResolution, size, 1)) / 2.0f;
	float3 diff = rgba.rgb - bluredRGB; // 细节
	rgba.rgb = rgba.rgb + diff * g_xe.sharpen.weight; // 加深细节部分
	return rgba;
}

// 浮雕
float3 xe_emboss(float2 uv)
{
	float2 iResolution;
    g_Tex0.GetDimensions(iResolution.x, iResolution.y);
	float2 size = float2(1.0f, 1.0f) / iResolution;

	float dx = 0, dy = 0;
	switch (g_xe.emboss.dir) // 0 ~ 7: 左上开始, 顺时针变化, 8 个方向
	{
	case 0: // TL
		dx = -size.x;
		dy = -size.y;
		break;
	case 1: // TC
		dy = -size.y;
		break;
	case 2: // TR
		dx = size.x;
		dy = -size.y;
		break;
	case 3: // R
		dx = size.x;
		break;
	case 4: // BR
		dx = size.x;
		dy = size.y;
		break;
	case 5: // BC
		dy = size.y;
		break;
	case 6: // BL
		dx = -size.x;
		dy = size.y;
		break;
	case 7: // L
		dx = -size.x;
		break;
	}
	float3 c1 = get_texel(float2(uv.x + dx, uv.y + dy)).rgb;
	float3 c2 = get_texel(float2(uv.x - dx, uv.y - dy)).rgb;
	float dc = rgb2gray(c1) - rgb2gray(c2);
	dc = dc * g_xe.emboss.weight; // 加深区别
	float gray = saturate(dc + 0.5f);
	return float3(gray, gray, gray);
}

// 降噪
float4 xe_denoise(float2 uv)
{
	float2 iResolution;
	g_Tex0.GetDimensions(iResolution.x, iResolution.y);
	float4 rgba = get_texel(uv);
	switch (g_xe.denoise.type) // 0: 椒盐, 1: 高斯
	{
	case 0:
	{
		float2 size = float2(1.0f, 1.0f) / iResolution;

		float4 texels[5] = {
			get_texel(float2(uv.x, uv.y - size.y)),
			get_texel(float2(uv.x - size.x, uv.y)),
			get_texel(uv),
			get_texel(float2(uv.x + size.x, uv.y)),
			get_texel(float2(uv.x, uv.y + size.y))
		};
		float grays[5];
		int index[5];
		int i = 0, mid = 2;
		for (i = 0; i < 5; i++)
		{
			grays[i] = rgba2gray(texels[i]);
			index[i] = i;
		}
#define SWAP(a, b) if (a < b) { int tmp = a; a = b; b = tmp; }
#define G(i) grays[index[i]]
#define I(i) index[i]

		if (G(0) < G(1))
		{
			index[0] = 1;
			index[1] = 0;
		}
		if (G(2) < G(3))
		{
			index[2] = 3;
			index[3] = 2;
		}
		if (G(0) < G(2))
		{
			SWAP(I(0), I(2));
			SWAP(I(1), I(3));
		}
		if (G(1) < G(4))
		{
			SWAP(I(1), I(4));
		}
		if (G(1) < G(2))
		{
			SWAP(I(1), I(2));
			SWAP(I(3), I(4));
		}
		if (G(4) < G(2))
		{
			mid = I(2);
		}
		else
		{
			mid = I(4);
		}
#undef SWAP
#undef G
#undef I
		return texels[mid];
//		int R = g_xe.denoise.weight;
//		float2 size = float2(1.0f, 1.0f) / iResolution;
//		switch (g_xe.denoise.weight)
//		{
//		case 1: // 5
//		{
//#define LOAD_TEXELS(uv, W, data) \
//{ \
//	int j = 0; \
//	float x = uv.x, y = uv.y - W * size.y; \
//	for (int i = 0; i < W; i++) \
//	{ \
//		data[j++] = get_texel(float2(x, y)).rgb; \
//		y = y + size.y; \
//	} \
//	x = uv.x - W * size.x, y = uv.y; \
//	for (int i = 0; i < W; i++) \
//	{ \
//		data[j++] = get_texel(float2(x, y)).rgb; \
//		x = x + size.x; \
//	} \
//	data[j] = rgba.rgb; \
//	x = uv.x + W * size.x, y = uv.y; \
//	for (int i = 0; i < W; i++) \
//	{ \
//		data[j++] = get_texel(float2(x, y)).rgb; \
//		x = x + size.x; \
//	} \
//	x = uv.x, y = uv.y + W * size.y; \
//	for (int i = 0; i < W; i++) \
//	{ \
//		data[j++] = get_texel(float2(x, y)).rgb; \
//		y = y + size.y; \
//	} \
//}
//#define MID_N(N, MID) \
//{ \
//	float3 data[N]; \
//	LOAD_TEXELS(uv, R, data); \
//	float r[N], g[N], b[N]; \
//	for (int i = 0; i < N; i++) \
//	{ \
//		r[i] = data[i].r; \
//		g[i] = data[i].g; \
//		b[i] = data[i].b; \
//	} \
//	SORT_N(r, N); \
//	SORT_N(g, N); \
//	SORT_N(b, N); \
//	rgba.rgb = float3(r[MID], g[MID], b[MID]); \
//}
//			MID_N(5, 2);
//			break;
//		}
//		case 2: // 9
//		{
//			MID_N(9, 4);
//			break;
//		}
//		case 3: // 13
//		{
//			MID_N(13, 6);
//			break;
//		}
//		case 4: // 17
//		{
//			MID_N(17, 8);
//			break;
//		}
//		case 5: // 21
//		{
//			MID_N(21, 10);
//			break;
//		}
//		}
		break;
	}
	case 1:
		rgba.rgb = xe_blur_linear_1d(uv, iResolution, /*g_xe.denoise.weight - */1, g_xe.denoise.x);
		break;
	}
	return rgba;
}

// 马赛克
float4 xe_mosaic(float2 uv)
{
	if (uv.x >= g_xe.mosaic.x1 && uv.x <= g_xe.mosaic.x2 && uv.y >= g_xe.mosaic.y1 && uv.y <= g_xe.mosaic.y2)
	{
		float2 dim;
		g_Tex0.GetDimensions(dim.x, dim.y);
		float base = max(dim.x, dim.y);
		float w = base * g_xe.mosaic.size;
		float h = w;
		w = w / dim.x;
		h = h / dim.y;
		float2 offset = uv - float2(g_xe.mosaic.x1, g_xe.mosaic.y1);
		uv.x = g_xe.mosaic.x1 + (float)((int)(offset.x / w) + 0.5f) * w;
		uv.y = g_xe.mosaic.y1 + (float)((int)(offset.y / h) + 0.5f) * h;
	}
	return get_texel(uv);
}

// 阴影绘制, 不包括图片
float4 xe_shadow(float2 uv)
{
	float blur = g_xe.shadow.blur;
	float4 rgba = float4(g_xe.shadow.r, g_xe.shadow.g, g_xe.shadow.b, g_xe.shadow.a);
	float inner = 0.5f - blur;
	float x = abs(uv.x - 0.5f);
	float y = abs(uv.y - 0.5f);
	if (x > inner && y > inner)
	{
		x = x - inner;
		y = y - inner;
		rgba.a = lerp(rgba.a, 0.0f, saturate(sqrt(x * x + y * y) / blur));
	}
	else
	{
		float offset = max(x, y);
		if (offset > inner)
			rgba.a = lerp(rgba.a, 0.0f, saturate((offset - inner) / blur));
	}
	return rgba;
}

// 球面化, 鱼眼实现
float4 xe_sperize(float2 uv)
{
	float x = uv.x - g_xe.spherize.x;
	float y = uv.y - g_xe.spherize.y;
	if (x == 0.0f && y == 0.0f) // 最凸点
	{
		return get_texel(uv);
	}

	float a = g_xe.spherize.a;
	float b = g_xe.spherize.b;
	float c = g_xe.spherize.c;
	float a2 = a * a, b2 = b * b, c2 = c * c;
	float x2 = x * x, y2 = y * y;
	float z2 = (1.0f - x2 / a2 - y2 / b2) * c2;
	if (z2 <= 0.0f) // 球面外
	{
		float4 texel = get_texel(uv);
		if (g_xe.spherize.clearOutside)
		{
			texel.a = 0.0f;
		}
		return texel;
	}

	float z = sqrt(z2);
	float r = sqrt(x2 + y2);
	float theta = atan2(r, z);
	//float w = theta / (PI / 2.0f) / r;
	float w = theta * 2.0f * _1_PI / r;
	float xratio = w * a;
	float yratio = w * b;
	uv.x = x * xratio + g_xe.spherize.x;
	uv.y = y * yratio + g_xe.spherize.y;
	return get_texel(uv);
}

// 漩涡
float4 xe_twirl(float2 uv)
{
	float x = uv.x - g_xe.twirl.x;
	float y = uv.y - g_xe.twirl.y;
	if (x == 0.0f && y == 0.0f)
	{
		return get_texel(uv);
	}

	float R = g_xe.twirl.r;
	float RADIAN = g_xe.twirl.radian;

	float theta = atan2(y, x);
	float r = y / sin(theta);
	if (r > R) // 漩涡外
	{
		float4 texel = get_texel(uv);
		if (g_xe.twirl.clearOutside)
		{
			texel.a = 0.0f;
		}
		return texel;
	}

	float radian = (1.0f - r / R) * RADIAN;
	theta = theta - radian;
	uv.x = r * cos(theta) + g_xe.twirl.x;
	uv.y = r * sin(theta) + g_xe.twirl.y;
	return get_texel(uv);
}

// 镜像, 万花筒实现
float4 xe_mirror(float2 uv)
{
	float x = uv.x - g_xe.mirror.x;
	float y = uv.y - g_xe.mirror.y;
	if (x == 0.0f && y == 0.0f)
	{
		return get_texel(uv);
	}

	float theta = atan2(y, x);
	float r = y / sin(theta);
	theta = theta - g_xe.mirror.offset;
	theta = fmod(fmod(theta + _2PI, _2PI) + _2PI, _2PI); // 避免负数
	uint n = floor(theta / g_xe.mirror.section);
	theta = fmod(theta, g_xe.mirror.section);
	if (n % 2 != 0)
	{
		theta = g_xe.mirror.section - theta;
	}
	theta = theta + g_xe.mirror.offset;
	uv.x = r * cos(theta) + g_xe.mirror.x;
	uv.y = r * sin(theta) + g_xe.mirror.y;
	return get_texel(uv);
}

// 夜视
float4 xe_nightvision(float2 uv)
{
	const float SCAN = 4.0f; // 扫描线高度

	float4 rgba = get_texel(uv);
	rgba.r = rgba.b = 0.0f;

	float2 iResolution;
    g_Tex0.GetDimensions(iResolution.x, iResolution.y);
	float yratio = SCAN / iResolution.y;
	float t = round(fmod(uv.y, yratio) / yratio);
	float n = g_xe.nightvision.noise ? frac((sin(uv.x) + cos(uv.y)) * 1000000.0f * (float)g_xe.nightvision.noise) : 1; // 噪音
	if (t == 1.0f)
	{
		rgba.g = saturate(rgba.g + n / 5.0f);
	}
	else
	{
		rgba.g = saturate(rgba.g - n / 5.0f);
	}
	// 对比度和色调调整
	rgba.rgb = contrastAdj(rgba.rgb, 0.3f);
	rgba.rgb = hsvAdj(rgba.rgb, -0.20f, 0.0f, 0.0f);
	return rgba;
}

// 计算跑马灯边角区域的长度
float corner_length(float x, float y, float l)
{
	return l * 2.0f * atan2(y, x) * _1_PI;
}

// 跑马灯
float4 xe_marquees(float2 uv)
{
	float2 S;
    g_Tex0.GetDimensions(S.x, S.y);
	float x = S.x * uv.x;
	float y = S.y * uv.y;
	float L = g_xe.marquees.l; // 灯带长度
	bool DIR = !g_xe.marquees.dir; // 是否顺时针
	float D0 = g_xe.marquees.d0; // 起始位置
	// colors at TexU

	// 边角大小
	float2 corner = float2(S.x * g_xe.marquees.x, S.y * g_xe.marquees.y);

	float left = corner.x, top = corner.y, right = S.x - corner.x, bottom = S.y - corner.y;
	float w = right - left, h = bottom - top;
	// 周长
	float p = (w + h) * 2 + corner.x * 4;
	float distance = D0 * p;
	if (x < left)
	{
		if (y < top) // lt
		{
			distance += w * 2 + corner.x * 3 + h * 2;
			if (DIR)
				distance += corner_length(left - x, top - y, corner.x);
			else
				distance += corner_length(top - y, left - x, corner.x);
		}
		else if (y > bottom) // lb
		{
			if (DIR)
				distance += w * 2 + corner.x * 2 + h + corner_length(y - bottom, left - x, corner.x);
			else
				distance += h + corner_length(left - x, y - bottom, corner.x);
		}
		else // l
		{
			if (DIR)
				distance += w * 2 + corner.x * 3 + h + bottom - y;
			else
				distance += y - top;
		}
	}
	else if (x <= right)
	{
		if (y <= top) // t
		{
			if (DIR)
				distance += x - left;
			else
				distance += w + h * 2 + 3 * corner.x + right - x;
		}
		else if (y >= bottom) // b
		{
			if (DIR)
				distance += w + corner.x * 2 + h + right - x;
			else
				distance += h + corner.x + x - left;
		}
		else // m
		{
			return get_texel(uv);
		}
	}
	else
	{
		if (y < top) // rt
		{
			if (DIR)
				distance += w + corner_length(top - y, x - right, corner.x);
			else
				distance += w + h * 2 + corner.x * 2 + corner_length(x - right, top - y, corner.x);
		}
		else if (y > bottom) // rb
		{
			if (DIR)
				distance += w + corner.x + h + corner_length(x - right, y - bottom, corner.x);
			else
				distance += w + corner.x + h + corner_length(y - bottom, x - right, corner.x);
		}
		else // r
		{
			if (DIR)
				distance += w + corner.x + y - top;
			else
				distance += w + corner.x * 2 + h + bottom - y;
		}
	}
	L *= p;
	float d = distance / L;
    return get_texel_mask(float2(d, 0.5f));
}

// 蒙版覆盖
float4 xe_mask(float2 uv)
{
	/*float4 rgba = get_texel(uv);
	float a = get_texel_mask(uv).a;
	if (g_xe.mask.reverse)
	{
		a = 1.0f - a;
	}
	rgba.a = a;
	return rgba;*/


	float4 rgba = get_texel(uv);
	float a = get_texel_mask(uv).a;
	float r = get_texel_mask(uv).r;
	float g = get_texel_mask(uv).g;
	float b = get_texel_mask(uv).b;
	if (g_xe.mask.bPicture)
	{
		r = r * a;
		g = g * a;
		b = b * a;
		if (g_xe.mask.reverse)
		{
			r = 1.0f - r;
			g = 1.0f - g;
			b = 1.0f - b;
		}
		rgba.r = rgba.r*r;
		rgba.g = rgba.g*g;
		rgba.b = rgba.b*b;
	}
	else
	{
		if (g_xe.mask.reverse)
		{
			a = 1.0f - a;
		}
	}
	rgba.a = a;
	return rgba;
}

// 转场特效
float4 xe_transation_one(float2 uv)
{
    float4 rgba = get_texel(uv);
    float4 rgbaMark = get_texel_mask(uv);  
    rgba = lerp(rgba, rgbaMark, rgbaMark.a);
    return rgba;
}

float4 xe_transation_two(float2 uv)
{
    float4 rgba = get_texel(uv);
    float4 rgbaMark = get_texel_mask(uv);
    float4 rgbaTrans = get_texel_trans(uv);
    
    if (g_xe.transation.bKeepWhite)
    {
		return rgba;//绘制顺序为先出场后入场，可以考虑不混合出场纹理，由入场纹理覆盖，若后续有问题再考虑混合出场

        //float3 rgb2 = lerp(0, rgba.rgb, rgbaMark.rgb);
		//float alpha = 1.0f;
		//if (rgb2.r < 0.1f && rgbaTrans.r < 0.8f && rgbaMark.r < 0.1f)
		//	alpha = 0.0f;
	    //rgb2 = lerp(rgb2.rgb, rgbaTrans.rgb, rgbaTrans.rgb);
		//return float4(rgb2.rgb, rgbaMark.r);
    }
    else
    {
        float3 rgb2 = lerp(rgba.rgb, 0, rgbaMark.rgb);
		//rgb2 = lerp(rgbaTrans.rgb, rgb2.rgb, 1.0 - rgbaTrans.rgb);
		float alpha = 1.0f;
		if (rgb2.r < 0.3f && rgbaTrans.r < 0.8f && rgbaMark.r > 0.1f)
			alpha = 0.0f;
		rgb2 = lerp(rgb2.rgb, rgbaTrans.rgb, rgbaTrans.rgb);
    
        return float4(rgb2.rgb, alpha);
    }
}

//float4 xe_test(float2 uv)
//{
//	uv = uv - float2(0.5f, 0.5f);
//	float theta = atan2(uv.y, uv.x);
//	theta = fmod(theta + 2.0f * PI, 2.0f * PI) / PI - 1.0f;
//	float4 rgba = float4(1.0f, 0.0f, 0.0f, 1.0f);
//	rgba.rgb = hsvAdj(rgba.rgb, theta, 0.0f, 0.0f);
//	return rgba;
//}

// 像素着色器
float4 PS(VertexPosHTex pIn) : SV_Target
{
	float2 uv = pIn.Tex;
	switch (g_xeType)
	{
	case 1:
		// FP16格式下，alpha值有可能大于1
		return float4(xe_blur(uv), saturate(get_texel(uv).a));
	case 2:
		return xe_sharpen(uv);
	case 3:
		// FP16格式下，alpha值有可能大于1
		return float4(xe_emboss(uv), saturate(get_texel(uv).a));
	case 4:
		return xe_denoise(uv);
	case 6:
		return xe_feather(get_texel(uv), uv, g_xe.feather.size);
	case 7:
		return xe_keying(get_texel(uv), float3(g_xe.keying.r, g_xe.keying.g, g_xe.keying.b), g_xe.keying.threshold, g_xe.keying.weight);
	case 8:
		return xe_blink(get_texel(uv));
	case 9:
		return rgbaAdj(get_texel(uv), float4(g_xe.rgba.r, g_xe.rgba.g, g_xe.rgba.b, g_xe.rgba.a));
	case 12:
		return xe_filter(get_texel(uv), float3(g_xe.filter.r, g_xe.filter.g, g_xe.filter.b));
	case 13:
		return xe_complemnt(get_texel(uv));
	case 15:
		return xe_mosaic(uv);
	case 16:
		return xe_shadow(uv);
	case 17:
	{
		float4 rgba = get_texel(uv);
		rgba.rgb = hsvAdj(rgba.rgb, g_xe.hsv.h, g_xe.hsv.s, g_xe.hsv.v);
		return rgba;
	}
	case 18:
		return xe_sperize(uv);
	case 19:
		return xe_twirl(uv);
	case 20:
		return xe_mirror(uv);
	case 21:
		return xe_nightvision(uv);
	case 22:
		return xe_marquees(uv);
	case 23:
		return xe_mask(uv);
    case 24:
        return xe_transation_one(uv);
    case 25:
        return xe_transation_two(uv);
        
	// passthrough

	case 0:
	case 5:  // Frame
	case 10: // shake
	case 11: // crop
	case 14: // scroll
	default:
		return get_texel(uv);
	}
}
