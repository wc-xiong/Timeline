#include "MediaBaseProperty.h"
#include "common/IResourceInterface.h"
#include "Common/Common/CommonRapidJson.h"
#include "Common/CommonDefine.h"
#include "Common/UICommon/UITranslate.h"
#include "Common/ResourceDefine.h"
#include "Common/MediaCommon.h"

#include "Model/ResourceModel/ResourceModel.h"
#include "Model/ResourceModel/ResourceModelExport.h"
#include <QVector2D>

#include <iostream>
#include <QMatrix4x4>
#include <QDebug>

#include "Transform.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CMediaBaseProperty::CMediaBaseProperty()
{
	m_nRotate = 0;
	
	m_nFillType = EMFT_FullWindow;
	m_nBlendType = EMBT_Cover;

	m_bLocked = false;
	m_bFixed = false;
	m_bHided = false;

	m_bHorFilp = false;
	m_bVerFilp = false;
	

	m_szSize = QSize(80, 60);
	m_OriginalSize = QSize(80, 80);
}

CMediaBaseProperty::~CMediaBaseProperty()
{
	
}

CMediaBaseProperty::CMediaBaseProperty(const CMediaBaseProperty & data)
{
	m_resId = data.m_resId;
	m_mediaId = data.m_mediaId;
	m_nResType = data.m_nResType;

	m_strName = data.m_strName;
	m_bLocked = data.m_bLocked;
	m_bFixed = data.m_bFixed;
	m_bHided = data.m_bHided;
	m_ptPos = data.m_ptPos;
	m_szSize = data.m_szSize;
	m_nRotate = data.m_nRotate;
	m_nFillType = data.m_nFillType;
	m_nBlendType = data.m_nBlendType;
	m_bHorFilp = data.m_bHorFilp;
	m_bVerFilp = data.m_bVerFilp;
	m_OriginalSize = data.m_OriginalSize;
	m_nDecodingMode = data.m_nDecodingMode;
}

CMediaBaseProperty & CMediaBaseProperty::operator=(const CMediaBaseProperty & data)
{
	if (this == &data)
		return *this;

	m_resId = data.m_resId;
	m_mediaId = data.m_mediaId;
	m_nResType = data.m_nResType;

	m_strName = data.m_strName;

	m_bLocked = data.m_bLocked;
	m_bFixed = data.m_bFixed;
	m_bHided = data.m_bHided;
	
	m_ptPos = data.m_ptPos;
	m_szSize = data.m_szSize;
	m_nRotate = data.m_nRotate;
	m_nFillType = data.m_nFillType;
	m_nBlendType = data.m_nBlendType;
	m_bHorFilp = data.m_bHorFilp;
	m_bVerFilp = data.m_bVerFilp;
	
	m_OriginalSize = data.m_OriginalSize;
	m_nDecodingMode = data.m_nDecodingMode;

	return *this;
}

CMediaBaseProperty::CMediaBaseProperty(std::shared_ptr<IMediaBaseProperty> pdata)
{
	CopyProperty(pdata);
}

QRectF CMediaBaseProperty::GetWrapRect()
{
	QRectF rc;
	rc.setLeft(INT_MAX);
	rc.setRight(INT_MIN);
	rc.setTop(INT_MAX);
	rc.setBottom(INT_MIN);

	std::vector<QPointF>vecPoints;
	GetPoints(vecPoints);

	for (auto pt : vecPoints)
	{
		if (rc.left() > pt.x()) rc.setLeft(pt.x());
		if (rc.right() < pt.x()) rc.setRight(pt.x());
		if (rc.top() > pt.y()) rc.setTop(pt.y());
		if (rc.bottom() < pt.y()) rc.setBottom(pt.y());
	}

	return rc;
}

