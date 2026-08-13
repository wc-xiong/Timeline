#pragma once
#include "MediaModelGlobal.h"

#include "Common/CommonDefine.h"
#include "Common/IMediaInterface.h"
#include "Common/ChannelMap.h"
#include <QPointF>
#include <QVector2D> 
#include <QMatrix4x4>
#include <mutex>
#define Xml_MediaModel_MediaId QString("MediaId")
#define Xml_MediaModel_PlayerId QString("PlayerId")
#define Xml_MediaModel_ScreenID QString("ScreenID")

class MEDIAMODEL_EXPORT CMediaNode: public IMediaNode
{
	INIT_ICOPYINTERFACE(CMediaNode)
public:
	CMediaNode(const QUuid& id);
	CMediaNode(const CMediaNode& node);											
	CMediaNode& operator=(const CMediaNode& node);
	virtual ~CMediaNode();

public:

	virtual void SetMediaName(QString name) override;
	virtual QString GetMediaName() override;

	void SetMediaId(const QUuid& id) override;
	QUuid GetMediaId() override;

	void SetResourceId(const QUuid& id) override;
	QUuid GetResourceId() override;

	void SetResourceType(int iType) override;
	int GetResourceType() override;

	void SetDrawRect(const QRect& rect) override;
	QRect GetDrawRect() override;

	void SetChangedState(const bool b) override;
	bool GetChangedState() override;

	virtual std::vector<CPointD>GetDrawSizeRectWithRotate()override;		//获取媒体外接矩阵
	virtual std::vector<CPointD>GetDrawSizeRectWithoutRotate()override;
	virtual std::vector<QPointF>GetDrawSizeRect()override;

	virtual QRectF GetWrapRect() override;

	virtual bool GetHorizontal() override;
	virtual bool GetVertical() override;

	virtual bool IsPtInMedia(std::vector<CPointD>pts) override;				//判断点集是否在媒体矩形中
	bool CMediaNode::IsPtInRect(std::vector<CPointD>& pts, std::vector<CPointD>& rectPts);

	virtual bool HasMediaPeakInRect(std::vector<CPointD>& arrBoxRectPt) override;
	virtual bool IsMediaIntersect(std::vector<CPointD>& arPts) override;

	virtual CPointD GetMediaPos() override;
	virtual void	SetMediaPos(float valueX, float valueY) override;

	virtual CPointD GetRotateRectCenter(float fradio) override;
	virtual CPointD GetRotateRectCenterWithoutRotate(float fradio) override;

	virtual CPointD GetDeleteIconPos(float fradio) override;
	virtual CPointD GetDeleteIconPosWithoutRotate(float fradio) override;

	virtual CPointD GetCenter() override;
	virtual CPointD GetSegmentCenter(const QRectF& segmentRect) override;
	virtual CPointD GetCenterWithoutRotate() override;

	virtual float	GetMediaWidth() override;
	virtual void	SetMediaWidth(float value) override;

	virtual float	GetMediaHeight() override;
	virtual void	SetMediaHeight(float value) override;

	virtual float	GetMediaRotateAngle() override;
	virtual void	SetMediaRotateAngle(float value) override;

	virtual float	GetVolumn() override;
	virtual void	SetVolumn(float value) override;

	virtual std::chrono::milliseconds	GetClipStartTime() override;
	virtual void	SetClipStartTime(const std::chrono::milliseconds& value) override;

	virtual std::chrono::milliseconds	GetClipEndTime() override;
	virtual void	SetClipEndTime(const std::chrono::milliseconds& value) override;

	virtual void	SetOriginalSize(QSize& sz) override;

	void Move(float xoffset, float yoffset)override;
	void Scale(float xradio, float yradio, float centerx, float centery)override;
	void Scale(float xradio, float yradio);
	void Rotate(float angleoffest, float centerx, float centery);
	void Rotate(float angleoffset)override;

	virtual bool LoadDataFromRapidJson(const rapidjson::Value &root) override;
	virtual void SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator) override;

	//切片视图操作对象框架接口	
	virtual CPointD					GetRotateCenter(bool bInput)override;
	virtual double					GetRotateAngle(bool bInput)override;					//旋转角度	
	virtual CPointD					GetRotateControl(bool bInput)override;					//旋转控制点	
	virtual CPointD					GetDeleteControl(bool bInput, float fradio)override;	//删除控制点
	virtual std::vector<CPointD>	GetSizePoint(bool bInput)override;						//缩放控制点
	virtual bool					IsPtInSide(const CPointD& pt, bool bInput = true)override;		//点是否在对象内部

	std::shared_ptr<IMediaBaseProperty> GetBaseProperty() override;
	std::shared_ptr<IEffectProperty> GetEffectProperty() override;
	std::shared_ptr<IFxProperty> GetFxProperty();

	void SetShareProperty(std::shared_ptr<IShareProperty> pShareProperty) override;
	std::shared_ptr<IShareProperty> GetShareProperty() override;

	void SetUsedChannelMap(bool bUse) override;
	bool GetUsedChannelMap() override;
	void SetAudioChannelMap(const SChannelMap& map) override;
	SChannelMap GetAudioChannelMap() override;

private:
	void UpdateDrawRect();

private:
	QString m_mediaName;
	QUuid	m_MediaId;				// 对应媒体ID
	QUuid	m_ResourceId;				// 对应素材库资源ID
	int m_resType = -1;

	QRect	m_DrawRect;				// 绘制的位置和大小
	bool	m_bChanged = false;		// 是否被切换过对应的PlayerID

	bool	m_bHorFilp = false;		// 水平翻转
	bool	m_bVerFilp = false;		// 垂直翻转

	QPointF		m_ptPos;			// 媒体位置
	QSizeF		m_szSize;			// 媒体大小
	float		m_nRotate = 0;		// 旋转角度

	std::chrono::milliseconds m_clipStartTime{0};
	std::chrono::milliseconds m_clipEndTime{0};

	bool		m_bUseChannelMap = false;	// 是否使用声道映射
	SChannelMap	m_channelMap;				// 声道映射参数

	std::shared_ptr<IMediaBaseProperty> m_pBaseProperty = nullptr;			// 基础属性
	std::shared_ptr<IEffectProperty> m_pEffectProperty = nullptr;			// 效果属性
	std::shared_ptr<IShareProperty> m_pShareProperty = nullptr;				// 共享属性
	std::shared_ptr<IFxProperty> m_pFxProperty = nullptr;					// 特效属性

	std::mutex	m_mutex;
};
