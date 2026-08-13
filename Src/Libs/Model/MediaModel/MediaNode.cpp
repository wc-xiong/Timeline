#include "MediaNode.h"
#include "Common/CommonFunctionMath.h"

#include <algorithm>
#include "Transform.h"
#include "MediaBaseProperty.h"
#include "EffectProperty.h"
#include "ShareProperty.h"
#include "FxProperty.h"

#include "Common/SignalCenter/GSNetSignalCenter.h"
#include "Net/NetCommon/TransDataDefine.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

#define Xml_MediaModel_MediaId QString("MediaId")
#define Xml_MediaModel_PlayerId QString("PlayerId")
#define Xml_MediaModel_MediaRect QString("MediaRect")
#define Xml_MediaModel_MediaPosX QString("MediaPosX")
#define Xml_MediaModel_MediaPosY QString("MediaPosY")
#define Xml_MediaModel_MediaWidth QString("MediaWidth")
#define Xml_MediaModel_MediaHeight QString("MediaHeight")
#define Xml_MediaModel_MediaAngle QString("MediaAngle")
#define Xml_MediaModel_MediaName QString("MediaName")
#define Xml_MediaModel_MediaResourceType QString("ResourceType")

#define Json_MediaModel_ClipBegin QString("MediaClipBegin")
#define Json_MediaModel_ClipEnd QString("MediaClipEnd")
#define Json_MediaModel_Volumn QString("MediaVolumn")

#define Xml_MediaModel_BaseProperty        QString("BaseProperty")         // 基础属性
#define Xml_MediaModel_EffectProperty      QString("EffectProperty")       // 特效属性
#define Xml_MediaModel_ShareProperty       QString("ShareProperty")        // 共享属性
#define Xml_MediaModel_FxProperty          QString("FxProperty")           // Fx特效属性

#define Json_MediaModel_UsedChannelMap     QString("UsedChannelMap")
#define Json_MediaModel_ChannelType        QString("ChannelType")
#define Json_MediaModel_ChannelMap         QString("ChannelMap")

#define Media_BitMapLocation_Y_Rotate 200
#define Media_BitMapLocation_X_Delete 20
#define Media_BitMapLocation_Y_Delete 12

CMediaNode::CMediaNode(const QUuid& id)
	: m_pBaseProperty(new CMediaBaseProperty)
	, m_pEffectProperty(new CEffectProperty)
	, m_pShareProperty(new CShareProperty)
	, m_pFxProperty(new CFxProperty)
{
	m_MediaId = id;

	m_pBaseProperty->SetMediaId(m_MediaId);
}

CMediaNode::~CMediaNode()
{

}

CMediaNode::CMediaNode(const CMediaNode & node)
{
	m_MediaId = node.m_MediaId;
	m_ResourceId = node.m_ResourceId;
	m_resType = node.m_resType;

	m_DrawRect = node.m_DrawRect;
	m_bChanged = node.m_bChanged;

	m_bHorFilp = node.m_bHorFilp;
	m_bVerFilp = node.m_bVerFilp;

	m_ptPos = node.m_ptPos;
	m_nRotate = node.m_nRotate;
	m_szSize = node.m_szSize;

	m_mediaName = node.m_mediaName;

	m_clipStartTime = node.m_clipStartTime;
	m_clipEndTime = node.m_clipEndTime;

	m_bUseChannelMap = node.m_bUseChannelMap;
	m_channelMap = node.m_channelMap;

	m_pBaseProperty.reset(new CMediaBaseProperty(node.m_pBaseProperty));
	m_pEffectProperty.reset(new CEffectProperty(node.m_pEffectProperty));
	m_pShareProperty.reset(new CShareProperty(node.m_pShareProperty));
	m_pFxProperty.reset(new CFxProperty(node.m_pFxProperty));

	m_pBaseProperty->SetMediaId(m_MediaId);

	m_pBaseProperty->SetResId(m_ResourceId);
	m_pBaseProperty->SetResType(m_resType);
}

