#include "ResourceBase.h"
#include "Common/Common/ResourceDefine.h"

#include "Common/CommonDefine.h"

#include "UICommon/UICommonApi.h"
#include "UICommon/UICommonSizeDefine.h"
#include "Model/GlobalModel/GlobalModelExport.h"
#include "Model/GlobalModel/SoftwareProject.h"
#include "Model/GlobalModel/DisplayerDataMgr.h"
#include "ResourceModelMaterialPath.h"

#include <QBitmap>
#include <QPainter>
#include <QCollator>
//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end::CResourceBase()

CResourceBase::CResourceBase()
{
	m_resId = QUuid::createUuid();
	m_nResType = ERT_Invalid;
	m_bDemuxered = true;
	m_bDemuxerSuccessed = true;
}

CResourceBase::CResourceBase(const CResourceBase& other)
{
	*this = other;
}

CResourceBase& CResourceBase::operator=(const CResourceBase& other)
{
	if (this == &other)
		return *this;

	m_resId = other.m_resId;
	m_nResType = other.m_nResType;
	m_szResolution = other.m_szResolution;
	m_nDuration = other.m_nDuration;
	m_Thumbnail = other.m_Thumbnail;
	m_strName = other.m_strName;
	m_strPath = other.m_strPath;
	m_nUsedCount = other.m_nUsedCount;
	m_nFilesize = other.m_nFilesize;
	m_nAudioChannels = other.m_nAudioChannels;
	m_bDemuxered = other.m_bDemuxered;
	m_bDemuxerSuccessed = other.m_bDemuxerSuccessed;

	m_strFormat = other.m_strFormat;
	m_videoFramerate = other.m_videoFramerate;
	m_vidioFormat = other.m_vidioFormat;
	m_videoBitrate = other.m_videoBitrate;
	m_bSupportHWDEC = other.m_bSupportHWDEC;
	m_bHWDEC = other.m_bHWDEC;
	m_nDecodingMode = other.m_nDecodingMode;
	m_audioFormat = other.m_audioFormat;
	m_audioSamplerate = other.m_audioSamplerate;
	m_strFullName = other.m_strFullName;
	m_bIsKeepOpen = other.m_bIsKeepOpen;

	return *this;
}

CResourceBase::~CResourceBase()
{
}

void CResourceBase::SetId(const QUuid &Guid)
{
	m_resId = Guid;
}

QUuid CResourceBase::GetId() const
{
	return m_resId;
}

void CResourceBase::SetName(const QString &strName)
{
	m_strName = strName;
}

QString CResourceBase::GetName() const
{
	return m_strName;
}

void CResourceBase::SetFullName(const QString &strName)
{
	m_strFullName = strName;
}

QString CResourceBase::GetFullName() const
{
	return m_strFullName;
}

void CResourceBase::SetType(int eType)
{
	m_nResType = eType;
}

int CResourceBase::GetType()
{
	return m_nResType;
}

void CResourceBase::SetResolution(QSize sz)
{
	m_szResolution = sz;
}

QSize CResourceBase::GetResolution()
{
	return m_szResolution;
}

void CResourceBase::SetPath(const QString &strPath)
{
	m_strPath = strPath;
}

QString CResourceBase::GetPath() const
{
	return m_strPath;
}

void CResourceBase::SetThumbnail(const QPixmap &pImage)
{
	m_Thumbnail = pImage;
}

QPixmap CResourceBase::GetThumbnail() const
{
	return m_Thumbnail;
}