void CMediaBaseProperty::GetPoints(std::vector<QPointF>&vecPoints)
{
	GetPointsWithoutFilp(vecPoints);

	std::lock_guard<std::mutex> lock(m_mutex);

	if (m_bHorFilp)
	{
		auto temp = vecPoints[0];
		vecPoints[0] = vecPoints[3];
		vecPoints[3] = temp;

		temp = vecPoints[1];
		vecPoints[1] = vecPoints[2];
		vecPoints[2] = temp;
	}

	if (m_bVerFilp)
	{
		auto temp = vecPoints[0];
		vecPoints[0] = vecPoints[1];
		vecPoints[1] = temp;

		temp = vecPoints[2];
		vecPoints[2] = vecPoints[3];
		vecPoints[3] = temp;
	}
}

QPointF CMediaBaseProperty::GetFixedRectCenterWithoutRotate(float fradio)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return QPointF(m_ptPos.x() + m_szSize.width() + 20 * fradio, m_ptPos.y() + (12 + 24 + 8) * fradio);
}

QPointF CMediaBaseProperty::GetLockRectCenterWithoutRotate(float fradio)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return QPointF(m_ptPos.x() + m_szSize.width() + 20 * fradio, m_ptPos.y() + 12 * fradio);
}

QPointF CMediaBaseProperty::GetFxRectCenterWithoutRotate(float fradio)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return QPointF(m_ptPos.x() + m_szSize.width() - 20 * fradio, m_ptPos.y() + 20 * fradio);
}

QPointF CMediaBaseProperty::GetRotateRectCenter(float fradio)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	QPointF pt = QPointF(m_ptPos.x() + m_szSize.width() / 2, m_ptPos.y() - 200 * fradio);

	QMatrix4x4 temp;
	temp.translate(m_ptPos.x(), m_ptPos.y());
	temp.rotate(m_nRotate, 0, 0, 1);
	temp.translate(-m_ptPos.x(), -m_ptPos.y());

	pt = temp.map(pt);

	return pt;
}

QPointF CMediaBaseProperty::GetCenter()
{
	std::lock_guard<std::mutex> lock(m_mutex);

	QPointF pt = QPointF(m_ptPos.x() + m_szSize.width() / 2, m_ptPos.y() + m_szSize.height() / 2);

	QMatrix4x4 temp;
	temp.translate(m_ptPos.x(), m_ptPos.y());
	temp.rotate(m_nRotate, 0, 0, 1);
	temp.translate(-m_ptPos.x(), -m_ptPos.y());

	pt = temp.map(pt);

	return pt;
}

void CMediaBaseProperty::GetPointsWithoutFilp(std::vector<QPointF>&vecPoints)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	vecPoints.clear();
	QPointF pt = QPointF(m_ptPos.x(), m_ptPos.y());
	vecPoints.push_back(pt);
	pt = QPointF(m_ptPos.x(), m_ptPos.y() + m_szSize.height());
	vecPoints.push_back(pt);
	pt = QPointF(m_ptPos.x() + m_szSize.width(), m_ptPos.y() + m_szSize.height());
	vecPoints.push_back(pt);
	pt = QPointF(m_ptPos.x() + m_szSize.width(), m_ptPos.y());
	vecPoints.push_back(pt);

	QMatrix4x4 temp;
	temp.translate(m_ptPos.x(), m_ptPos.y());
	temp.rotate(m_nRotate, 0, 0, 1);
	temp.translate(-m_ptPos.x(), -m_ptPos.y());

	for (auto& pt : vecPoints)
		pt = temp.map(pt);
}

void CMediaBaseProperty::Move(float xoffset, float yoffset)
{
	if (GetLocked() || GetFixed()) return;
	std::lock_guard<std::mutex> lock(m_mutex);

	CTransformHelper temp(m_ptPos, m_szSize, m_nRotate);
	temp.Move(xoffset, yoffset);
	m_ptPos = temp.getPos();
	m_szSize = temp.getSize();
	m_nRotate = temp.getAngle();
}