CMediaNode & CMediaNode::operator=(const CMediaNode & node)
{
	if (this == &node)
		return *this;

	m_MediaId = node.m_MediaId;
	m_ResourceId = node.m_ResourceId;
	m_resType = node.m_resType;

	m_DrawRect = node.m_DrawRect;
	m_bChanged = node.m_bChanged;

	m_bHorFilp = node.m_bHorFilp;
	m_bVerFilp = node.m_bVerFilp;

	m_ptPos = node.m_ptPos;
	m_nRotate = node.m_nRotate;
	m_szSize = node.m_szSize;

	m_mediaName = node.m_mediaName;

	m_clipStartTime = node.m_clipStartTime;
	m_clipEndTime = node.m_clipEndTime;

	m_bUseChannelMap = node.m_bUseChannelMap;
	m_channelMap = node.m_channelMap;

	m_pBaseProperty = node.m_pBaseProperty;
	m_pEffectProperty = node.m_pEffectProperty;
	m_pShareProperty = node.m_pShareProperty;
	m_pFxProperty = node.m_pFxProperty;

	m_pBaseProperty->SetMediaId(m_MediaId);

	m_pBaseProperty->SetResId(m_ResourceId);
	m_pBaseProperty->SetResType(m_resType);

	return *this;
}

void CMediaNode::SaveDataToRapidJson(rapidjson::Value &rapidRoot, rapidjson::Document::AllocatorType &allocator)
{
	rapidRoot.SetObject();
	rapidRoot.AddMember(rapidjson::Value(Xml_MediaModel_MediaId.toUtf8().constData(), allocator),
		rapidjson::Value(m_MediaId.toString().toUtf8().constData(), allocator), allocator);
	rapidRoot.AddMember(rapidjson::Value(Xml_MediaModel_PlayerId.toUtf8().constData(), allocator),
		rapidjson::Value(m_ResourceId.toString().toUtf8().constData(), allocator), allocator);
	rapidRoot.AddMember(rapidjson::Value(Xml_MediaModel_MediaName.toUtf8().constData(), allocator),
		rapidjson::Value(m_mediaName.toUtf8().constData(), allocator), allocator);
	rapidRoot.AddMember(rapidjson::Value(Xml_MediaModel_MediaResourceType.toUtf8().constData(), allocator), m_resType, allocator);

	rapidjson::Value rectArray(rapidjson::kArrayType);
	rectArray.PushBack(static_cast<int>(m_ptPos.x()), allocator);
	rectArray.PushBack(static_cast<int>(m_ptPos.y()), allocator);
	rectArray.PushBack(static_cast<int>(m_szSize.width()), allocator);
	rectArray.PushBack(static_cast<int>(m_szSize.height()), allocator);
	rectArray.PushBack(static_cast<int>(m_nRotate), allocator);
	rapidRoot.AddMember(rapidjson::Value(Xml_MediaModel_MediaRect.toUtf8().constData(), allocator), rectArray, allocator);

	rapidRoot.AddMember(rapidjson::Value(Json_MediaModel_ClipBegin.toUtf8().constData(), allocator), static_cast<int64_t>(m_clipStartTime.count()), allocator);
	rapidRoot.AddMember(rapidjson::Value(Json_MediaModel_ClipEnd.toUtf8().constData(), allocator), static_cast<int64_t>(m_clipEndTime.count()), allocator);

	rapidRoot.AddMember(rapidjson::Value(Json_MediaModel_UsedChannelMap.toUtf8().constData(), allocator), m_bUseChannelMap, allocator);
	rapidRoot.AddMember(rapidjson::Value(Json_MediaModel_ChannelType.toUtf8().constData(), allocator), static_cast<int>(m_channelMap.type), allocator);
	rapidjson::Value channelMapAr(rapidjson::kArrayType);
	for (int i = 0; i < CHANNEL_SYSTEM_MAX; ++i)
		channelMapAr.PushBack(static_cast<int>(m_channelMap.map[i]), allocator);
	rapidRoot.AddMember(rapidjson::Value(Json_MediaModel_ChannelMap.toUtf8().constData(), allocator), channelMapAr, allocator);

	rapidjson::Value baseRapid(rapidjson::kObjectType);
	rapidjson::Value effectRapid(rapidjson::kObjectType);
	rapidjson::Value shareRapid(rapidjson::kObjectType);
	rapidjson::Value fxRapid(rapidjson::kObjectType);

	m_pBaseProperty->SaveDataToRapidJson(baseRapid, allocator);
	m_pEffectProperty->SaveDataToRapidJson(effectRapid, allocator);
	m_pShareProperty->SaveDataToRapidJson(shareRapid, allocator);
	m_pFxProperty->SaveDataToRapidJson(fxRapid, allocator);

	rapidRoot.AddMember(rapidjson::Value(Xml_MediaModel_BaseProperty.toUtf8().constData(), allocator), baseRapid, allocator);
	rapidRoot.AddMember(rapidjson::Value(Xml_MediaModel_EffectProperty.toUtf8().constData(), allocator), effectRapid, allocator);
	rapidRoot.AddMember(rapidjson::Value(Xml_MediaModel_ShareProperty.toUtf8().constData(), allocator), shareRapid, allocator);
	rapidRoot.AddMember(rapidjson::Value(Xml_MediaModel_FxProperty.toUtf8().constData(), allocator), fxRapid, allocator);
}