QPixmap CResourceBase::GetThumbnail(bool bUIImage, bool bNeedUsedSigned) const
{
	if (m_Thumbnail.isNull())
		return QPixmap();

	// 缩小 QPixmap 的内容，留出一个像素的边框空间
	QPixmap pix = bUIImage ? m_Thumbnail.scaled(Res_ThumbnailUIShowWidth /*- nSizeDefine_2*/, Res_ThumbnailUIShowHeight /*- nSizeDefine_2*/) : m_Thumbnail;

	if (!bUIImage && !bNeedUsedSigned)
		return pix;

	//圆角pixmap，对内部pixmap进行圆角处理
	if (bUIImage)
	{
		QPainter painterPix(&pix);
		QPainterPath path;
		int nRadius = nSizeDefine_2; // 圆角半径
		path.addRoundedRect(pix.rect(), nRadius, nRadius);
		painterPix.setClipping(true);
		painterPix.setClipPath(path);
		painterPix.drawPixmap(0, 0, pix);
	}

	// 创建一个稍大的 QPixmap 以容纳缩小后的图像和边框
	QPixmap result(Res_ThumbnailUIShowWidth, Res_ThumbnailUIShowHeight);
	result.fill(Qt::transparent);

	QPainter painter(&result);
	// 创建一个圆角矩形路径
	QPainterPath path;
	QRect rc = result.rect().adjusted(nSizeDefine_1, nSizeDefine_1, -nSizeDefine_1, -nSizeDefine_1); // 为留出边框调整矩形位置和大小
	int nRadius = nSizeDefine_2; // 圆角半径
	path.addRoundedRect(rc, nRadius, nRadius);

	// 设置剪切路径，这样绘制时只会在圆角矩形内绘制
	painter.setClipPath(path);

	// 将缩小后的 pix 绘制到圆角矩形内部
	painter.drawPixmap(nSizeDefine_1, nSizeDefine_1, pix);

	// 移除剪切路径以绘制边框
	painter.setClipping(false);

	// 绘制边框
	if (bUIImage)
	{
		painter.setPen(QPen(QColor(0, 0, 0)));
		QRect rc = result.rect();
		painter.drawRoundedRect(rc.adjusted(0, 0, -nSizeDefine_1, -nSizeDefine_1), nRadius, nRadius);
	}

	// 如果需要绘制 "已使用" 图标
	if (bNeedUsedSigned && m_nResType != ERT_Folder && m_nUsedCount > 0)
	{
		QRect rc = result.rect();
		rc.setTop(rc.top() + nSizeDefine_1);
		rc.setLeft(rc.left() + nSizeDefine_1);
		rc.setRight(rc.left() + nSizeDefine_13);
		rc.setBottom(rc.top() + nSizeDefine_13);
		painter.drawPixmap(rc, QPixmap(":/dark/images/ResourceWnd/ResourceUsed.png"));
	}

	return result;
}

void CResourceBase::SetFilesize(long long nFileSize)
{
	m_nFilesize = nFileSize;
}

long long CResourceBase::GetFilesize() const
{
	return m_nFilesize;
}

void CResourceBase::SetAudioChannels(int nAudioChannels)
{
	m_nAudioChannels = nAudioChannels;
}

int CResourceBase::GetAudioChannels() const
{
	return m_nAudioChannels;
}


void CResourceBase::SetResourceFormat(const QString &strResourceFormat)
{
	m_strFormat = strResourceFormat;
}

QString CResourceBase::GetResourceFormat() const
{
	return m_strFormat;
}

void CResourceBase::SetDuration(__int64 nLenght)
{
	m_nDuration = nLenght;
}
__int64 CResourceBase::GetDuration() const
{
	return m_nDuration;
}

void CResourceBase::UpdateResourceUseCount(int nCount)
{
	m_nUsedCount += nCount;
}

void CResourceBase::SetResourceUseCount(int nUseCount)
{
	if (nUseCount >= 0)
		m_nUsedCount = nUseCount;
}

int CResourceBase::GetResourceUseCount()
{
	return m_nUsedCount;
}