void CMediaBaseProperty::Scale(float xradio, float yradio, float centerx, float centery)
{
	if (GetLocked() || GetFixed()) return;
	std::lock_guard<std::mutex> lock(m_mutex);

	if (xradio < 0) m_bHorFilp = !m_bHorFilp;
	if (yradio < 0) m_bVerFilp = !m_bVerFilp;

	CTransformHelper temp(m_ptPos, m_szSize, m_nRotate);
	temp.Scale(xradio, yradio, centerx, centery);
	m_ptPos = temp.getPos();
	m_szSize = temp.getSize();
	m_nRotate = temp.getAngle();
}

void CMediaBaseProperty::Scale(float xradio, float yradio)
{
	if (GetLocked() || GetFixed()) return;
	std::lock_guard<std::mutex> lock(m_mutex);

	if (xradio < 0) m_bHorFilp = !m_bHorFilp;
	if (yradio < 0) m_bVerFilp = !m_bVerFilp;

	CTransformHelper temp(m_ptPos, m_szSize, m_nRotate);
	temp.Scale(xradio, yradio);
	m_ptPos = temp.getPos();
	m_szSize = temp.getSize();
	m_nRotate = temp.getAngle();
}

void CMediaBaseProperty::Rotate(float angleoffset, float centerx, float centery)
{
	if (GetLocked()) return;
	std::lock_guard<std::mutex> lock(m_mutex);

	CTransformHelper temp(m_ptPos, m_szSize, m_nRotate);
	temp.Rotate(angleoffset, centerx, centery);
	m_ptPos = temp.getPos();
	m_szSize = temp.getSize();
	m_nRotate = temp.getAngle();
}

void CMediaBaseProperty::Rotate(float angleoffset)
{
	if (GetLocked()) return;
	std::lock_guard<std::mutex> lock(m_mutex);

	CTransformHelper temp(m_ptPos, m_szSize, m_nRotate);
	temp.Rotate(angleoffset);
	m_ptPos = temp.getPos();
	m_szSize = temp.getSize();
	m_nRotate = temp.getAngle();
}

void CMediaBaseProperty::SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator)
{
	root.SetObject();
	root.AddMember("LockState", m_bLocked, allocator);
	root.AddMember("FixedState", m_bFixed, allocator);
	root.AddMember("HideState", m_bHided, allocator);
	root.AddMember("Pos_X", m_ptPos.rx() + FLT_EPSILON, allocator);
	root.AddMember("Pos_Y", m_ptPos.ry() + FLT_EPSILON, allocator);
	root.AddMember("Width", m_szSize.width() + FLT_EPSILON, allocator);
	root.AddMember("Height", m_szSize.height() + FLT_EPSILON, allocator);
	root.AddMember("OriginalWidth", m_OriginalSize.width(), allocator);
	root.AddMember("OriginalHeight", m_OriginalSize.height(), allocator);
	root.AddMember("Rotate", m_nRotate, allocator);
	root.AddMember("KeepAspectRatio", m_bKeepAspectRatio, allocator);
	root.AddMember("FillType", m_nFillType, allocator);
	root.AddMember("BlendType", m_nBlendType, allocator);
	root.AddMember("HorFilp", m_bHorFilp, allocator);
	root.AddMember("VerFilp", m_bVerFilp, allocator);
	root.AddMember("DecodingMode", m_nDecodingMode, allocator);
}

