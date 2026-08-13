#include "TimeLineNote.h"
#include "ConvertHelper.h"

CTimeLineNote::CTimeLineNote()
	: m_id(QUuid::createUuid())
{
}

CTimeLineNote::CTimeLineNote(const QUuid& id)
	: m_id(id)
{
}

CTimeLineNote::~CTimeLineNote()
{
}

CTimeLineNote::CTimeLineNote(const CTimeLineNote& other)
{
	m_id = other.m_id;
	m_noteName = other.m_noteName;
	m_location = other.m_location;
}

CTimeLineNote& CTimeLineNote::operator=(const CTimeLineNote& other)
{
	if (this == &other)
		return *this;

	std::unique_lock<std::shared_mutex> lock(m_mtx);
	m_id = other.m_id;
	m_noteName = other.m_noteName;
	m_location = other.m_location;
	return *this;
}

bool CTimeLineNote::operator<(const CTimeLineNote& other)
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	return m_location < other.m_location;
}

QUuid CTimeLineNote::GetId()
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	return m_id;
}

void CTimeLineNote::SetId(const QUuid& id)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	m_id = id;
}

QString CTimeLineNote::GetNoteName()
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	return m_noteName;
}

void CTimeLineNote::SetNoteName(const QString& name)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	m_noteName = name;
}

std::chrono::milliseconds CTimeLineNote::GetNotePostion()
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	return m_location;
}

void CTimeLineNote::SetNotePostion(const std::chrono::milliseconds& time)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	// Note位置帧对齐：Floor
	m_location = g_pConvertHelper->formatTimeWithFrame(time, RoundingMode::Floor);
}

#define TimeLineNote_Id_Json "TimeLineNote_Id"
#define TimeLineNote_Name_Json "TimeLineNote_Name"
#define TimeLineNote_Position_Json "TimeLineNote_Position"

bool CTimeLineNote::LoadDataFromRapidJson(const rapidjson::Value& root)
{
	if (!root.IsObject())
		return false;

	if (root.HasMember(TimeLineNote_Id_Json) && root[TimeLineNote_Id_Json].IsString())
	{
		m_id = QUuid::fromString(QString::fromUtf8(root[TimeLineNote_Id_Json].GetString()));
	}

	if (root.HasMember(TimeLineNote_Name_Json) && root[TimeLineNote_Name_Json].IsString())
	{
		m_noteName = QString::fromUtf8(root[TimeLineNote_Name_Json].GetString());
	}

	if (root.HasMember(TimeLineNote_Position_Json) && root[TimeLineNote_Position_Json].IsInt64())
	{
		m_location = g_pConvertHelper->formatTimeWithFrame(
			std::chrono::milliseconds(root[TimeLineNote_Position_Json].GetInt64()), RoundingMode::Floor);
	}

	return true;
}

void CTimeLineNote::SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator)
{
	root.SetObject();

	QByteArray nameBytes = m_noteName.toUtf8();
	root.AddMember(rapidjson::StringRef(TimeLineNote_Name_Json), rapidjson::Value(nameBytes.constData(), allocator), allocator);
	root.AddMember(rapidjson::StringRef(TimeLineNote_Id_Json), rapidjson::Value(m_id.toString().toUtf8().constData(), allocator), allocator);
	root.AddMember(rapidjson::StringRef(TimeLineNote_Position_Json), static_cast<int64_t>(m_location.count()), allocator);
}
