//***************************************************************************************
// Effects.h by X_Jun(MKXJun) (C) 2018-2020 All Rights Reserved.
// Licensed under the MIT License.
//
// 简易特效管理框架
// Simple effect management framework.
//***************************************************************************************

#ifndef EFFECTS_H
#define EFFECTS_H

#include <memory>
#include "GSMedia/RenderCommon/LightHelper.h"
#include "GSMedia/RenderCommon/RenderStates.h"
#include "GSMedia/RenderDevice/IDeviceResourceDefine.h"
#include "Common/Common/MediaCommon.h"
#include "Common/Common/XEffectCommon.h"
#include "Common/Common/VideoDataCommon.h"

#pragma warning(disable:4251)

/**
 * @brief 2D/3D 通用基础着色器 Effect 管理类
 *
 * 封装顶点/像素/曲面细分着色器、常量缓冲区与渲染状态，
 * 提供世界/视图/投影矩阵、全局/屏幕/单元色彩、XEffect 特效参数及光照材质的设置接口，
 * 供 CTextureRender、CD3DEffectRender 等渲染模块在 Draw 前统一配置 GPU 参数。
 */
class __declspec(dllexport) BasicEffect
{
public:
	/** @brief 顶点着色器类型枚举，用于 SetRenderState 选择 VS 与输入布局 */
	enum VertexShader {
		VSBasic,      // 基础顶点着色器（默认全屏四边形路径）
		VS2D,         // 2D 顶点色输入布局，用于带 per-vertex 颜色的 2D 绘制
		VS3D,         // 3D 位置+纹理坐标输入布局
		VSTransfer,   // 转场/曲面细分传递用顶点着色器
		VSPNT,        // 位置+法线+纹理坐标，配合 Phong 光照
		VSNoChange    // 保持当前已绑定的顶点着色器不变
	};

	/** @brief 外壳着色器（Hull Shader）类型枚举，用于曲面细分管线 */
	enum EHullShader {
		HSNone,       // 不使用外壳着色器
		HSTransfer,   // 曲面细分传递用外壳着色器
		HSNoChange    // 保持当前已绑定的外壳着色器不变
	};

	/** @brief 域着色器（Domain Shader）类型枚举，用于曲面细分后的顶点生成 */
	enum EDomainShader {
		DSNone,       // 不使用域着色器
		DSLiner,      // 线性插值域着色器（创意显示拉伸）
		DSNoChange    // 保持当前已绑定的域着色器不变
	};

	/** @brief 像素着色器类型枚举，用于 SetRenderState 选择 PS 常量缓冲布局 */
	enum PixelShader {
		PSBasic,      // 基础像素着色器（YUV/RGB 常规采样）
		PS2D,         // 2D 专用像素着色器
		PS3D,         // 3D Phong 光照像素着色器
		PSXEffect,    // XEffect 后处理链像素着色器（模糊/HSV/边框等）
		PSPNT,        // 位置+法线+纹理像素着色器
		PSNoChange    // 保持当前已绑定的像素着色器不变
	};

	/** @brief 混合模式枚举，控制 OM 混合状态 */
	enum BlendMode {
		BMCover,      // 不透明覆盖（默认）
		BMBlend,      // Alpha 混合
		BMScreen,     // 滤色/屏幕混合
		BMNoChange,   // 保持当前混合状态不变
		BMPremulAlpha // 预乘 Alpha（源纹理 RGB 已含透明度，如淡化烘焙后的媒体 RT）
	};

	/** @brief 采样器状态枚举，控制 PS 纹理采样过滤与寻址模式 */
	enum SamplerState {
		SamplerPointClamp,    // 点过滤 + Clamp 寻址
		SamplerPointWrap,     // 点过滤 + Wrap 寻址
		SamplerLineClamp,     // 线性过滤 + Clamp 寻址
		SamplerLineBorder,    // 线性过滤 + Border 寻址
		SamplerLineWrap,      // 线性过滤 + Wrap 寻址
		SamplerAnisotropic,   // 各向异性过滤（默认）
		SampleNoChange        // 保持当前采样器不变
	};

	BasicEffect();
	virtual ~BasicEffect();