void CMediaBaseProperty::LoadDataFromRapidJson(const rapidjson::Value &root)
{
	if (!root.IsObject())
		return;
	m_bLocked = root.HasMember("LockState") ? root["LockState"].GetBool() : m_bLocked;
	m_bFixed = root.HasMember("FixedState") ? root["FixedState"].GetBool() : m_bFixed;
	m_bHided = root.HasMember("HideState") ? root["HideState"].GetBool() : m_bHided;

	float nPos_X = static_cast<float>(RapidJsonGetDoubleOrDefault(root, "Pos_X", 0.0));
	float nPos_Y = static_cast<float>(RapidJsonGetDoubleOrDefault(root, "Pos_Y", 0.0));
	m_ptPos = QPointF(nPos_X, nPos_Y);

	float nWidth = static_cast<float>(RapidJsonGetDoubleOrDefault(root, "Width", 0.0));
	float nHeight = static_cast<float>(RapidJsonGetDoubleOrDefault(root, "Height", 0.0));
	m_szSize = QSizeF(nWidth, nHeight);

	int nOriginalWidth = root.HasMember("OriginalWidth") ? root["OriginalWidth"].GetInt() : 0;
	int nOriginalHeight = root.HasMember("OriginalHeight") ? root["OriginalHeight"].GetInt() : 0;
	m_OriginalSize = QSize(nOriginalWidth, nOriginalHeight);

	m_nRotate = static_cast<float>(RapidJsonGetDoubleOrDefault(root, "Rotate", m_nRotate));
	m_bKeepAspectRatio = root.HasMember("KeepAspectRatio") ? root["KeepAspectRatio"].GetBool() : m_bKeepAspectRatio;
	m_nFillType = root.HasMember("FillType") ? root["FillType"].GetInt() : m_nFillType;
	m_nBlendType = root.HasMember("BlendType") ? root["BlendType"].GetInt() : m_nBlendType;
	m_bHorFilp = root.HasMember("HorFilp") ? root["HorFilp"].GetBool() : m_bHorFilp;
	m_bVerFilp = root.HasMember("VerFilp") ? root["VerFilp"].GetBool() : m_bVerFilp;
	m_nDecodingMode = root.HasMember("DecodingMode") ? root["DecodingMode"].GetInt() : ED_Auto;
}

QRectF CMediaBaseProperty::GetSizeRect()
{
	return QRectF(m_ptPos, m_szSize);
}

void CMediaBaseProperty::Replace()
{
	m_vecResInfo.clear();
}

void CMediaBaseProperty::CopyProperty(std::shared_ptr<IMediaBaseProperty> pdata)
{
	m_resId = pdata->GetResId();
	m_mediaId = pdata->GetMediaId();
	m_nResType = pdata->GetResType();

	m_strName = pdata->GetName();

	m_bLocked = pdata->GetLocked();
	m_bFixed = pdata->GetFixed();
	m_bHided = pdata->GetHided();

	m_ptPos = pdata->GetPos();
	m_szSize = pdata->GetSize();
	m_nRotate = pdata->GetRotate();
	m_bKeepAspectRatio = pdata->GetKeepAspectRatio();
	m_sizeKeepAspectRatio = pdata->GetSizeKeepAspectRatio();
	m_nFillType = pdata->GetFillType();
	m_nBlendType = pdata->GetBlendType();
	m_bHorFilp = pdata->GetHorizontal();
	m_bVerFilp = pdata->GetVertical();

	m_OriginalSize = pdata->GetOriginalSize();
	m_nDecodingMode = pdata->GetDecodingMode();

}

void CMediaBaseProperty::SetPos(QPoint ptPos)
{
	m_bDirty = true;
	auto temp = ptPos - m_ptPos;
	Move(temp.x(), temp.y());
}

void CMediaBaseProperty::SetRotate(float nRotate)
{
	m_bDirty = true;
	//std::unique_lock<std::mutex> lock(m_mutex);
	auto temp = nRotate - m_nRotate;
	//lock.unlock();

	Rotate(temp);
}

void CMediaBaseProperty::SetSize(QSize szSize)
{
	m_bDirty = true;
	auto tempx = (float)szSize.width() / (float)m_szSize.width();
	auto tempy = (float)szSize.height() / (float)m_szSize.height();
	Scale(tempx, tempy);
}

void CMediaBaseProperty::SetWidth(int iValue)
{
	m_bDirty = true;
	auto tempx = (float)iValue / (float)m_szSize.width();
	Scale(tempx, 1);
}

