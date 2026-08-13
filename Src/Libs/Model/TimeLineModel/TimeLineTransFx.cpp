#include "TimeLineTransFx.h"
#include "Common/Common/TransationTypeDefine.h"
#include "ConvertHelper.h"

CTimeLineTransFx::CTimeLineTransFx()
	: m_id(QUuid::createUuid())
	, m_transType(ETFT_Fade)
{
}

CTimeLineTransFx::CTimeLineTransFx(const QUuid& id)
	: m_id(id)
	, m_transType(ETFT_Fade)
{
}

CTimeLineTransFx::~CTimeLineTransFx()
{
}

CTimeLineTransFx::CTimeLineTransFx(const CTimeLineTransFx& other)
{
	m_id = other.m_id;
	m_layerId = other.m_layerId;
	m_leftObjectId = other.m_leftObjectId;
	m_rightObjectId = other.m_rightObjectId;
	m_duration = other.m_duration;
	m_transType = other.m_transType;
}

CTimeLineTransFx& CTimeLineTransFx::operator=(const CTimeLineTransFx& other)
{
	if (this == &other)
		return *this;

	std::unique_lock<std::shared_mutex> lock(m_mtx);
	m_id = other.m_id;
	m_layerId = other.m_layerId;
	m_leftObjectId = other.m_leftObjectId;
	m_rightObjectId = other.m_rightObjectId;
	m_duration = other.m_duration;
	m_transType = other.m_transType;
	return *this;
}

QUuid CTimeLineTransFx::GetId()
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	return m_id;
}

void CTimeLineTransFx::SetId(const QUuid& id)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	m_id = id;
}

QUuid CTimeLineTransFx::GetLayerId()
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	return m_layerId;
}

void CTimeLineTransFx::SetLayerId(const QUuid& id)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	m_layerId = id;
}

QUuid CTimeLineTransFx::GetLeftObjectId()
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	return m_leftObjectId;
}

void CTimeLineTransFx::SetLeftObjectId(const QUuid& id)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	m_leftObjectId = id;
}

QUuid CTimeLineTransFx::GetRightObjectId()
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	return m_rightObjectId;
}

void CTimeLineTransFx::SetRightObjectId(const QUuid& id)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	m_rightObjectId = id;
}

std::chrono::milliseconds CTimeLineTransFx::GetDuration()
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	return m_duration;
}

void CTimeLineTransFx::SetDuration(const std::chrono::milliseconds& dur)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	// 转场时长帧对齐：Floor；正时长至少1帧
	auto aligned = g_pConvertHelper->formatTimeWithFrame(dur, RoundingMode::Floor);
	if (dur.count() > 0 && aligned.count() == 0)
		aligned = g_pConvertHelper->frameToTimes(1);
	m_duration = aligned;
}

int CTimeLineTransFx::GetTransType()
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	return m_transType;
}

void CTimeLineTransFx::SetTransType(int type)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	m_transType = type;
}

#define TimeLineTransFx_Id_Json "TimeLineTransFx_Id"
#define TimeLineTransFx_LayerId_Json "TimeLineTransFx_LayerId"
#define TimeLineTransFx_LeftObjectId_Json "TimeLineTransFx_LeftObjectId"
#define TimeLineTransFx_RightObjectId_Json "TimeLineTransFx_RightObjectId"
#define TimeLineTransFx_Duration_Json "TimeLineTransFx_Duration"
#define TimeLineTransFx_TransType_Json "TimeLineTransFx_TransType"

bool CTimeLineTransFx::LoadDataFromRapidJson(const rapidjson::Value& root)
{
	if (!root.IsObject())
		return false;

	if (root.HasMember(TimeLineTransFx_Id_Json) && root[TimeLineTransFx_Id_Json].IsString())
		m_id = QUuid::fromString(QString::fromUtf8(root[TimeLineTransFx_Id_Json].GetString()));

	if (root.HasMember(TimeLineTransFx_LayerId_Json) && root[TimeLineTransFx_LayerId_Json].IsString())
		m_layerId = QUuid::fromString(QString::fromUtf8(root[TimeLineTransFx_LayerId_Json].GetString()));

	if (root.HasMember(TimeLineTransFx_LeftObjectId_Json) && root[TimeLineTransFx_LeftObjectId_Json].IsString())
		m_leftObjectId = QUuid::fromString(QString::fromUtf8(root[TimeLineTransFx_LeftObjectId_Json].GetString()));

	if (root.HasMember(TimeLineTransFx_RightObjectId_Json) && root[TimeLineTransFx_RightObjectId_Json].IsString())
		m_rightObjectId = QUuid::fromString(QString::fromUtf8(root[TimeLineTransFx_RightObjectId_Json].GetString()));

	if (root.HasMember(TimeLineTransFx_Duration_Json) && root[TimeLineTransFx_Duration_Json].IsInt64())
	{
		auto dur = std::chrono::milliseconds(root[TimeLineTransFx_Duration_Json].GetInt64());
		auto aligned = g_pConvertHelper->formatTimeWithFrame(dur, RoundingMode::Floor);
		if (dur.count() > 0 && aligned.count() == 0)
			aligned = g_pConvertHelper->frameToTimes(1);
		m_duration = aligned;
	}

	if (root.HasMember(TimeLineTransFx_TransType_Json) && root[TimeLineTransFx_TransType_Json].IsInt())
		m_transType = root[TimeLineTransFx_TransType_Json].GetInt();

	return true;
}

void CTimeLineTransFx::SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator)
{
	root.SetObject();
	root.AddMember(rapidjson::StringRef(TimeLineTransFx_Id_Json),
		rapidjson::Value(m_id.toString().toUtf8().constData(), allocator), allocator);
	root.AddMember(rapidjson::StringRef(TimeLineTransFx_LayerId_Json),
		rapidjson::Value(m_layerId.toString().toUtf8().constData(), allocator), allocator);
	root.AddMember(rapidjson::StringRef(TimeLineTransFx_LeftObjectId_Json),
		rapidjson::Value(m_leftObjectId.toString().toUtf8().constData(), allocator), allocator);
	root.AddMember(rapidjson::StringRef(TimeLineTransFx_RightObjectId_Json),
		rapidjson::Value(m_rightObjectId.toString().toUtf8().constData(), allocator), allocator);
	root.AddMember(rapidjson::StringRef(TimeLineTransFx_Duration_Json),
		static_cast<int64_t>(m_duration.count()), allocator);
	root.AddMember(rapidjson::StringRef(TimeLineTransFx_TransType_Json), m_transType, allocator);
}