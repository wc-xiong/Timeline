//***************************************************************************************
// IEffects.h by X_Jun(MKXJun) (C) 2018-2022 All Rights Reserved.
// Licensed under the MIT License.
//
// 特效基类定义
// Effect interface definitions.
//***************************************************************************************

#ifndef IEFFECT_H
#define IEFFECT_H

//#include "WinMin.h"
#include <memory>
#include <vector>
#include <d3d11_1.h>
#include <DirectXMath.h>

class Material;
struct MeshData;
struct DirectionalLight;
struct PointLight;
struct SpotLight;
struct Light;
struct Ray;

// 单个MeshData需要设置到输入装配阶段的内容
// 输入布局、strides、offsets和图元由Effect Pass提供
// 其余数据源自MeshData
struct MeshDataInput
{
    ID3D11InputLayout* pInputLayout = nullptr;
    D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    std::vector<ID3D11Buffer*> pVertexBuffers;
    ID3D11Buffer* pIndexBuffer = nullptr;
    std::vector<uint32_t> strides;
    std::vector<uint32_t> offsets;
    uint32_t indexCount = 0;
};

class IEffect
{
public:
    IEffect() = default;
    virtual ~IEffect() = default;
    // 不允许拷贝，允许移动
    IEffect(const IEffect&) = delete;
    IEffect& operator=(const IEffect&) = delete;
    IEffect(IEffect&&) = default;
    IEffect& operator=(IEffect&&) = default;

	virtual bool InitAll(ID3D11Device* device) = 0;

	// 默认状态来绘制,给已经展开的模型
	virtual void SetRenderDefault() = 0;

	virtual void SetMultiRenderDefault() = 0;

	// 选中准备,给已经展开的模型
	virtual void SetRenderSelectPrepare() = 0;

	virtual void SetMultiRenderSelectPrepare() = 0;

	// 选中状态绘制
	virtual void SetRenderSelect() = 0;

	// 视角立方体
	virtual void SetRenderPick() = 0;

	// 锚点绘制
	virtual void SetRenderAnchorPick() = 0;

	// 绘制线框
	virtual void SetRenderWireframe() = 0;

	// 绘制地面网格
	virtual void SetRenderGridGround() = 0;

	// 使用固定颜色绘制，受自定义光照影响，给未展开的模型使用
	virtual void SetRenderPureColorWithLight(bool bPrepare) = 0;

	virtual void SetMultiRenderPureColorWithLight(bool bPrepare) = 0;

	// 使用固定颜色绘制，受基础光照影响，给相机、光源、视角立方体使用
	virtual void SetRenderPure(bool bPrepare) = 0;

	virtual void SetMultiRenderPure(bool bPrepare) = 0;

	// 具有纹理映射，给沉浸式编辑弹窗的预览模型使用（光照来自 g_Light）
	virtual void SetLightTexture() = 0;

	// AI全景专用：纹理坐标由PS内等距矩形投影计算，采样全景图
	virtual void SetPanoramaLit() = 0;

	// AI全景选中态（MRT + stencil prepare）
	virtual void SetMultiPanoramaLit(bool bPrepare) = 0;

	// 绘制贴图
	virtual void SetRenderTexture() = 0;

	// 绘制vr贴图
	virtual void SetRenderVRTexture() = 0;

	// 使用景物模型专用像素着色器
	virtual void SetSight(bool bPrepare) = 0;

	virtual void SetMultiSight(bool bPrepare) = 0;

	virtual void SetSightDoubleSided(bool bPrepare) = 0;

	virtual void SetMultiSightDoubleSided(bool bPrepare) = 0;

	// 几何选中
	virtual void SetGeometryPick() = 0;

	virtual void SetTextureRect(uint32_t pos, const DirectX::XMFLOAT4& rc) = 0;
	virtual void SetTextureNum(int num) = 0;
	virtual void SetPureColor(const DirectX::XMFLOAT4& color) = 0;

    // 更新并绑定常量缓冲区
    virtual void Apply(ID3D11DeviceContext * deviceContext) = 0;
};

class IEffectTransform
{
public:
    virtual void XM_CALLCONV SetWorldMatrix(const DirectX::FXMMATRIX& W) = 0;
    virtual void XM_CALLCONV SetViewMatrix(const DirectX::FXMMATRIX& V) = 0;
    virtual void XM_CALLCONV SetProjMatrix(const DirectX::FXMMATRIX& P) = 0;
};

class IEffectMaterial
{
public:
    virtual void SetMaterial(const Material& material) = 0;
};

class IEffectMeshData
{
public:
    enum EInputDataType
    {
        InputData_None = 0,
        InputData_Vertice = 1,
        InputData_Normal = 2,
        InputData_Texture = 4,
        InputData_Color = 8,
        InputData_PickColor = 16,
    };

    virtual MeshDataInput GetInputData(const MeshData& meshData, int type) = 0;
};

class IEffectLight
{
public:
	// 光源允许的最大数目
	static const int maxLightNum = 48;
	// 自定义光源
	virtual void SetLight(uint32_t pos, const Light& light) = 0;
	// 设置光源个数
	virtual void SetNum(int num) = 0;
	// 设置观察位置
	virtual void SetEyePos(const DirectX::XMFLOAT3& eyePos) = 0;
	// 设置环境光强度
	virtual void SetAmbientIntensity(float value) = 0;
	// 设置环境光颜色
	virtual void SetAmbientColor(const DirectX::XMFLOAT3& color) = 0;
};

class IEffectGeometryPick
{
public:
	virtual void SetGeometryRay(const Ray& ray) = 0;
	virtual void SetGeometryBoxRay(const Ray* boxRay) = 0;
	virtual void SetGeometryResult(const DirectX::XMFLOAT3& result) = 0;
	virtual void SetCameraType(const bool& isPerspect) = 0;
};

#endif