void CMediaBaseProperty::SetHeight(int iValue)
{
	m_bDirty = true;
	auto tempy = (float)iValue / (float)m_szSize.height();
	Scale(1, tempy);
}

QPixmap CMediaBaseProperty::GetThumbnail()
{
	if (!m_pixRes.isNull())
		return m_pixRes;

	std::shared_ptr<IResourceBase> resNode = g_pResourceModel->GetResourceNode(m_resId);
	if (resNode)
	{
		m_pixRes = resNode->GetThumbnail(false, false);
		return m_pixRes;
	}

	return QPixmap();
}

std::vector<QString> CMediaBaseProperty::GetInfo()
{
	if (!m_vecResInfo.empty())
		return m_vecResInfo;
	
	std::shared_ptr<IResourceBase> resNode = g_pResourceModel->GetResourceNode(m_resId);
	if (!resNode)
		return m_vecResInfo;

	if (resNode->GetType() == ERT_Video)
	{
		//str.append(CCommonTranslate::Name() + ":" + resNode.GetName() + "\n");

		QString str11 = CCommonTranslate::Resolution() + QString(": %1x%2").arg(resNode->GetResolution().width()).arg(resNode->GetResolution().height());
		QString str12 = CCommonTranslate::VideoFrameRate() + QString(": %3").arg(QString::number(resNode->GetVideoFrameRate(), 'f', 2));
		QString str13 = resNode->GetbHWDEC() ? CCommonTranslate::VideoHardwareDecode() : CCommonTranslate::VideoSoftwareDecode();
		m_vecResInfo.push_back(str11);
		m_vecResInfo.push_back(str12);
		m_vecResInfo.push_back(str13);

		QString str21 = CCommonTranslate::VideoFormat() + ": " + resNode->GetVideoFormat();
		QString str22 = CCommonTranslate::VideoBitRate() + QString(": %3Kbps").arg(QString::number(resNode->GetVideoBitRate() / 1000.0, 'f', 2));
		m_vecResInfo.push_back(str21);
		m_vecResInfo.push_back(str22);
		m_vecResInfo.push_back(QString());

		if (!resNode->GetAudioFormat().isNull())
		{
			QString str31 = CCommonTranslate::AudioFormat() + ": " + resNode->GetAudioFormat();
			QString str32 = CCommonTranslate::AudioSampleRate() + QString(": %3KHz").arg(QString::number(resNode->GetAudioSampleRate() / 1000.0, 'f', 2));
			QString str33 = CCommonTranslate::AudioChannels() + QString(": %4").arg(resNode->GetAudioChannels());
			m_vecResInfo.push_back(str31);
			m_vecResInfo.push_back(str32);
			m_vecResInfo.push_back(str33);
		}
	}
	else if (resNode->GetType() == ERT_Audio)
	{
		//str.append(CCommonTranslate::Name() + ":" + resNode.GetName() + "\n");

		QString str31 = CCommonTranslate::AudioFormat() + ": " + resNode->GetAudioFormat();
		QString str32 = CCommonTranslate::AudioSampleRate() + QString(": %3KHz").arg(resNode->GetAudioSampleRate() / 1000.0);
		QString str33 = CCommonTranslate::AudioChannels() + QString(": %4").arg(resNode->GetAudioChannels());
		m_vecResInfo.push_back(str31);
		m_vecResInfo.push_back(str32);
		m_vecResInfo.push_back(str33);
	}
	return m_vecResInfo;
}

QSize CMediaBaseProperty::GetOriginalSize()
{
	return m_OriginalSize;
}

bool CMediaBaseProperty::IsHWDEC()
{
	std::shared_ptr<IResourceBase> resNode = g_pResourceModel->GetResourceNode(m_resId);
	if (resNode)
		return resNode->GetbHWDEC();

	return false;
}