bool CMediaNode::LoadDataFromRapidJson(const rapidjson::Value& root)
{
	const QByteArray idKey = Xml_MediaModel_MediaId.toUtf8();
	const QByteArray playerKey = Xml_MediaModel_PlayerId.toUtf8();
	const QByteArray nameKey = Xml_MediaModel_MediaName.toUtf8();
	const QByteArray resTypeKey = Xml_MediaModel_MediaResourceType.toUtf8();
	const QByteArray rectKey = Xml_MediaModel_MediaRect.toUtf8();
	const QByteArray clipBeginKey = Json_MediaModel_ClipBegin.toUtf8();
	const QByteArray clipEndKey = Json_MediaModel_ClipEnd.toUtf8();
	const QByteArray volumeKey = Json_MediaModel_Volumn.toUtf8();
	const QByteArray baseKey = Xml_MediaModel_BaseProperty.toUtf8();
	const QByteArray effectKey = Xml_MediaModel_EffectProperty.toUtf8();
	const QByteArray shareKey = Xml_MediaModel_ShareProperty.toUtf8();
	const QByteArray fxKey = Xml_MediaModel_FxProperty.toUtf8();

	if (!root.IsObject())
		return false;

	if (root.HasMember(idKey.constData()) && root[idKey.constData()].IsString())
		m_MediaId = QUuid(QString::fromUtf8(root[idKey.constData()].GetString()));

	if (root.HasMember(playerKey.constData()) && root[playerKey.constData()].IsString())
		m_ResourceId = QUuid(QString::fromUtf8(root[playerKey.constData()].GetString()));

	if (root.HasMember(nameKey.constData()) && root[nameKey.constData()].IsString())
		m_mediaName = QString::fromUtf8(root[nameKey.constData()].GetString());

	if (root.HasMember(resTypeKey.constData()) && root[resTypeKey.constData()].IsInt())
		m_resType = root[resTypeKey.constData()].GetInt();

	if (root.HasMember(rectKey.constData()) && root[rectKey.constData()].IsArray())
	{
		const rapidjson::Value& rectArray = root[rectKey.constData()];
		if (rectArray.Size() >= 5)
		{
			const int x = rectArray[0].GetInt();
			const int y = rectArray[1].GetInt();
			const int width = rectArray[2].GetInt();
			const int height = rectArray[3].GetInt();
			const int angle = rectArray[4].GetInt();
			m_ptPos = QPointF(x, y);
			m_szSize = QSizeF(width, height);
			m_nRotate = angle;
		}
	}

	if (root.HasMember(clipBeginKey.constData()) && root[clipBeginKey.constData()].IsNumber())
		m_clipStartTime = std::chrono::milliseconds(root[clipBeginKey.constData()].GetInt64());

	if (root.HasMember(clipEndKey.constData()) && root[clipEndKey.constData()].IsNumber())
		m_clipEndTime = std::chrono::milliseconds(root[clipEndKey.constData()].GetInt64());

	const QByteArray usedChannelMapKey = Json_MediaModel_UsedChannelMap.toUtf8();
	const QByteArray channelTypeKey = Json_MediaModel_ChannelType.toUtf8();
	const QByteArray channelMapKey = Json_MediaModel_ChannelMap.toUtf8();

	if (root.HasMember(usedChannelMapKey.constData()) && root[usedChannelMapKey.constData()].IsBool())
		m_bUseChannelMap = root[usedChannelMapKey.constData()].GetBool();
	if (root.HasMember(channelTypeKey.constData()) && root[channelTypeKey.constData()].IsInt())
		m_channelMap.type = static_cast<unsigned>(root[channelTypeKey.constData()].GetInt());
	if (root.HasMember(channelMapKey.constData()) && root[channelMapKey.constData()].IsArray())
	{
		const rapidjson::Value& channelMapParam = root[channelMapKey.constData()];
		const int count = std::min<int>(CHANNEL_SYSTEM_MAX, static_cast<int>(channelMapParam.Size()));
		for (int i = 0; i < count; ++i)
		{
			if (channelMapParam[i].IsInt())
				m_channelMap.map[i] = static_cast<unsigned>(channelMapParam[i].GetInt());
		}
	}

	if (root.HasMember(baseKey.constData()) && root[baseKey.constData()].IsObject())
	{
		m_pBaseProperty->LoadDataFromRapidJson(root[baseKey.constData()]);
		m_pBaseProperty->SetName(m_mediaName);
		m_pBaseProperty->SetResId(m_ResourceId);
		m_pBaseProperty->SetResType(m_resType);
		m_pBaseProperty->SetMediaId(m_MediaId);
		m_pBaseProperty->SetPos(QPoint(m_ptPos.x(), m_ptPos.y()));
		m_pBaseProperty->SetSize(QSize(m_szSize.width(), m_szSize.height()));
		m_pBaseProperty->SetRotate(m_nRotate);
	}

	if (root.HasMember(effectKey.constData()) && root[effectKey.constData()].IsObject())
		m_pEffectProperty->LoadDataFromRapidJson(root[effectKey.constData()]);
	if (root.HasMember(shareKey.constData()) && root[shareKey.constData()].IsObject())
		m_pShareProperty->LoadDataFromRapidJson(root[shareKey.constData()]);
	if (root.HasMember(fxKey.constData()) && root[fxKey.constData()].IsObject())
		m_pFxProperty->LoadDataFromRapidJson(root[fxKey.constData()]);

	UpdateDrawRect();
	return true;
}

