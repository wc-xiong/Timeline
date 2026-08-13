#pragma once
#include "Common/Common/Maths/Matrix3x2.h"
#include <QLineF>
#include <QUuid>
#include <chrono>
#include <QMatrix4x4>
#include <d3d11.h>
#include <memory>

#include "VideoDataCommon.h"

struct SMediaPty;
struct SColorPty;
class CTextureRender;
class BasicEffect;
struct STexDatas;
struct SDrawTextureData;

class IBaseRenderItem
{
public:
	virtual ~IBaseRenderItem() {};

	virtual int GetType() = 0;

	virtual QUuid GetId() const = 0;

	virtual void SetBasicEffect(BasicEffect* pEffect) = 0;

	// 媒体渲染，传递全局参数
	virtual	void Render(const SDrawTextureData& texNode, const SColorPty& globalPty) = 0;

	// 更新渲染绘制信息（媒体滚动位置等）
	virtual void UpdateDrawData() = 0;

	virtual std::shared_ptr<SMediaPty> GetMediaPty() = 0;

	virtual SRenderItemTex GetDrawData() = 0;

	// 发布句柄（拷贝 SHARED 面）是否在上次消费后发生过重建；true 时需刷新 Spout 等外部订阅
	virtual bool ConsumePublishTexDirty() { return false; }

	virtual void UpdateMediaPtyByRedoUndo(std::shared_ptr<SMediaPty> pMediaPty) = 0;
};

struct SMediaScrollParam;
class IMediaScrollRenderItem
{
public:
	virtual ~IMediaScrollRenderItem() {};
};
