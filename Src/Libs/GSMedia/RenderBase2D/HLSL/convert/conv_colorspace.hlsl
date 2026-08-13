#ifndef _CONV_COLORSPACE_
#define _CONV_COLORSPACE_

static const float4x4 REC_2020_RGB2YUV =
{
    0.2627, 0.6780, 0.0593, 0.0,
    -0.1396, -0.3604, 0.5000, 0.0,
    0.5000, -0.4598, -0.0402, 0.0,
    0.0, 0.0, 0.0, 1.0
};

static const float4x4 REC_2020_YUV2RGB =
{
    1.0000, -0.0000, 1.4746, 0.0,
    1.0000, -0.1645, -0.5713, 0.0,
    1.0000, 1.8814, -0.0001, 0.0,
    0.0, 0.0, 0.0, 1.0
};

static const float4x4 REC_709_RGB2YUV =
{
    0.2126, 0.7152, 0.0722, 0.0,
    -0.11457, -0.38543, 0.5000, 0.0,
    0.5000, -0.45415, -0.04585, 0.0,
    0.0, 0.0, 0.0, 1.0
};

static const float4x4 REC_709_YUV2RGB =
{
    1.0000, -0.000, 1.5748, 0.0,
    1.0000, -0.187327, -0.468124, 0.0,
    1.0000, 1.85560, 0.0000, 0.0,
    0.0, 0.0, 0.0, 1.0

};

static const float4x4 REC_601_RGB2YUV =
{
    0.299, 0.587, 0.114, 0.0,
    -0.169, -0.331, 0.5000, 0.0,
    0.5000, -0.419, -0.081, 0.0,
    0.0, 0.0, 0.0, 1.0
};

static const float4x4 REC_601_YUV2RGB =
{
    1.0000, -0.0009, 1.4019, 0.0,
    1.0000, -0.3437, -0.7142, 0.0,
    1.0000, 1.7721, 0.0009, 0.0,
    0.0, 0.0, 0.0, 1.0
};

#define CLR_SPACE_601 0
#define CLR_SPACE_709 1
#define CLR_SPACE_2020 2

float4 Colorspace601_Conversion_RGB2YUV(const float4 rgba)
{
    return mul(REC_601_RGB2YUV, rgba);
}

float4 Colorspace601_Conversion_YUV2RGB(const float4 yuva)
{
    return mul(REC_601_YUV2RGB, yuva);
}

float4 Colorspace709_Conversion_RGB2YUV(const float4 rgba)
{
    return mul(REC_709_RGB2YUV, rgba);
}

float4 Colorspace709_Conversion_YUV2RGB(const float4 yuva)
{
    return mul(REC_709_YUV2RGB, yuva);
}

float4 Colorspace2020_Conversion_RGB2YUV(const float4 rgba)
{
    return mul(REC_2020_RGB2YUV, rgba);
}

float4 Colorspace2020_Conversion_YUV2RGB(const float4 yuva)
{
    return mul(REC_2020_YUV2RGB, yuva);
}

#endif