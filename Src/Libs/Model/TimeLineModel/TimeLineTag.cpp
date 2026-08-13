#include "TimeLineTag.h"
#include "ConvertHelper.h"

CTimeLineTag::CTimeLineTag()
	: m_id(QUuid::createUuid())
{
}

CTimeLineTag::CTimeLineTag(const QUuid& id)
	: m_id(id)
{
}

CTimeLineTag::~CTimeLineTag()
{
}

CTimeLineTag::CTimeLineTag(const CTimeLineTag& other)
{
	m_id = other.m_id;
	m_tagName = other.m_tagName;
	m_tagPosition = other.m_tagPosition;
	m_tagTCPosition = other.m_tagTCPosition;
}

CTimeLineTag& CTimeLineTag::operator=(const CTimeLineTag& other)
{
	if (this == &other)
		return *this;

	std::unique_lock<std::shared_mutex> lock(m_mtx);
	m_id = other.m_id;
	m_tagName = other.m_tagName;
	m_tagPosition = other.m_tagPosition;
	m_tagTCPosition = other.m_tagTCPosition;
	return *this;
}

QUuid CTimeLineTag::GetId()
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	return m_id;
}

void CTimeLineTag::SetId(const QUuid& id)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	m_id = id;
}

QString CTimeLineTag::GetTagName()
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	return m_tagName;
}

void CTimeLineTag::SetTagName(const QString& name)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	m_tagName = name;
}

std::chrono::milliseconds CTimeLineTag::GetTagPosition()
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	return m_tagPosition;
}

void CTimeLineTag::SetTagPosition(const std::chrono::milliseconds& time)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	// Tag位置帧对齐：Floor
	m_tagPosition = g_pConvertHelper->formatTimeWithFrame(time, RoundingMode::Floor);
}

std::chrono::milliseconds CTimeLineTag::GetTagTCPosition()
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	return m_tagTCPosition;
}

void CTimeLineTag::SetTagTCPosition(const std::chrono::milliseconds& time)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	// Tag TC位置帧对齐：Floor
	m_tagTCPosition = g_pConvertHelper->formatTimeWithFrame(time, RoundingMode::Floor);
}

#define TimeLineTag_Id_Json "TimeLineTag_Id"
#define TimeLineTag_Name_Json "TimeLineTag_Name"
#define TimeLineTag_Position_Json "TimeLineTag_Position"
#define TimeLineTag_TCPosition_Json "TimeLineTag_TCPosition"

bool CTimeLineTag::LoadDataFromRapidJson(const rapidjson::Value& root)
{
	if (!root.IsObject())
		return false;

	if (root.HasMember(TimeLineTag_Id_Json) && root[TimeLineTag_Id_Json].IsString())
	{
		m_id = QUuid::fromString(QString::fromUtf8(root[TimeLineTag_Id_Json].GetString()));
	}

	if (root.HasMember(TimeLineTag_Name_Json) && root[TimeLineTag_Name_Json].IsString())
	{
		m_tagName = QString::fromUtf8(root[TimeLineTag_Name_Json].GetString());
	}

	if (root.HasMember(TimeLineTag_Position_Json) && root[TimeLineTag_Position_Json].IsInt64())
	{
		m_tagPosition = g_pConvertHelper->formatTimeWithFrame(
			std::chrono::milliseconds(root[TimeLineTag_Position_Json].GetInt64()), RoundingMode::Floor);
	}

	if (root.HasMember(TimeLineTag_TCPosition_Json) && root[TimeLineTag_TCPosition_Json].IsInt64())
	{
		m_tagTCPosition = g_pConvertHelper->formatTimeWithFrame(
			std::chrono::milliseconds(root[TimeLineTag_TCPosition_Json].GetInt64()), RoundingMode::Floor);
	}

	return true;
}

void CTimeLineTag::SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator)
{
	root.SetObject();

	QByteArray nameBytes = m_tagName.toUtf8();
	root.AddMember(rapidjson::StringRef(TimeLineTag_Name_Json), rapidjson::Value(nameBytes.constData(), allocator), allocator);
	root.AddMember(rapidjson::StringRef(TimeLineTag_Id_Json), rapidjson::Value(m_id.toString().toUtf8().constData(), allocator), allocator);
	root.AddMember(rapidjson::StringRef(TimeLineTag_Position_Json), static_cast<int64_t>(m_tagPosition.count()), allocator);
	root.AddMember(rapidjson::StringRef(TimeLineTag_TCPosition_Json), static_cast<int64_t>(m_tagTCPosition.count()), allocator);
}