void CMediaNode::UpdateDrawRect()
{
	QRect newRect(m_ptPos.x(), m_ptPos.y(), m_szSize.width(), m_szSize.height());
	m_DrawRect = newRect;
}

void CMediaNode::SetMediaName(QString name)
{
	m_mediaName = name;
	m_pBaseProperty->SetName(m_mediaName);
}

QString CMediaNode::GetMediaName()
{
	return m_mediaName;
}

inline void		CMediaNode::SetMediaId(const QUuid& id) { m_MediaId = id; m_pBaseProperty->SetMediaId(id); }
inline void		CMediaNode::SetResourceId(const QUuid& id) { m_ResourceId = id; m_pBaseProperty->SetResId(id); }
inline void		CMediaNode::SetChangedState(const bool b) { m_bChanged = b; }

inline QUuid	CMediaNode::GetMediaId() { return m_MediaId; }
inline QUuid	CMediaNode::GetResourceId() { return m_ResourceId; }

void CMediaNode::SetResourceType(int iType)
{
	m_resType = iType;
	m_pBaseProperty->SetResType(iType);
}

int CMediaNode::GetResourceType()
{
	return m_resType;
}

QRect CMediaNode::GetDrawRect()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_DrawRect;
}

void CMediaNode::SetDrawRect(const QRect& rect)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_DrawRect = rect;
	m_ptPos.setX(rect.x());
	m_ptPos.setY(rect.y());
	m_szSize.setWidth(rect.width());
	m_szSize.setHeight(rect.height());

	m_pBaseProperty->SetPos(QPoint(rect.x(), rect.y()));
	m_pBaseProperty->SetSize(QSize(rect.width(), rect.height()));
}
inline bool	CMediaNode::GetChangedState()
{
	if (m_bChanged)
	{
		m_bChanged = false;
		return !m_bChanged;;
	}

	return m_bChanged;
}
std::vector<QPointF> CMediaNode::GetDrawSizeRect()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	std::vector<QPointF> vecPoints;

	QPointF pt = QPointF(m_ptPos.x(), m_ptPos.y());
	vecPoints.push_back(pt);
	pt = QPointF(m_ptPos.x(), m_ptPos.y() + m_szSize.height());
	vecPoints.push_back(pt);
	pt = QPointF(m_ptPos.x() + m_szSize.width(), m_ptPos.y() + m_szSize.height());
	vecPoints.push_back(pt);
	pt = QPointF(m_ptPos.x() + m_szSize.width(), m_ptPos.y());
	vecPoints.push_back(pt);

	std::vector<QPointF>sizePoints;
	sizePoints.push_back(vecPoints[0]);
	sizePoints.push_back((vecPoints[0] + vecPoints[1]) * 0.5);
	sizePoints.push_back(vecPoints[1]);
	sizePoints.push_back((vecPoints[1] + vecPoints[2]) * 0.5);
	sizePoints.push_back(vecPoints[2]);
	sizePoints.push_back((vecPoints[2] + vecPoints[3]) * 0.5);
	sizePoints.push_back(vecPoints[3]);
	sizePoints.push_back((vecPoints[3] + vecPoints[0]) * 0.5);
	return sizePoints;
}

