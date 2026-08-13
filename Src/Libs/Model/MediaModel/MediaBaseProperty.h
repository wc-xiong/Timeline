#pragma once
#include "MediaModelGlobal.h"
#include <QString>
#include <QTime>
#include <QPoint>
#include <QSize>
#include <QPointF>
#include <QVector>
#include <mutex>
#include <QRectF>
#include <QPixmap>

#include "Common/IMediaPropertyInterface.h"
#include "Common/CommonDefine.h"

class MEDIAMODEL_EXPORT CMediaBaseProperty
	: public IMediaBaseProperty
	, public std::enable_shared_from_this<CMediaBaseProperty>
{
public:
	CMediaBaseProperty();
	virtual ~CMediaBaseProperty();

	CMediaBaseProperty(const CMediaBaseProperty& data);					 // 拷贝构造
	CMediaBaseProperty& operator=(const CMediaBaseProperty& data);		 // 拷贝赋值

	CMediaBaseProperty(std::shared_ptr<IMediaBaseProperty> pdata);		 // 构造

public:
	bool IsDirty() override;
	void SetDirty(bool b) override;

	void SetResId(const QUuid& id) override;
	QUuid GetResId() override;

	void SetMediaId(const QUuid& id)override {
		m_mediaId = id;
	}
	QUuid GetMediaId() override {
		return m_mediaId;
	}

	void SetResType(int iType) override;
	int GetResType() override;

	QString& GetName() override;
	void SetName(const QString& name) override;

	bool IsHWDEC()override;
	std::vector<QString> GetInfo() override;

	void SetDecodingMode(int mode) override;
	int GetDecodingMode() override;

	QPixmap GetThumbnail();

	void SetOriginalSize(QSize& sz) override;
	QSize GetOriginalSize() override;

	void SetRotatable(bool bRotatable) override;
	bool GetRotatable() override;

	void SetLocked(bool bLocked) override;
	bool GetLocked() override;

	void SetFixed(bool bLocked) override;
	bool GetFixed() override;

	void SetHided(bool bHided) override;
	bool GetHided() override;

	void SetPos(QPoint ptPos) override;
	QPointF GetPos() override;

	void SetSize(QSize szSize) override;
	QSizeF GetSize() override;

	void SetRotate(float nRotate) override;
	float GetRotate() override;

	void SetKeepAspectRatio(bool bKeepAspectRatio) override;
	bool GetKeepAspectRatio() override;

	void SetSizeKeepAspectRatio(QSizeF szSize) override;
	QSizeF GetSizeKeepAspectRatio() override;

	void SetFillType(int type) override;
	int GetFillType() override;

	void SetBlendType(int type) override;
	int GetBlendType() override;

	void SetWidth(int iValue) override;
	int GetWidth() const override;

	void SetHeight(int iValue) override;
	int GetHeight() const override;

	void SetHorizontal(bool b)override;
	bool GetHorizontal()override;

	void SetVertical(bool b)override;
	bool GetVertical()override;

	QRectF GetWrapRect();
	void GetPoints(std::vector<QPointF>&vecPoints);
	void GetPointsWithoutFilp(std::vector<QPointF>&vecPoints);

	QPointF GetFixedRectCenterWithoutRotate(float fradio);
	QPointF GetLockRectCenterWithoutRotate(float fradio);
	QPointF GetFxRectCenterWithoutRotate(float fradio);
	QPointF GetRotateRectCenter(float fradio);

	QPointF GetCenter();

	void Move(float xoffset, float yoffset);
	void Scale(float xradio, float yradio, float centerx, float centery);
	void Scale(float xradio, float yradio);
	void Rotate(float angleoffest, float centerx, float centery);
	void Rotate(float angleoffset);

	virtual void SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator) override;
	virtual void LoadDataFromRapidJson(const rapidjson::Value &root) override;

	QRectF GetSizeRect() override;

	void Replace() override;

	virtual void CopyProperty(std::shared_ptr<IMediaBaseProperty> pNode) override;

	//网页媒体移动回调函数

