#ifndef _BASIC_
#define _BASIC_

#include "LightHelper.hlsli"

Texture2D g_Tex0 : register(t0);
Texture2D g_Tex1 : register(t1);
Texture2D g_Tex2 : register(t2);

Texture2D g_TexMask : register(t3);
Texture2D g_TexTrans : register(t4);

SamplerState g_Sampler : register(s0);    // 采样器

cbuffer CBVSChangesEveryDrawing : register(b0)
{
    matrix g_World;
	matrix g_WorldInvTranspose;
}

cbuffer CBVSChangesEveryFrame : register(b1)
{
    matrix g_View;
	float3 g_EyePosW;
}

cbuffer CBVSChangesOnResize : register(b2)
{
    matrix g_Proj;
}

cbuffer CBPSChangesEveryDrawing : register(b3)
{
    struct
    {
        float lumin;
        float contrast;
        float hue;
        float saturability;
        float4 rgba;
    } g_cellClr;
    
}

cbuffer CBPSChangesCellPty : register(b4)
{
    struct
    {
        int bit;
        int space;
        int range;
        int gamma;

    } g_cellMeta;

    int textType;
}

cbuffer CBPSChangesEveryFrame : register(b5)
{
    struct
    {
        float lumin;
        float contrast;
        float hue;
        float saturability;
        float4 rgba;
    } g_globalClr;
    
    struct
    {
        float lumin;
        float contrast;
        float hue;
        float saturability;
        float4 rgba;
    } g_screenClr;
    
    int g_HDRPty;
}

cbuffer XEffectPty : register(b6)
{
	int g_xeType;

	struct
	{
		struct
		{
			// 模糊参数
			int sizeIndex;
			int x;
		} blur; // 模糊特效

		struct
		{
			int dir; // 0 ~ 7: 左上开始, 顺时针变化, 8 个方向
			float weight; // 加深程度
		} emboss; // 浮雕

		struct
		{
			float weight; // 加深程度, 0 ~ 5
		} sharpen; // 锐化

		struct
		{
			int type; // 0: 椒盐, 1: 高斯
			int x;
			int weight; // 加深程度, 0 ~ 4
		} denoise; // 降噪

		struct
		{
			float r, g, b;
			float size;
		} frame; // 边框特效

		struct
		{
			float size;
		} feather;

		struct
		{
			float r, g, b;
			float threshold;
			float weight; // Y 分量权重
		} keying;

		struct
		{
			float r, g, b, a;
		} blink;

		struct
		{
			float r, g, b, a;
		} rgba;

		struct
		{
			float r, g, b;
		} filter;

		struct
		{
			float x1, y1, x2, y2; // 左上角, 右下角
			float size; // 块大小
		} mosaic;

		struct
		{
			float r, g, b, a;
			float blur;
		} shadow;

		struct
		{
			float h, s, v;
		} hsv;

		struct
		{
			float x, y; // 中心点, 0 ~ 1.0
			float a, b, c; // 长短高半径, 0 ~ 1.0
			int clearOutside;
		} spherize;

		struct
		{
			float x, y; // 中心点, 0 ~ 1.0
			float r; // 半径, 0 ~ 1.0
			float radian; // 弧度
			int clearOutside;
		} twirl;

		struct
		{
			float x, y; // 中心点, 0 ~ 1.0
			float section; // 分区大小, 弧度
			float offset; // 起始位置, 弧度
		} mirror;

		struct
		{
			int noise;
		} nightvision;

		struct
		{
			float l; // 灯带长度
			float x, y; // 灯带宽度
			float d0; // 起始距离
			int dir; // 是否顺时针
		} marquees; // 跑马灯

		struct
		{
			int reverse; // 是否反转
			int bPicture; // 是否图片
        } mask; // 蒙版
        
        struct
        {
           int bKeepWhite;
        }transation;
        
    } g_xe;
}

cbuffer CBLightHelperBuffer : register(b8)
{
	DirectionalLight g_DirLight[8];
	PointLight g_PointLight[8];
	SpotLight g_SpotLight[8];
	Material g_Material;
}

struct VertexPosTex
{
    float3 PosL : POSITION;
    float2 Tex : TEXCOORD;
};

struct VertexPosHTex
{
    float4 PosH : SV_POSITION;
    float2 Tex : TEXCOORD;
};

struct VertexPosClr
{
    float3 PosL : POSITION;
    float4 Clr : COLOR;
};

struct VertexPosHClr
{
    float4 PosH : SV_POSITION;
    float4 Clr : COLOR;
};

struct VertexPosNormalTex
{
	float3 PosL : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex : TEXCOORD;
};

struct VertexPosHWNormalTex
{
	float4 PosH : SV_POSITION;
	float3 PosW : POSITION; // 在世界中的位置
	float3 NormalW : NORMAL; // 法向量在世界中的方向
	float2 Tex : TEXCOORD;
};

#endif