std::vector<CPointD> CMediaNode::GetDrawSizeRectWithRotate()
{
	std::vector<CPointD> arrSizePt;
	std::vector<QPointF>sizePoints;
	sizePoints = GetDrawSizeRect();
	QMatrix4x4 temp;
	temp.translate(m_ptPos.x(), m_ptPos.y());
	temp.rotate(m_nRotate, 0, 0, 1);
	temp.translate(-m_ptPos.x(), -m_ptPos.y());
	for (auto& pt : sizePoints)
		pt = temp.map(pt);

	for (auto i : sizePoints)
	{
		CPointD tmp(i.x(), i.y());
		arrSizePt.push_back(tmp);
	}
	if (m_bHorFilp)
	{
		std::swap(arrSizePt[0], arrSizePt[6]);
		std::swap(arrSizePt[1], arrSizePt[5]);
		std::swap(arrSizePt[2], arrSizePt[4]);
	}

	if (m_bVerFilp)
	{
		std::swap(arrSizePt[0], arrSizePt[2]);
		std::swap(arrSizePt[3], arrSizePt[7]);
		std::swap(arrSizePt[4], arrSizePt[6]);
	}
	return arrSizePt;
}

std::vector<CPointD> CMediaNode::GetDrawSizeRectWithoutRotate()
{

	std::vector<CPointD> arrSizePt;
	std::vector<QPointF>sizePoints;
	sizePoints = GetDrawSizeRect();
	for (auto i : sizePoints)
	{
		CPointD tmp(i.x(), i.y());
		arrSizePt.push_back(tmp);
	}
	return arrSizePt;
}

QRectF CMediaNode::GetWrapRect()
{
	QRectF rc;
	rc.setLeft(INT_MAX);
	rc.setRight(INT_MIN);
	rc.setTop(INT_MAX);
	rc.setBottom(INT_MIN);

	std::vector<CPointD>vecPoints;
	vecPoints = GetDrawSizeRectWithRotate();

	for (auto pt : vecPoints)
	{
		if (rc.left() > pt.x) rc.setLeft(pt.x);
		if (rc.right() < pt.x) rc.setRight(pt.x);
		if (rc.top() > pt.y) rc.setTop(pt.y);
		if (rc.bottom() < pt.y) rc.setBottom(pt.y);
	}

	return rc;
}

bool CMediaNode::GetHorizontal()
{
	return m_bHorFilp;
}

bool CMediaNode::GetVertical()
{
	return m_bVerFilp;
}

bool CMediaNode::IsPtInRect(std::vector<CPointD>& pts, std::vector<CPointD>& rectPts)
{
	if (rectPts.size() != 8) return false;
	for (auto i : pts) {
		if ((i.x >= rectPts[0].x && i.x <= rectPts[6].x) && (i.y >= rectPts[0].y && i.y <= rectPts[4].y)) return true;
	}
	return false;
}

bool CMediaNode::HasMediaPeakInRect(std::vector<CPointD>& arrBoxRectPt)
{
	std::vector<CPointD> vecPoint = GetSizePoint(true);

	std::lock_guard<std::mutex> lock(m_mutex);
	for (const auto& mediaPt : vecPoint)
	{
		if (G_Math_IsPtInPts(arrBoxRectPt, mediaPt))
			return true;
	}
	return false;
}

bool CMediaNode::IsMediaIntersect(std::vector<CPointD>& arPts)
{
	std::vector<CPointD> vecPoint = GetSizePoint(true);

	std::lock_guard<std::mutex> lock(m_mutex);
	int nMeidiaPtSize = vecPoint.size();

	// 判断多边形是否相交 - (不包含在内部）
	double x1, y1, x2, y2;
	for (int i = 0; i < arPts.size(); i++)
	{
		x1 = arPts[i].x;
		y1 = arPts[i].y;
		x2 = arPts[(i + 1) % arPts.size()].x;
		y2 = arPts[(i + 1) % arPts.size()].y;

		for (int j = 0; j < nMeidiaPtSize; j++)
		{
			CPointD ptCur = vecPoint[j];
			CPointD ptNext = vecPoint[(j + 1) % nMeidiaPtSize];

			if (G_Math_IsIntersect(x1, y1, x2, y2, ptCur.x, ptCur.y, ptNext.x, ptNext.y))
				return true;
		}
	}
	return false;
}