private:
	std::mutex m_mutex;

	QUuid m_resId;			// 对应素材库资源ID
	QUuid m_mediaId;			// 媒体ID
	int m_nResType = -1;

	//基础属性
	QString m_strName;		    // 媒体名称
	int     m_pixFormat;		// 素材格式
	QSize	m_OriginalSize;		// 素材原始大小
	int     m_nDecodingMode = ED_Auto;	// 解码偏好（EDecoding）

	bool m_bRotatable;	// 是否旋转
	bool m_bLocked;		// 是否锁定
	bool m_bFixed;		// 是否钉住
	bool m_bHided;		// 是否隐藏
	int m_nFillType;	// 填充类型
	int m_nBlendType;	// 混合模式

	bool m_bHorFilp = false;    // 水平翻转
	bool m_bVerFilp = false;	// 垂直翻转
	
	QPointF m_ptPos;		// 媒体位置
	QSizeF m_szSize;		// 媒体大小
	float m_nRotate;		// 旋转角度
	bool m_bKeepAspectRatio = false;
	QSizeF m_sizeKeepAspectRatio; // 保持比例基准

	bool m_bDirty = true;

	std::vector<QString> m_vecResInfo;	// 素材信息，防止重复从素材库获取
	QPixmap m_pixRes;	// 素材缩略图，防止重复从素材库获取
};

inline bool CMediaBaseProperty::IsDirty() { return m_bDirty; }
inline void CMediaBaseProperty::SetDirty(bool b) { m_bDirty = b; }
inline void CMediaBaseProperty::SetResId(const QUuid& id) { m_resId = id;}
inline QUuid CMediaBaseProperty::GetResId() { return m_resId; }
inline void CMediaBaseProperty::SetResType(int iType) { m_nResType = iType;}
inline int CMediaBaseProperty::GetResType()	{	return m_nResType;	}
inline void CMediaBaseProperty::SetName(const QString& name) { m_strName = name; }
inline QString& CMediaBaseProperty::GetName(){ return m_strName; }
inline void CMediaBaseProperty::SetDecodingMode(int mode) { m_nDecodingMode = mode; }
inline int CMediaBaseProperty::GetDecodingMode() { return m_nDecodingMode; }
inline void CMediaBaseProperty::SetOriginalSize(QSize & sz) { m_OriginalSize = sz; }
inline void CMediaBaseProperty::SetRotatable(bool bRotatable) { m_bDirty = true; m_bRotatable = bRotatable; }
inline bool CMediaBaseProperty::GetRotatable() { return m_bRotatable; }
inline void CMediaBaseProperty::SetLocked(bool bLocked) { m_bDirty = true; m_bLocked = bLocked; }
inline bool CMediaBaseProperty::GetLocked() { return m_bLocked; }
inline void CMediaBaseProperty::SetFixed(bool bFixed) { m_bDirty = true; m_bFixed = bFixed; }
inline bool CMediaBaseProperty::GetFixed() { return m_bFixed; }
inline void CMediaBaseProperty::SetHided(bool bHided){ m_bDirty = true; m_bHided = bHided; }
inline bool CMediaBaseProperty::GetHided(){ return m_bHided; }
inline QPointF CMediaBaseProperty::GetPos() { return m_ptPos; }
inline QSizeF CMediaBaseProperty::GetSize() { return m_szSize; }
inline float CMediaBaseProperty::GetRotate() { return m_nRotate; }
inline void CMediaBaseProperty::SetKeepAspectRatio(bool bKeepAspectRatio) { m_bKeepAspectRatio = bKeepAspectRatio; }
inline bool CMediaBaseProperty::GetKeepAspectRatio() { return m_bKeepAspectRatio; }
inline void CMediaBaseProperty::SetSizeKeepAspectRatio(QSizeF szSize) { m_sizeKeepAspectRatio = szSize; }
inline QSizeF CMediaBaseProperty::GetSizeKeepAspectRatio() { return m_sizeKeepAspectRatio; }
inline void CMediaBaseProperty::SetFillType(int type) { m_bDirty = true; m_nFillType = type; }
inline int CMediaBaseProperty::GetFillType() { return m_nFillType; }
inline void CMediaBaseProperty::SetBlendType(int type) { m_bDirty = true; m_nBlendType = type; }
inline int CMediaBaseProperty::GetBlendType() { return m_nBlendType; }
inline int CMediaBaseProperty::GetWidth() const { return m_szSize.toSize().width(); }
inline int CMediaBaseProperty::GetHeight() const { return m_szSize.toSize().height(); }

inline void CMediaBaseProperty::SetHorizontal(bool b) { m_bDirty = true; m_bHorFilp = b; }
inline bool CMediaBaseProperty::GetHorizontal() { return m_bHorFilp; }

inline void CMediaBaseProperty::SetVertical(bool b) { m_bDirty = true; m_bVerFilp = b; }
inline bool CMediaBaseProperty::GetVertical() { return m_bVerFilp; }
