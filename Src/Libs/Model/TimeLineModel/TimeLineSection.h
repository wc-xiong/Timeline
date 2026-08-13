#pragma once
#include "TimeLineModelGlobal.h"
#include "Common/Common/ITimeLineInterface.h"
#include "Common/Common/QUuidEx.h"

#include <map>
#include <unordered_map>
#include <shared_mutex>
#include <vector>
using namespace std::chrono;
class CTimeLineSection : public ITimeLineSection
{
	INIT_ICOPYINTERFACE(CTimeLineSection)
public:
	CTimeLineSection();
	virtual ~CTimeLineSection();
	CTimeLineSection(const CTimeLineSection& other);
	CTimeLineSection& operator=(const CTimeLineSection& other);

	QUuid GetId()override;

	QString GetSectionName()override;
	void SetSectionName(const QString& name)override;

	void SetSectionStartTime(const std::chrono::milliseconds& time)override;
	std::chrono::milliseconds GetSectionStartTime()override;

	std::chrono::milliseconds GetSectionEndTime()override;
	std::chrono::milliseconds GetSectionValidEndTime()override;
	void SetSectionEndTime(const std::chrono::milliseconds& time)override;

	bool LoadDataFromRapidJson(const rapidjson::Value& root)override;
	void SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator)override;

private:
	std::shared_mutex m_mtx;
	QUuid m_id;
	QString m_sectionName;
	std::chrono::milliseconds m_startTime = 0ms;
	std::chrono::milliseconds m_endTime = 0ms;
};