bool CMediaNode::IsPtInMedia(std::vector<CPointD> pts)
{
	std::vector<CPointD>vecPts;
	QMatrix4x4 temp;
	temp.translate(m_ptPos.x(), m_ptPos.y());
	temp.rotate(-m_nRotate, 0, 0, 1);
	temp.translate(-m_ptPos.x(), -m_ptPos.y());

	for (auto i : pts) {
		QPointF pt(i.x, i.y);
		pt = temp.map(pt);
		vecPts.push_back(CPointD(pt.x(), pt.y()));
	}
	return IsPtInRect(vecPts, GetDrawSizeRectWithoutRotate());
}

CPointD CMediaNode::GetMediaPos()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	CPointD pos;
	pos.x = m_DrawRect.x();
	pos.y = m_DrawRect.y();
	return pos;
}

void CMediaNode::SetMediaPos(float valueX, float valueY)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_ptPos = QPointF(valueX, valueY);
	UpdateDrawRect();

	QPoint pos(valueX, valueY);
	m_pBaseProperty->SetPos(pos);
	return;
}

CPointD CMediaNode::GetRotateRectCenter(float fradio)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	//QPointF pt = QPointF(m_ptPos.x() + m_szSize.width() / 2, m_ptPos.y() - Media_BitMapLocation_Y_Rotate * fradio);

	QPointF pt = QPointF(m_ptPos.x(), m_ptPos.y() - Media_BitMapLocation_Y_Rotate * fradio);

	QMatrix4x4 temp;
	temp.translate(m_ptPos.x(), m_ptPos.y());
	temp.rotate(m_nRotate, 0, 0, 1);
	temp.translate(-m_ptPos.x(), -m_ptPos.y());

	pt = temp.map(pt);


	return CPointD(pt.x(),pt.y());
}

CPointD CMediaNode::GetRotateRectCenterWithoutRotate(float fradio)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	QPointF pt = QPointF(m_ptPos.x() + m_szSize.width() / 2, m_ptPos.y() - Media_BitMapLocation_Y_Rotate * fradio);
	return CPointD(pt.x(), pt.y());
}

CPointD CMediaNode::GetDeleteIconPos(float fradio)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	QPointF pt(m_ptPos.x() + m_szSize.width() + Media_BitMapLocation_X_Delete * fradio, m_ptPos.y() + Media_BitMapLocation_Y_Delete * fradio);
	QMatrix4x4 temp;
	temp.translate(m_ptPos.x(), m_ptPos.y());
	temp.rotate(m_nRotate, 0, 0, 1);
	temp.translate(-m_ptPos.x(), -m_ptPos.y());

	pt = temp.map(pt);
	return CPointD(pt.x(), pt.y());
}

CPointD CMediaNode::GetDeleteIconPosWithoutRotate(float fradio)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	QPointF p(m_ptPos.x() + m_szSize.width() + Media_BitMapLocation_X_Delete * fradio, m_ptPos.y() + Media_BitMapLocation_Y_Delete * fradio);
	return CPointD(p.x(), p.y());
}

CPointD CMediaNode::GetCenter()
{

	QPointF pt = QPointF(m_ptPos.x() + m_szSize.width() / 2, m_ptPos.y() + m_szSize.height() / 2);
	QMatrix4x4 temp;
	temp.translate(m_ptPos.x(), m_ptPos.y());
	temp.rotate(m_nRotate, 0, 0, 1);
	temp.translate(-m_ptPos.x(), -m_ptPos.y());

	pt = temp.map(pt);

	return CPointD(pt.x(), pt.y());
}

CPointD CMediaNode::GetSegmentCenter(const QRectF& segmentRect)
{
	QPointF pt = QPointF(m_ptPos.x() + segmentRect.left() + segmentRect.width() / 2, 
		m_ptPos.y() + +segmentRect.top() + segmentRect.height() / 2);
	
	QMatrix4x4 temp;
	temp.translate(m_ptPos.x(), m_ptPos.y());
	temp.rotate(m_nRotate, 0, 0, 1);
	temp.translate(-m_ptPos.x(), -m_ptPos.y());

	pt = temp.map(pt);

	return CPointD(pt.x(), pt.y());
}