	BasicEffect(BasicEffect& moveFrom) = delete;
	BasicEffect& operator=(BasicEffect& moveFrom) = delete;
	BasicEffect(BasicEffect&& moveFrom) noexcept;
	BasicEffect& operator=(BasicEffect&& moveFrom) noexcept;

	/**
	 * @brief 初始化 Effect 所需的全部 GPU 资源
	 *
	 * 编译并创建顶点/像素/曲面细分着色器、输入布局及常量缓冲区；
	 * 若已初始化则直接返回 true。
	 *
	 * @param device D3D11 设备指针，不可为 nullptr
	 * @return 初始化成功返回 true，device 无效或编译失败返回 false
	 */
	bool InitAll(ID3D11Device * device);

	/**
	 * @brief 按枚举类型绑定默认渲染状态并切换着色器管线
	 *
	 * 设置深度模板（无深度测试）、光栅化（无背面剔除）、输入布局、
	 * 顶点/像素着色器、混合模式及采样器。绘制前调用以建立一致的默认管线。
	 *
	 * @param deviceContext D3D11 设备上下文
	 * @param vsType 顶点着色器类型，默认 VSBasic
	 * @param pstype 像素着色器类型，默认 PSBasic
	 * @param bmMode 混合模式，默认 BMCover
	 * @param bmFactor 混合因子 RGBA 数组（4 个 float），BMBlend/BMScreen 时使用；可为 nullptr
	 * @param sample 采样器状态，默认 SamplerAnisotropic
	 */
	void SetRenderState(ID3D11DeviceContext * deviceContext, VertexShader vsType = VSBasic, PixelShader pstype = PSBasic, BlendMode bmMode = BMCover, float* bmFactor = nullptr, SamplerState sample = SamplerAnisotropic);

	/**
	 * @brief 设置世界矩阵并写入 VS 绘制常量缓冲
	 *
	 * 同时更新 worldInvTranspose，供光照或法线变换使用。
	 *
	 * @param W 世界矩阵（行主序传入，内部转置后上传）
	 * @return 设置前的旧世界矩阵（转置形式）
	 */
	DirectX::FXMMATRIX XM_CALLCONV SetWorldMatrix(const DirectX::FXMMATRIX& W);

	/**
	 * @brief 设置视图矩阵并写入 VS 帧常量缓冲
	 *
	 * @param V 视图矩阵（行主序传入，内部转置后上传）
	 * @return 设置前的旧视图矩阵（转置形式）
	 */
	DirectX::FXMMATRIX XM_CALLCONV SetViewMatrix(const DirectX::FXMMATRIX& V);

	/**
	 * @brief 设置投影矩阵并写入 VS 帧常量缓冲
	 *
	 * @param P 投影矩阵（行主序传入，内部转置后上传）
	 * @return 设置前的旧投影矩阵（转置形式）
	 */
	DirectX::FXMMATRIX XM_CALLCONV SetProjMatrix(const DirectX::FXMMATRIX& P);

	/**
	 * @brief 设置曲面细分因子（Tessellation Factor）
	 *
	 * 写入 HS 常量缓冲，控制创意显示拉伸时的细分密度。
	 *
	 * @param tessFactor 细分因子，值越大 patch 细分越细
	 */
	void SetTessFactor(float tessFactor);

	/**
	 * @brief 进入创意显示拉伸（曲面细分）渲染模式
	 *
	 * 切换为 4 控制点 Patch 拓扑，绑定曲面细分专用 VS/HS/DS 及对应输入布局。
	 *
	 * @param deviceContext D3D11 设备上下文
	 */
	void SetStretchingMode(ID3D11DeviceContext * deviceContext);

	/**
	 * @brief 退出创意显示拉伸模式，恢复常规三角形列表绘制
	 *
	 * 解绑曲面细分着色器，恢复默认输入布局、深度模板与光栅化状态。
	 *
	 * @param deviceContext D3D11 设备上下文
	 */
	void ReleaseStretchingMode(ID3D11DeviceContext * deviceContext);

	/**
	 * @brief 设置全局色彩属性（亮度/对比度/色相/饱和度/RGB/透明度）
	 *
	 * 写入 PS 帧常量缓冲 globalClr 字段，作用于整屏或图层级的全局调色，
	 * 通常由 IRenderItem::Render 传入的全局 SColorPty 驱动。
	 *
	 * @param clrPty 全局色彩属性，含 luminance、contrast、hue、saturability、red、green、blue、alpha
	 */
	void SetGlobalClr(const SColorPty& clrPty);