void CResourceBase::SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator)
{
	root.SetObject();
	root.AddMember(rapidjson::Value(Xml_ResourceModel_ResourceType, allocator), static_cast<int>(m_nResType), allocator);
	root.AddMember(rapidjson::Value(Xml_ResourceModel_ResourceID, allocator), rapidjson::Value(m_resId.toString().toUtf8().constData(), allocator), allocator);
	root.AddMember(rapidjson::Value(Xml_ResourceModel_ResourceName, allocator), rapidjson::Value(m_strName.toUtf8().constData(), allocator), allocator);
	root.AddMember(rapidjson::Value(Xml_ResourceModel_ResourceFullName, allocator), rapidjson::Value(m_strFullName.toUtf8().constData(), allocator), allocator);
	root.AddMember(rapidjson::Value(Xml_ResourceModel_ResourceUsedCount, allocator), m_nUsedCount, allocator);
	root.AddMember(rapidjson::Value(Xml_ResourceModel_ResourcePath, allocator), rapidjson::Value(m_strPath.toUtf8().constData(), allocator), allocator);
	root.AddMember(rapidjson::Value(Xml_ResourceModel_ResourceFormat, allocator), rapidjson::Value(m_strFormat.toUtf8().constData(), allocator), allocator);
	root.AddMember(rapidjson::Value(Xml_ResourceModel_Duration, allocator), static_cast<int64_t>(m_nDuration), allocator);
	root.AddMember(rapidjson::Value(Xml_ResourceModel_Width, allocator), m_szResolution.width(), allocator);
	root.AddMember(rapidjson::Value(Xml_ResourceModel_Height, allocator), m_szResolution.height(), allocator);
	root.AddMember(rapidjson::Value(Xml_ResourceModel_SupportHWDEC, allocator), m_bSupportHWDEC, allocator);
	root.AddMember(rapidjson::Value(Xml_ResourceModel_HWDEC, allocator), m_bHWDEC, allocator);
	root.AddMember(rapidjson::Value(Xml_ResourceModel_DecodingMode, allocator), m_nDecodingMode, allocator);
	root.AddMember(rapidjson::Value(Xml_ResourceModel_VideoFramerate, allocator), m_videoFramerate + FLT_EPSILON, allocator);
	root.AddMember(rapidjson::Value(Xml_ResourceModel_VidioFormat, allocator), rapidjson::Value(m_vidioFormat.toUtf8().constData(), allocator), allocator);
	root.AddMember(rapidjson::Value(Xml_ResourceModel_VideoBitrate, allocator), static_cast<int64_t>(m_videoBitrate), allocator);
	root.AddMember(rapidjson::Value(Xml_ResourceModel_AudioChannels, allocator), m_nAudioChannels, allocator);
	root.AddMember(rapidjson::Value(Xml_ResourceModel_AudioFormat, allocator), rapidjson::Value(m_audioFormat.toUtf8().constData(), allocator), allocator);
	root.AddMember(rapidjson::Value(Xml_ResourceModel_AudioSamplerate, allocator), static_cast<int64_t>(m_audioSamplerate), allocator);
	root.AddMember(rapidjson::Value(Xml_ResourceModel_AlwaysOpen, allocator), m_bIsKeepOpen, allocator);
}

bool CResourceBase::LoadDataFromRapidJson(const rapidjson::Value& root)
{
	if (!root.IsObject())
		return false;
	const auto has = [&root](const char* k) { return root.HasMember(k); };
	const auto getS = [&root](const char* k) { return QString::fromUtf8(root[k].GetString()); };

	m_resId = has(Xml_ResourceModel_ResourceID) ? getS(Xml_ResourceModel_ResourceID) : QString();
	m_nResType = has(Xml_ResourceModel_ResourceType) ? (EResourceType)root[Xml_ResourceModel_ResourceType].GetInt() : ERT_Invalid;
	m_strName = has(Xml_ResourceModel_ResourceName) ? getS(Xml_ResourceModel_ResourceName) : QString();
	m_strFullName = has(Xml_ResourceModel_ResourceFullName) ? getS(Xml_ResourceModel_ResourceFullName) : QString();
	m_nUsedCount = has(Xml_ResourceModel_ResourceUsedCount) ? root[Xml_ResourceModel_ResourceUsedCount].GetInt() : 0;
	m_strPath = has(Xml_ResourceModel_ResourcePath) ? getS(Xml_ResourceModel_ResourcePath) : QString();
	m_strFormat = has(Xml_ResourceModel_ResourceFormat) ? getS(Xml_ResourceModel_ResourceFormat) : QString();
	

	if (g_pSoftwareProject->GetProjectType() == ESPT_Pack && IsFileResource(m_nResType))
	{
		QString strPrjPath = g_pSoftwareProject->GetProjectPath();
		m_strPath = strPrjPath.left(strPrjPath.lastIndexOf('/')) + (has(Xml_ResourceModel_ResourcePath) ? getS(Xml_ResourceModel_ResourcePath) : QString());
	}
	else
		m_strPath = has(Xml_ResourceModel_ResourcePath) ? getS(Xml_ResourceModel_ResourcePath) : QString();

	// 仅远程播放端将路径映射到素材缓存；控制端与本机播放端保持原始路径
	if (g_pDisplayDataMgr->IsRemoteDisplayer() && IsNeedsFileTransfer(m_nResType))
	{
		const QString cachePath = MakeMaterialCacheAbsolutePath(m_strPath);
		if (!cachePath.isEmpty())
		{
			m_strPath = cachePath;
		}
	}

	m_nDuration = has(Xml_ResourceModel_Duration) ? root[Xml_ResourceModel_Duration].GetInt64() : 0;

	int nWidth = has(Xml_ResourceModel_Width) ? root[Xml_ResourceModel_Width].GetInt() : 0;
	int nHeight = has(Xml_ResourceModel_Height) ? root[Xml_ResourceModel_Height].GetInt() : 0;
	m_szResolution = QSize(nWidth, nHeight);

	m_bSupportHWDEC = has(Xml_ResourceModel_SupportHWDEC) ? root[Xml_ResourceModel_SupportHWDEC].GetBool() : false;
	m_bHWDEC = has(Xml_ResourceModel_HWDEC) ? root[Xml_ResourceModel_HWDEC].GetBool() : false;
	m_nDecodingMode = has(Xml_ResourceModel_DecodingMode) ? root[Xml_ResourceModel_DecodingMode].GetInt() : ED_Auto;
	m_videoFramerate = has(Xml_ResourceModel_VideoFramerate) ? root[Xml_ResourceModel_VideoFramerate].GetDouble() : 0.0;
	m_vidioFormat = has(Xml_ResourceModel_VidioFormat) ? getS(Xml_ResourceModel_VidioFormat) : QString();
	m_videoBitrate = has(Xml_ResourceModel_VideoBitrate) ? root[Xml_ResourceModel_VideoBitrate].GetInt64() : 0;
	m_nAudioChannels = has(Xml_ResourceModel_AudioChannels) ? root[Xml_ResourceModel_AudioChannels].GetInt() : 0;
	m_audioFormat = has(Xml_ResourceModel_AudioFormat) ? getS(Xml_ResourceModel_AudioFormat) : QString();
	m_audioSamplerate = has(Xml_ResourceModel_AudioSamplerate) ? root[Xml_ResourceModel_AudioSamplerate].GetInt64() : 0;
	m_bIsKeepOpen = has(Xml_ResourceModel_AlwaysOpen) ? root[Xml_ResourceModel_AlwaysOpen].GetBool() : false;

	return true;
}