CPointD CMediaNode::GetCenterWithoutRotate()
{
	QPointF pt = QPointF(m_ptPos.x() + m_szSize.width() / 2, m_ptPos.y() + m_szSize.height() / 2);
	return CPointD(pt.x(), pt.y());
}

float CMediaNode::GetMediaWidth()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_szSize.width();
}

void CMediaNode::SetMediaWidth(float value)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_szSize.setWidth(value);
	m_pBaseProperty->SetWidth(value);
}

float CMediaNode::GetMediaHeight()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_szSize.height();
}

void CMediaNode::SetMediaHeight(float value)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_szSize.setHeight(value);
	m_pBaseProperty->SetHeight(value);
}

float CMediaNode::GetMediaRotateAngle()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_nRotate;
}

void CMediaNode::SetMediaRotateAngle(float value)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_nRotate = value;
}

float CMediaNode::GetVolumn()
{
	return m_pShareProperty->GetVolume();
}

void CMediaNode::SetVolumn(float value)
{
	if (m_pShareProperty)
		m_pShareProperty->SetVolume(value);
}

std::chrono::milliseconds CMediaNode::GetClipStartTime()
{
	return m_clipStartTime;
}

void CMediaNode::SetClipStartTime(const std::chrono::milliseconds& value)
{
	m_clipStartTime = value;
	emit g_pGSNetSignal->signalMediaNodeOp(ETDT_UpdateMediaNode, m_MediaId);
}

std::chrono::milliseconds CMediaNode::GetClipEndTime()
{
	return m_clipEndTime;
}

void CMediaNode::SetClipEndTime(const std::chrono::milliseconds& value)
{
	m_clipEndTime = value;
	emit g_pGSNetSignal->signalMediaNodeOp(ETDT_UpdateMediaNode, m_MediaId);
}

void CMediaNode::SetOriginalSize(QSize & sz)
{
	m_pBaseProperty->SetOriginalSize(sz);
}

void CMediaNode::Move(float xoffset, float yoffset)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	CTransformHelper temp(m_ptPos, m_szSize, m_nRotate);
	temp.Move(xoffset, yoffset);
	m_ptPos = temp.getPos();
	m_szSize = temp.getSize();
	m_nRotate = temp.getAngle();
	UpdateDrawRect();

	m_pBaseProperty->Move(xoffset, yoffset);

	emit g_pGSNetSignal->signalMediaNodeOp(ETDT_UpdateMediaNode, m_MediaId);
}

void CMediaNode::Scale(float xradio, float yradio, float centerx, float centery)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	if (xradio < 0)
	{
		m_bHorFilp = !m_bHorFilp;
	}
	if (yradio < 0)
	{
		m_bVerFilp = !m_bVerFilp;
	}

	CTransformHelper temp(m_ptPos, m_szSize, m_nRotate);
	temp.Scale(xradio, yradio, centerx, centery);
	m_ptPos = temp.getPos();
	m_szSize = temp.getSize();
	m_nRotate = temp.getAngle();
	UpdateDrawRect();

	m_pBaseProperty->Scale(xradio, yradio, centerx, centery);

	emit g_pGSNetSignal->signalMediaNodeOp(ETDT_UpdateMediaNode, m_MediaId);
}

void CMediaNode::Scale(float xradio, float yradio)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	if (xradio < 0) m_bHorFilp = !m_bHorFilp;
	if (yradio < 0) m_bVerFilp = !m_bVerFilp;

	CTransformHelper temp(m_ptPos, m_szSize, m_nRotate);
	temp.Scale(xradio, yradio);
	m_ptPos = temp.getPos();
	m_szSize = temp.getSize();
	m_nRotate = temp.getAngle();
	UpdateDrawRect();

	m_pBaseProperty->Scale(xradio, yradio);

	emit g_pGSNetSignal->signalMediaNodeOp(ETDT_UpdateMediaNode, m_MediaId);
}

void CMediaNode::Rotate(float angleoffset, float centerx, float centery)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	CTransformHelper temp(m_ptPos, m_szSize, m_nRotate);
	temp.Rotate(angleoffset, centerx, centery);
	m_ptPos = temp.getPos();
	m_szSize = temp.getSize();
	m_nRotate = temp.getAngle();
	UpdateDrawRect();

	m_pBaseProperty->Rotate(angleoffset, centerx, centery);

	emit g_pGSNetSignal->signalMediaNodeOp(ETDT_UpdateMediaNode, m_MediaId);
}

