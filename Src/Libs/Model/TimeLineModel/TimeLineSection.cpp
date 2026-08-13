#include "TimeLineSection.h"
#include "ConvertHelper.h"

CTimeLineSection::CTimeLineSection():
	m_id(QUuid::createUuid())
{
}

CTimeLineSection::~CTimeLineSection()
{

}

CTimeLineSection::CTimeLineSection(const CTimeLineSection& other)
	: m_sectionName(other.m_sectionName)
	, m_startTime(other.m_startTime)
	, m_endTime(other.m_endTime)
	, m_id(other.m_id)
{
}

CTimeLineSection& CTimeLineSection::operator=(const CTimeLineSection& other)
{
	if (this != &other)
	{
		m_sectionName = other.m_sectionName;
		m_startTime = other.m_startTime;
		m_endTime = other.m_endTime;
		m_id = other.m_id;
	}
	return *this;
}

QUuid CTimeLineSection::GetId()
{
	return m_id;
}

QString CTimeLineSection::GetSectionName()
{
	return m_sectionName;
}

void CTimeLineSection::SetSectionName(const QString& name)
{
	m_sectionName = name;
}

void CTimeLineSection::SetSectionStartTime(const std::chrono::milliseconds& time)
{
	// Section起点帧对齐：Floor
	m_startTime = g_pConvertHelper->formatTimeWithFrame(time, RoundingMode::Floor);
}

std::chrono::milliseconds CTimeLineSection::GetSectionStartTime()
{
	return m_startTime;
}

std::chrono::milliseconds CTimeLineSection::GetSectionEndTime()
{
	return m_endTime;
}

std::chrono::milliseconds CTimeLineSection::GetSectionValidEndTime()
{
	int endFrames = g_pConvertHelper->timeToFrames(m_endTime, RoundingMode::Floor);
	if (endFrames <= 0)
		return std::chrono::milliseconds(0);
	return g_pConvertHelper->frameToTimes(endFrames - 1);
}

void CTimeLineSection::SetSectionEndTime(const std::chrono::milliseconds& time)
{
	// Section终点帧对齐：Floor
	m_endTime = g_pConvertHelper->formatTimeWithFrame(time, RoundingMode::Floor);
}

bool CTimeLineSection::LoadDataFromRapidJson(const rapidjson::Value& root)
{
	if (!root.IsObject())
		return false;

	if (root.HasMember("sectionName") && root["sectionName"].IsString())
	{
		m_sectionName = QString::fromUtf8(root["sectionName"].GetString());
	}

	if (root.HasMember("startTime") && root["startTime"].IsInt64())
	{
		m_startTime = g_pConvertHelper->formatTimeWithFrame(
			std::chrono::milliseconds(root["startTime"].GetInt64()), RoundingMode::Floor);
	}

	if (root.HasMember("endTime") && root["endTime"].IsInt64())
	{
		m_endTime = g_pConvertHelper->formatTimeWithFrame(
			std::chrono::milliseconds(root["endTime"].GetInt64()), RoundingMode::Floor);
	}

	if (root.HasMember("id") && root["id"].IsString())
	{
		m_id = QUuid::fromString(QString::fromUtf8(root["id"].GetString()));
	}

	return true;
}

void CTimeLineSection::SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator)
{
	root.SetObject();

	QByteArray nameBytes = m_sectionName.toUtf8();
	root.AddMember("sectionName", rapidjson::Value(nameBytes.constData(), allocator), allocator);
	root.AddMember("startTime", static_cast<int64_t>(m_startTime.count()), allocator);
	root.AddMember("endTime", static_cast<int64_t>(m_endTime.count()), allocator);
	root.AddMember("id", rapidjson::Value(m_id.toString().toUtf8().constData(), allocator), allocator);
}