bool CResourceBase::IsSupportFx()
{
	switch (m_nResType)
	{
	case ERT_Video:
	case ERT_Picture:
	case ERT_VideoCapture:
	case ERT_WebMedia:
	case ERT_Ndi:
		return true;
	default:
		return false;
	}

	return false;
}

void CResourceBase::SetResDemuxered(bool bValue)
{
	m_bDemuxered = bValue;
}

bool CResourceBase::GetResDemuxered()
{
	return m_bDemuxered;
}

//是否支持硬解码
void CResourceBase::SetSupportHWDEC(bool bSupportHWDEC)
{
	m_bSupportHWDEC = bSupportHWDEC;
}

bool CResourceBase::GetbSupportHWDEC() const
{
	return m_bSupportHWDEC;
}

void CResourceBase::SetHWDEC(bool bHWDEC)
{
	m_bHWDEC = bHWDEC;
}

bool CResourceBase::GetbHWDEC() const
{
	return m_bHWDEC;
}

void CResourceBase::SetDecodingMode(int mode)
{
	m_nDecodingMode = mode;
}

int CResourceBase::GetDecodingMode() const
{
	return m_nDecodingMode;
}

// 设置视频帧率
void CResourceBase::SetVideoFrameRate(double nVeidoFrameRate)
{
	m_videoFramerate = nVeidoFrameRate;
}
double CResourceBase::GetVideoFrameRate() const
{
	return m_videoFramerate;
}

// 设置视频格式
void CResourceBase::SetVideoFormat(QString strVideoFormat)
{
	m_vidioFormat = strVideoFormat;
}
QString CResourceBase::GetVideoFormat() const
{
	return m_vidioFormat;
}

// 设置视频码率
void CResourceBase::SetVideoBitRate(__int64 nVeidoBitRate)
{
	m_videoBitrate = nVeidoBitRate;
}
__int64 CResourceBase::GetVideoBitRate() const
{
	return m_videoBitrate;
}

// 设置音频格式
void CResourceBase::SetAudioFormat(QString strAudioFormat)
{
	m_audioFormat = strAudioFormat;
}
QString CResourceBase::GetAudioFormat() const
{
	return m_audioFormat;
}

// 设置音频采样率
void CResourceBase::SetAudioSampleRate(__int64 nAudioSampleRate)
{
	m_audioSamplerate = nAudioSampleRate;
}
__int64 CResourceBase::GetAudioSampleRate() const
{
	return m_audioSamplerate;
}