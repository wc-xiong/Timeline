// 曲面细分因子
cbuffer ConstantBufferTessellation : register(b7)
{
    float g_TessFactor;
}

// 顶点着色器输入
struct VertexIn
{
    float3 PosL : POSITION;
    float2 Tex : TEXCOORD;
};

// 顶点着色器输出
struct VertexOut
{
    float4 PosL : POSITION;
    float2 Tex : TEXCOORD;
};

// 常量外壳着色器输入的结构体，标识曲面细分因子
struct PatchTess
{
    float EdgeTess[4] : SV_TessFactor;
    float InsideTess[2] : SV_InsideTessFactor;
};

// 控制点外壳着色器输入（与顶点着色器输出一致）
struct HullOut
{
    float4 PosL : POSITION;
    float2 Tex : TEXCOORD;
};

// 域着色器输出（将作为像素着色器的输入）
struct DomainOut
{
    float4 PosH : SV_POSITION;
    float2 Tex : TEXCOORD;
};