	/**
	 * @brief 设置屏幕级色彩属性及 HDR 显示参数
	 *
	 * 写入 PS 帧常量缓冲 screenClr 与 srnHdrPty，用于屏幕输出端的
	 * 色彩校正与 HDR/SDR 色调映射配置。
	 *
	 * @param clrPty 屏幕色彩调节参数（结构同 SColorPty）
	 * @param hdrPty HDR 属性标志或级别，与着色器中 srnHdrPty 语义一致
	 */
	void SetScreenClr(const SColorPty& clrPty, int hdrPty);

	/**
	 * @brief 设置当前绘制单元（Cell/素材块）的色彩属性
	 *
	 * 写入 PS 绘制常量缓冲 cellClr，在全局调色基础上叠加单素材的
	 * 亮度、对比度、色相、饱和度及 RGBA 调节。
	 *
	 * @param clrPty 单元级色彩属性
	 */
	void SetCellClr(const SColorPty& clrPty);

	/**
	 * @brief 设置当前纹理单元的视频色彩元数据
	 *
	 * 写入 PS 单元属性常量缓冲 cellMeta，供像素着色器执行
	 * YUV→RGB 矩阵变换、色域（709/601）、范围（Full/Limited）及位深处理。
	 *
	 * @param clrMete 视频色彩元数据，含 color_bit、color_space、color_range、color_gamma、color_type
	 */
	void SetCellPty(const Video_ClrMetedata& clrMete);

	/**
	 * @brief 设置 XEffect 后处理特效 GPU 参数块
	 *
	 * 将 CBXEffect 整体拷贝至专用常量缓冲，着色器根据 type 字段
	 * 从匿名联合体中读取 blur、hsv、mosaic、frame、shadow 等对应特效参数。
	 * 由 CD3DEffectRender 在各子特效 Draw 前调用。
	 *
	 * @param pty 统一特效参数容器，含 type 及当前生效特效的参数联合体
	 */
	void SetXEffectPty(const CBXEffect& pty);

	/**
	 * @brief 设置指定槽位的平行光参数
	 *
	 * @param pos 光源数组索引
	 * @param dirLight 平行光结构（方向、颜色、强度等）
	 */
	void SetDirLight(size_t pos, const DirectionalLight& dirLight);

	/**
	 * @brief 设置指定槽位的点光源参数
	 *
	 * @param pos 光源数组索引
	 * @param pointLight 点光源结构（位置、衰减、颜色等）
	 */
	void SetPointLight(size_t pos, const PointLight& pointLight);

	/**
	 * @brief 设置指定槽位的聚光灯参数
	 *
	 * @param pos 光源数组索引
	 * @param spotLight 聚光灯结构（方向、锥角、衰减等）
	 */
	void SetSpotLight(size_t pos, const SpotLight& spotLight);

	/**
	 * @brief 设置 Phong 材质参数（环境/漫反射/镜面反射等）
	 *
	 * 写入光照常量缓冲，配合 PS3D/VSPNT 路径使用。
	 *
	 * @param material Phong 材质结构
	 */
	void SetMaterial(const PhongMaterial& material);

	/**
	 * @brief 设置摄像机/观察者世界空间位置
	 *
	 * 写入 VS 帧常量缓冲 eyePos，供镜面反射与光照距离计算。
	 *
	 * @param eyePos 观察者位置向量（XMVECTOR）
	 */
	void XM_CALLCONV SetEyePos(DirectX::FXMVECTOR eyePos);

	/**
	 * @brief 将已修改的常量缓冲区提交到 GPU 并绑定到渲染管线
	 *
	 * 遍历脏标记的 CB，更新 Map/Unmap 后分别 Bind 到 VS/HS/PS 对应槽位。
	 * 所有 Set* 调用后、Draw 前必须调用一次。
	 *
	 * @param deviceContext D3D11 设备上下文
	 */
	void Apply(ID3D11DeviceContext * deviceContext);

private:
	class Impl;
	std::unique_ptr<Impl> pImpl;
	RenderStates m_RenderStates;
};

#endif
