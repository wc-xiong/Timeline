#pragma once

#include <QUuid>
#include <cstdint>
#include <memory>
#include <vector>
#include "Common/Common/MediaCommon.h"

struct ID3D11ShaderResourceView;
struct ID3D11Texture2D;
struct SMediaPty;
struct VertexPosTex;

/**
 * @brief 阶段3/4：ClipFx MediaFxRender + TransFx 顶点计算
 * 与 PreviewGpuPipeline 共用 Device_Decode，避免跨设备 SRV。
 */
class CPreviewFxPass
{
public:
	static CPreviewFxPass& Instance();

	bool EnsureInit();
	void Clear();

	/** 从 MediaNode 绑定 SMediaPty（pFxPty / pBasePty / pEffectPty） */
	void BindClip(const QUuid& clipId);
	void Unbind();
	bool HasBoundMedia() const;

	void SetTransInfo(const STransInfo& info);
	const STransInfo& GetTransInfo() const;

	/**
	 * 对输入纹理应用可见 ClipFx；无特效或失败返回 inSrv。
	 * ptsUs：媒体时间（微秒），供 Blink 等时序特效。
	 */
	ID3D11ShaderResourceView* Apply(
		ID3D11ShaderResourceView* inSrv,
		ID3D11Texture2D* inTex,
		int w, int h,
		int64_t ptsUs);

	/** 裁剪 UV（UpdateFxRenderData 后），用于 letterbox 四边形 */
	bool GetCropUv(float& u0, float& v0, float& u1, float& v1);

	/** 预览变换：相对画布中心的 NDC 偏移/缩放/旋转（度） */
	bool GetPreviewTransform(float& offsetXNdc, float& offsetYNdc,
		float& scaleX, float& scaleY, float& rotZDeg);

	/**
	 * 阶段4：顶点类转场。成功时写出 NDC 顶点/索引（可多于四边形）。
	 * Fade 返回 false（由 alpha 处理）。
	 */
	bool BuildTransVertices(std::vector<VertexPosTex>& verts, std::vector<unsigned int>& indices);

private:
	CPreviewFxPass();
	~CPreviewFxPass();

	struct Impl;
	Impl* m_pImpl = nullptr;
};
