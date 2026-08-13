#pragma once
#include "CommonDefine.h"
#include "ICopyInterface.h"
#include "Common/Common/Maths/Point.h"
#include "Common/Common/ICustomDataInterface.h"

#include <QUuid>
#include <QRect>
#include <vector>
#include <QDomNode>
#include <memory>
#include <rapidjson/document.h>

#include "IMediaPropertyInterface.h"

class IMediaNode :public ICopyInterface, public IManipulableObject
{
public:
	virtual ~IMediaNode() {}

	virtual void SetMediaName(QString name) = 0;
	virtual QString GetMediaName() = 0;

	virtual void SetMediaId(const QUuid& id) = 0;
	virtual QUuid GetMediaId() = 0;

	virtual void SetResourceType(int iType) = 0;
	virtual int GetResourceType() = 0;

	virtual void SetResourceId(const QUuid& id) = 0;
	virtual QUuid GetResourceId() = 0;

	virtual void SetDrawRect(const QRect& rect) = 0;
	virtual QRect GetDrawRect() = 0;

	virtual bool IsPtInMedia(std::vector<CPointD>pts) = 0;
	virtual bool HasMediaPeakInRect(std::vector<CPointD>& arrBoxRectPt) = 0;
	virtual bool IsMediaIntersect(std::vector<CPointD>& arPts) = 0;

	virtual void SetChangedState(const bool b) = 0;
	virtual bool GetChangedState() = 0;

	virtual bool LoadDataFromRapidJson(const rapidjson::Value &root) = 0;
	virtual void SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator) = 0;

	virtual CPointD GetRotateRectCenter(float fradio) = 0;
	virtual CPointD GetRotateRectCenterWithoutRotate(float fradio) = 0;

	virtual CPointD GetDeleteIconPos(float fradio) = 0;
	virtual CPointD GetDeleteIconPosWithoutRotate(float fradio) =0;

	virtual CPointD GetCenter() = 0;
	virtual CPointD GetCenterWithoutRotate() = 0;
	/**
		* @brief 获取媒体分段旋转后的中心位置
		* @param segmentRect 媒体分段区域
		* @return 返回媒体分段旋转后的中心位置
		* @note 纯虚接口，子类必须实现
	*/
	virtual CPointD GetSegmentCenter(const QRectF& segmentRect) = 0;

	virtual std::vector<CPointD> GetDrawSizeRectWithRotate() = 0;
	virtual std::vector<CPointD> GetDrawSizeRectWithoutRotate() = 0;
	virtual std::vector<QPointF> GetDrawSizeRect() = 0;

	virtual QRectF GetWrapRect() = 0;

	virtual bool GetHorizontal() = 0;
	virtual bool GetVertical() = 0;

	virtual CPointD GetMediaPos() = 0;
	virtual void	SetMediaPos(float valueX,float valueY) = 0;

	virtual float	GetMediaWidth() = 0;
	virtual void	SetMediaWidth(float value) = 0;

	virtual float	GetMediaHeight() = 0;
	virtual void	SetMediaHeight(float value) = 0;

	virtual float	GetMediaRotateAngle() = 0;
	virtual void	SetMediaRotateAngle(float value) = 0;

	virtual float	GetVolumn() = 0;
	virtual void	SetVolumn(float value) = 0;

	virtual std::chrono::milliseconds	GetClipStartTime() = 0;
	virtual void	SetClipStartTime(const std::chrono::milliseconds& value) = 0;

	virtual std::chrono::milliseconds	GetClipEndTime() = 0;
	virtual void	SetClipEndTime(const std::chrono::milliseconds& value) = 0;

	virtual void	SetOriginalSize(QSize& sz) = 0;

	virtual void	Move(float xoffset, float yoffset) = 0;
	virtual void	Scale(float xradio, float yradio, float centerx, float centery) = 0;
	virtual void	Rotate(float angleoffset) = 0;

	virtual std::shared_ptr<IMediaBaseProperty> GetBaseProperty() = 0;
	virtual std::shared_ptr<IEffectProperty> GetEffectProperty() = 0;
	virtual std::shared_ptr<IFxProperty> GetFxProperty() = 0;

	virtual void SetShareProperty(std::shared_ptr<IShareProperty> pShareProperty) = 0;
	virtual std::shared_ptr<IShareProperty> GetShareProperty() = 0;

	// 声道映射（媒体节点级，非共享属性）
	virtual void SetUsedChannelMap(bool bUse) = 0;
	virtual bool GetUsedChannelMap() = 0;
	virtual void SetAudioChannelMap(const SChannelMap& map) = 0;
	virtual SChannelMap GetAudioChannelMap() = 0;
};

class IMediaModel :public ICopyInterface
{
public:
	virtual ~IMediaModel() {}


	//获取媒体,获取所有媒体的QUuid
	virtual std::vector<QUuid> GetAllMediaId() = 0;
	virtual std::weak_ptr<IMediaNode> GetMediaNode(const QUuid &id) = 0;

	virtual std::weak_ptr<IEffectProperty> GetGlobalProperty() = 0;

	// 获取对应素材的所有媒体id
	virtual std::vector<QUuid> GetResMeadiaIds(const QUuid& resId) = 0;

	// 判断当前媒体是否已经创建
	virtual bool IsMediaExist(const QUuid &uuid) = 0;
	virtual bool IsMediaExistByPlayerId(const QUuid &uuid) = 0;

	virtual D2D1_RECT_D GetBoundingRectPts(const std::vector<QUuid>& vecId) = 0;

	virtual	IMediaNode* CreateReplaceMedia(const QUuid & mediaId) = 0;
	virtual	void DeleteMediaNode(const QUuid &id) = 0;
	virtual	void ClearMediaNodes() = 0;

	virtual	void AddMedia(std::shared_ptr<IMediaNode> ptr) = 0;
	virtual	std::shared_ptr<IMediaNode> CreateMediaNode() = 0;

	virtual void ResetGlobalProperty() = 0;

	//Model数据保存与加载
	virtual void SaveMediaModelToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator) = 0;
	virtual void LoadMediaModelFromRapidJson(const rapidjson::Value &root) = 0;
};