void CMediaNode::Rotate(float angleoffset)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	CTransformHelper temp(m_ptPos, m_szSize, m_nRotate);
	temp.Rotate(angleoffset, m_ptPos.x(), m_ptPos.y());
	m_ptPos = temp.getPos();
	m_szSize = temp.getSize();
	m_nRotate = temp.getAngle();
	UpdateDrawRect();
	m_pBaseProperty->Rotate(angleoffset);

	emit g_pGSNetSignal->signalMediaNodeOp(ETDT_UpdateMediaNode, m_MediaId);
}


//切片视图操作对象框架接口	
CPointD CMediaNode::GetRotateCenter(bool bInput)
{
	QPointF pt = QPointF(m_ptPos.x(), m_ptPos.y() );
	return CPointD(pt.x(), pt.y());
}

double	CMediaNode::GetRotateAngle(bool bInput)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_nRotate;
}

CPointD	CMediaNode::GetRotateControl(bool bInput)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	QPointF pt = QPointF(m_ptPos.x(), m_ptPos.y() - Media_BitMapLocation_Y_Rotate * 1.0);
	QMatrix4x4 temp;
	temp.translate(m_ptPos.x(), m_ptPos.y());
	temp.rotate(m_nRotate, 0, 0, 1);
	temp.translate(-m_ptPos.x(), -m_ptPos.y());

	pt = temp.map(pt);

	return CPointD(pt.x(), pt.y());
}

CPointD	CMediaNode::GetDeleteControl(bool bInput, float fradio)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	QPointF pt(m_ptPos.x() + m_szSize.width() + Media_BitMapLocation_X_Delete * fradio, m_ptPos.y() + Media_BitMapLocation_Y_Delete * fradio);
	QMatrix4x4 temp;
	temp.translate(m_ptPos.x(), m_ptPos.y());
	temp.rotate(m_nRotate, 0, 0, 1);
	temp.translate(-m_ptPos.x(), -m_ptPos.y());

	pt = temp.map(pt);
	return CPointD(pt.x(), pt.y());
}

std::vector<CPointD>	CMediaNode::GetSizePoint(bool bInput)
{
	std::vector<CPointD> arrSizePt;
	std::vector<QPointF>sizePoints;
	sizePoints = GetDrawSizeRect();
	QMatrix4x4 temp;
	temp.translate(m_ptPos.x(), m_ptPos.y());
	temp.rotate(m_nRotate, 0, 0, 1);
	temp.translate(-m_ptPos.x(), -m_ptPos.y());
	for (auto& pt : sizePoints)
		pt = temp.map(pt);

	for (auto i : sizePoints)
	{
		CPointD tmp(i.x(), i.y());
		arrSizePt.push_back(tmp);
	}
	return arrSizePt;
}

bool CMediaNode::IsPtInSide(const CPointD& pt, bool bInput)
{
	std::vector<CPointD> vectemp = GetDrawSizeRectWithRotate();

	if (G_Math_IsPtInPts(vectemp, pt))
		return true;
	else
		return false;
}

std::shared_ptr<IMediaBaseProperty> CMediaNode::GetBaseProperty()
{
	return m_pBaseProperty;
}

std::shared_ptr<IEffectProperty> CMediaNode::GetEffectProperty()
{
	return m_pEffectProperty;
}

std::shared_ptr<IFxProperty> CMediaNode::GetFxProperty()
{
	return m_pFxProperty;
}

void CMediaNode::SetShareProperty(std::shared_ptr<IShareProperty> pShareProperty)
{
	m_pShareProperty = pShareProperty;
}

std::shared_ptr<IShareProperty> CMediaNode::GetShareProperty()
{
	return m_pShareProperty;
}

void CMediaNode::SetUsedChannelMap(bool bUse)
{
	m_bUseChannelMap = bUse;
}

bool CMediaNode::GetUsedChannelMap()
{
	return m_bUseChannelMap;
}

void CMediaNode::SetAudioChannelMap(const SChannelMap& map)
{
	m_channelMap = map;
}

SChannelMap CMediaNode::GetAudioChannelMap()
{
	return m_channelMap;
}
